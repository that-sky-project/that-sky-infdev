#include <chrono>
#include <thread>
#include <cstring>
#include <cmath>
#include <includes/htmodloader.h>
#include <Memory/Heap.hpp>
#include "sky/skyGfx.hpp"
#include "sky/skyScene.hpp"
#include "sky/skyAvatarBarn.hpp"
#include "sky/skyCollisionGeo.hpp"
#include "sky/skyMaterialDefBarn.hpp"
#include "sky/skyTypePlaceholders.hpp"
#include "world/synth/perlinNoise.hpp"
#include "world/heightMapChunkBarn.hpp"
#include "render/vertexArrayElements.hpp"

HEAP_TAG_REGISTER(tag_HeightMapChunk)

// ----------------------------------------------------------------------------
// [SECTION] HeightMapChunkSourceParams
// ----------------------------------------------------------------------------

META_REGISTER_CLASS(HeightMapChunkSourceParams, MetaClassImpl<Object>::Must_call_META_REGISTER_CLASS)
META_DATA_CLASS(HeightMapChunkSourceParams, Tool_Export, "true")
META_DATA_CLASS(HeightMapChunkSourceParams, Infdev_LevelExport, "true")

META_REGISTER_SIMPLE_MEMBER(HeightMapChunkSourceParams, seed)
META_DATA_MEMBER_VARIABLE(HeightMapChunkSourceParams, seed, Tool_DefaultValue, "1196250184")

META_REGISTER_SIMPLE_MEMBER(HeightMapChunkSourceParams, viewDistance)
META_DATA_MEMBER_VARIABLE(HeightMapChunkSourceParams, viewDistance, Tool_DefaultValue, "17")
META_DATA_MEMBER_VARIABLE(HeightMapChunkSourceParams, viewDistance, Tool_MinMaxValues, "(4, 20)")

// ----------------------------------------------------------------------------
// [SECTION] HeightMapChunkSource
// ----------------------------------------------------------------------------

void HeightMapChunkSource::Initialize(
  i32 seed
) {
  Random random = Random(seed);
  m_seed = seed;
  m_noise->Initialize(&random, 16);
}

void HeightMapChunkSource::Terminate() {
  m_noise->Terminate();
}

void HeightMapChunkSource::LoadChunk(
  HeightMapChunk *chunk
) {
  f32 heights[HeightMapChunk::kDataSize][HeightMapChunk::kDataSize];
  m_noise->GetRegion(
    (f32 *)heights,
    chunk->GetPos().x * HeightMapChunk::kTileSize - 1,
    chunk->GetPos().z * HeightMapChunk::kTileSize - 1,
    HeightMapChunk::kDataSize, HeightMapChunk::kDataSize,
    1368.824f, 1368.824f);
  for (u32 z = 0; z < 19; z++) {
    for (u32 x = 0; x < 19; x++) {
      heights[x][z] *= 1e-4f;
      heights[x][z] += 2.0f;
    }
  }
  chunk->SetHeights((f32 *)heights);
}

// ----------------------------------------------------------------------------
// [SECTION] HeightMapChunkBarn::RenderData
// ----------------------------------------------------------------------------

static void s_BuildIndices(
  u16 *dst,
  u16 side,
  u16 cover
) {
  // Vertices and indices (x-major order):
  //       (x)      (x+1)
  // (z)    v0 ------ v1
  //        |      /  |
  //        |    /    |
  //        |  /      |
  // (z+1)  v2 ------ v3
  //
  // TODO: Optimize with SIMD.
  for (u32 z = 0; z < cover; z++) {
    for (u32 x = 0; x < cover; x++) {
      u32 quadIdx = z * cover + x;
      u32 baseIdx = quadIdx * 6;

      u32 v0 = x * side + z;
      u32 v1 = v0 + side;
      u32 v2 = v0 + 1;
      u32 v3 = v1 + 1;

      // Triangle 1: v0, v2, v1
      dst[baseIdx + 0] = v0;
      dst[baseIdx + 1] = v2;
      dst[baseIdx + 2] = v1;

      // Triangle 2: v1, v2, v3
      dst[baseIdx + 3] = v1;
      dst[baseIdx + 4] = v2;
      dst[baseIdx + 5] = v3;
    }
  }
}

void HeightMapChunkBarn::RenderData::Initialize(
  cstring tag,
  const GfxType *types,
  const GfxAttr *attrs,
  u32 attrCount,
  ResourceManager *resources,
  cstring shader,
  RenderList *renderList,
  Heap *heap
) {
  data.BeginDefinition(tag, kChunkVtxCount * kMaxChunks);

  // Prefill indices.
  u16 *indices = (u16 *)heap->Allocate(
    sizeof(u16) * kChunkIdxCount,
    tag_HeightMapChunk,
    alignof(u16));
  
  s_BuildIndices(indices, 17, 16);

  data.AddIndexBuffer(0, kGfxType_SHORT, kGfxBind_UploadSingle, kChunkIdxCount, indices);
  data.AddVertexBuffer(0, types, attrs, attrCount, kGfxBind_UploadTriple, 0, nullptr);
  data.EndDefinition();

  // Free the temporary index buffer.
  heap->Free(indices);

  render.Initialize(&data, resources, shader, renderList, 0, nullptr);

  // Allocate chunks.
  render.AllocVertexSparse(true, heap, kMaxChunks);
}

void HeightMapChunkBarn::RenderData::Terminate() {
  render.Dequeue();
  render.AllocVertexSparse(false, nullptr, 0);
  render.Release();
  data.Release();
}

// ----------------------------------------------------------------------------
// [SECTION] HeightMapChunkBarn::ClientChunk
// ----------------------------------------------------------------------------

void HeightMapChunkBarn::ClientChunk::Initialize(
  Heap *heap,
  CollisionGeoBarn *collisionGeoBarn,
  const ChunkPos &pos
) {
  // Allocate collision vertices.
  m_dvtx = (TerrainDepthVertex *)heap->Allocate(
    sizeof(TerrainDepthVertex) * kChunkVtxCount,
    tag_HeightMapChunk,
    0x10);

  m_mvtx = (TerrainMaterialVertex *)heap->Allocate(
    sizeof(TerrainMaterialVertex) * kChunkVtxCount,
    tag_HeightMapChunk,
    0x10);

  m_data.Initialize(pos);

  m_heap = heap;
  m_collisionGeoBarn = collisionGeoBarn;

  m_hasBuiltMesh = false;
  m_hasBuiltGeo = false;
  m_hasLoaded = false;
}

void HeightMapChunkBarn::ClientChunk::Terminate() {
  if (m_dvtx) m_heap->Free(m_dvtx);
  m_dvtx = nullptr;

  if (m_mvtx) m_heap->Free(m_mvtx);
  m_mvtx = nullptr;

  m_data.Terminate();

  RemoveCollision();
}

void HeightMapChunkBarn::ClientChunk::BuildMesh() {
  if (!m_hasLoaded || m_hasBuiltMesh)
    return;

  // Get chunk data.
  ChunkPos chunkPos = GetPos();
  const f32 *heights = m_data.GetHeights();

  // Generate vertices with positions and normals.
  for (u32 z = 0; z < HeightMapChunk::kRealSize; z++) {
    for (u32 x = 0; x < HeightMapChunk::kRealSize; x++) {
      // NOTE: chunksource.GetRegion() stores data in x-major order:
      //       buffer[x * zSize + z].
      u32 idx = x * HeightMapChunk::kRealSize + z;
      f32 height = m_data.Index(x, z);

      // World position.
      f32 worldX = chunkPos.x * 16.0f + x;
      f32 worldZ = chunkPos.z * 16.0f + z;

      GrassShVertex vtx = {worldX, height, worldZ};
      TerrainDepthVertex vtx2 = {worldX, height, worldZ};

      // Calculate normal using neighboring heights.
      // Data is stored in x-major order: heights[x * 17 + z]
      f32 hL = m_data.Index((i32)x - 1, (i32)z);
      f32 hR = m_data.Index((i32)x + 1, (i32)z);
      f32 hD = m_data.Index((i32)x, (i32)z - 1);
      f32 hU = m_data.Index((i32)x, (i32)z + 1);

      // Tangent vectors.
      f32 tx = 2.0f, ty = hR - hL, tz = 0.0f;
      f32 bx = 0.0f, by = hU - hD, bz = 2.0f;

      // Cross product for normal.
      f32 nx = by * tz - bz * ty;
      f32 ny = bz * tx - bx * tz;
      f32 nz = bx * ty - by * tx;

      // Normalize.
      f32 len = sqrtf(nx * nx + ny * ny + nz * nz);
      if (len > 0.0f) {
        nx /= len;
        ny /= len;
        nz /= len;
      } else {
        nx = 0.0f;
        ny = 1.0f;
        nz = 0.0f;
      }

      // Pack normal into BYTE4 format (range -1..1 -> -127..127).
      // a_normal.w is used as the weight of the material.
      vtx.a_normal = (i32)R8G8B8A8_SNORM(nx, ny, nz, 1.0f);

      // Keep default light values.
      vtx.a_light0 = 0x407F7F7F;
      vtx.a_light1 = 0x0000FFB3;
      vtx.a_light2 = 0xFF80FF80;

      m_mvtx[idx] = vtx;
      m_dvtx[idx] = vtx2;
    }
  }

  m_hasBuiltMesh = true;
}

void HeightMapChunkBarn::ClientChunk::BuildCollision() {
  if (!m_hasLoaded || m_hasBuiltGeo)
    return;

  Matrix4 transform = Matrix4(1);
  Material material = kMaterial_Grass;
  u32 light = 0x00000000;

  CollisionGeoMeshData meshData;
  meshData.tag = "ClientChunk";

  // Fill indices.
  // TODO: Cache the unchanged index buffer.
  u16 *indices = (u16 *)m_heap->Allocate(
    sizeof(u16) * kCollisionChunkIdxCount,
    tag_HeightMapChunk,
    alignof(u16));

  s_BuildIndices(indices, 18, 17);

  // Set index buffer.
  meshData.idxBuffer = indices;
  meshData.idxCount = kCollisionChunkIdxCount;
  meshData.idxStride = sizeof(u16);

  // Allocate collision vertices.
  TerrainMaterialVertex *cvtx = (TerrainMaterialVertex *)m_heap->Allocate(
    sizeof(TerrainMaterialVertex) * kCollisionChunkVtxCount,
    tag_HeightMapChunk,
    0x10);

  // Generate vertices.
  ChunkPos chunkPos = GetPos();
  for (u32 z = 0; z < kCollisionChunkSize; z++) {
    for (u32 x = 0; x < kCollisionChunkSize; x++) {
      u32 idx = x * kCollisionChunkSize + z;
      f32 height = m_data.Index(x, z);

      // World position.
      f32 worldX = chunkPos.x * 16.0f + x;
      f32 worldZ = chunkPos.z * 16.0f + z;

      // Collect the AABB of the chunk.
      m_min = Vector4::minval(m_min, Vector4(worldX, height, worldZ, 0));
      m_max = Vector4::maxval(m_max, Vector4(worldX, height, worldZ, 0));

      TerrainMaterialVertex v = {worldX, height, worldZ};

      cvtx[idx] = v;
    }
  }

  // Set vertex buffer.
  meshData.vtxBuffer = cvtx;
  meshData.vtxCount = kCollisionChunkVtxCount;
  meshData.vtxStride = sizeof(TerrainMaterialVertex);

  // Set min/max value of collision.
  meshData.min = m_min - 0.1f;
  meshData.max = m_max + 0.1f;

  m_geoIdx = m_collisionGeoBarn->AddGeo(meshData);

  CollisionGeoInstanceData instData;

  instData.mtrlData = &material;
  instData.mtrlType = kGfxType_UBYTE;
  instData.mtrlStride = 0;

  instData.lightData = (u08 *)cvtx + offsetof(TerrainMaterialVertex, a_light0);
  instData.lightType = kGfxType_UBYTE4;
  instData.lightStride = sizeof(TerrainMaterialVertex);

  // Mask == 0x10, terrain.
  // Mask == 0x40, edge-protection will be incorrectly triggered.
  instData.mask = 0x10;
  instData.unk_1 = 1.0f;

  m_geoInst = m_collisionGeoBarn->AddInstance(m_geoIdx, transform, instData, nullptr);

  m_hasBuiltGeo = true;

  HTTellText(
    "BUILD COLLISIONGEO (%d, %d) (%f, %f, %f) -> (%f, %f, %f), GEO %d, INST %p",
    GetPos().x, GetPos().z,
    m_min->x, m_min->y, m_min->z,
    m_max->x, m_max->y, m_max->z,
    m_geoIdx,
    m_geoInst);

  m_heap->Free(indices);
  m_heap->Free(cvtx);
}

void HeightMapChunkBarn::ClientChunk::RemoveCollision() {
  if (m_geoInst) m_collisionGeoBarn->RemoveInstance(m_geoInst);
  m_geoInst = nullptr;

  if (m_geoIdx) m_collisionGeoBarn->RemoveGeo(m_geoIdx);
  m_geoIdx = 0;

  m_hasBuiltGeo = false;
}

// ----------------------------------------------------------------------------
// [SECTION] HeightMapChunkBarn
// ----------------------------------------------------------------------------

META_REGISTER_CLASS(HeightMapChunkBarn, MetaClassImpl<Module>::Must_call_META_REGISTER_CLASS)

// Chunk generator thread.
void HeightMapChunkBarn::ms_ChunkUpdateThread(
  HeightMapChunkBarn *heightMapChunkBarn
) {
  while (heightMapChunkBarn->m_running) {
    {
      // Load chunks.
      std::lock_guard<std::shared_mutex> lock(heightMapChunkBarn->m_lock);
      for (auto &it: heightMapChunkBarn->m_clientChunks) {
        auto &chunk = it.second;
        if (!chunk.m_hasLoaded) {
          heightMapChunkBarn->m_chunkSource->LoadChunk(&chunk.m_data);
          chunk.m_hasLoaded = true;
          HTTellText("LOADED CHUNK (%d, %d)", chunk.GetPos().x, chunk.GetPos().z);
        }
      }
    }

    // Sleep for 50 ms to reduce CPU usage.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

// Initialize the object.
META_REGISTER_FUNCTION_MEMBER(HeightMapChunkBarn, Initialize)
void HeightMapChunkBarn::Initialize(
  CollisionGeoBarn *collisionGeoBarn
) {
  m_collisionGeoBarn = collisionGeoBarn;

  // Create Heap object.
  m_heap = new Heap();
  m_heap->Initialize(
    new char[0x2000000],
    0x2000000, // 32MiB
    "HeightMapChunkBarn",
    true,
    false
  );

  // Initialize world generator.
  m_running = true;
  m_chunkUpdateThread = std::thread(ms_ChunkUpdateThread, this);
}

// Deinitialize the object.
META_REGISTER_FUNCTION_MEMBER(HeightMapChunkBarn, Terminate)
void HeightMapChunkBarn::Terminate() {
  m_running = false;
  if (m_chunkUpdateThread.joinable()) {
    m_chunkUpdateThread.join();
  }

  m_ClearClientChunk();

  // NOTE: A more stable implementation would be to first take out `m_base` and
  // release it. However, many other objects in Sky (at least in my view) do not
  // do this, so let's just release the `Heap` itself.
  m_heap->Terminate();
  delete m_heap;
}

// Initialize chunk renderer contexts.
META_REGISTER_FUNCTION_MEMBER(HeightMapChunkBarn, OnLevelLoad)
META_DATA_MEMBER_FUNCTION(HeightMapChunkBarn, OnLevelLoad, ArgName, "(levelName)")
void HeightMapChunkBarn::OnLevelLoad(
  Game *game,
  ResourceManager *resources,
  Scene *scene,
  MaterialDefBarn *materialDefBarn,
  cstring levelName
) {
  if (!m_hasParams)
    return;

  // Set chunk generator params.
  m_seed = m_params.seed;
  m_viewDistance = m_params.viewDistance;

  // Initialize chunk source.
  m_chunkSource->Initialize(m_seed);

  // Initialize renderer.
  m_depth.Initialize(
    "Infdev_DEPTH",
    TerrainDepthVertex::kTypes,
    TerrainDepthVertex::kAttrs,
    TerrainDepthVertex::kNumAttrs,
    resources,
    "TerrainDepth",
    scene->GetRenderListByName("TerrainDepth"),
    m_heap
  );
  m_mats.Initialize(
    "Infdev_GRASSSH",
    GrassShVertex::kTypes,
    GrassShVertex::kAttrs,
    GrassShVertex::kNumAttrs,
    resources,
    "GrassSh",
    scene->GetRenderListByName("TerrainMats"),
    m_heap
  );

  // Add material uniforms.
  MaterialDefBarn::SetMaterialShaderUniforms(
    m_mats.render.GetPipelineInstance(),
    materialDefBarn->GetDef(kMaterial_Grass),
    resources);
}

// Deinitialize chunk renderer contexts.
META_REGISTER_FUNCTION_MEMBER(HeightMapChunkBarn, OnLevelUnload)
META_DATA_MEMBER_FUNCTION(HeightMapChunkBarn, OnLevelUnload, ArgName, "(levelName)")
void HeightMapChunkBarn::OnLevelUnload(
  cstring levelName
) {
  if (!m_hasParams)
    return;

  m_hasParams = false;

  // Deinitialize renderer.
  m_depth.Terminate();
  m_mats.Terminate();

  // Remove all client chunks.
  m_ClearClientChunk();

  // Terminate chunk source.
  m_chunkSource->Terminate();
}

// Load and unload chunks by the position of local avatar.
META_REGISTER_FUNCTION_MEMBER(HeightMapChunkBarn, Update)
META_DATA_MEMBER_FUNCTION(HeightMapChunkBarn, Update, ArgName, "(levelName)")
void HeightMapChunkBarn::Update(
  Game *game,
  AvatarBarn *avatarBarn,
  cstring levelName
) {
  if (!m_hasParams)
    return;

  Avatar *avatar = avatarBarn->TryGetLocalAvatar(false);
  if (!avatar)
    return;

  Vector4 avatarPos = avatar->GetTransform()[3];
  //avatar->Teleport(Vector3(4, 4, 4), false);
  //avatar->Teleport(Vector3(avatarPos[0], avatarPos[1], avatarPos[2]), true);

  ChunkPos avatarChunkPos = {
    (i32)floorf(avatarPos->x / 16.0f),
    (i32)floorf(avatarPos->z / 16.0f)
  };

  // Only update chunks when avatar position changes.
  if (m_lastPos != avatarChunkPos) {
    m_lastPos = avatarChunkPos;

    i32 r = (i32)m_viewDistance;
    std::vector<ChunkPos> chunksToUnload;

    // Unload chunks.
    {
      std::shared_lock<std::shared_mutex> lock(m_lock);
      for (const auto &pair: m_clientChunks) {
        const ChunkPos &chunkPos = pair.first;
        i32 dx = chunkPos.x - avatarChunkPos.x
          , dz = chunkPos.z - avatarChunkPos.z;

        if (dx * dx + dz * dz > (r + 1) * (r + 1))
          chunksToUnload.push_back(chunkPos);
      }
    }

    for (const ChunkPos &pos: chunksToUnload)
      m_RemoveClientChunk(pos);

    // Add chunks.
    for (i32 dx = -r; dx <= r; ++dx) {
      for (i32 dz = -r; dz <= r; ++dz) {
        if (dx * dx + dz * dz > r * r)
          continue;

        ChunkPos chunkPos = {avatarChunkPos.x + dx, avatarChunkPos.z + dz};

        if (!m_IsClientChunkAdded(chunkPos))
          m_AddClientChunk(chunkPos);
      }
    }
  }

  // Build chunk mesh.
  {
    std::lock_guard<std::shared_mutex> lock(m_lock);
    for (auto &pair: m_clientChunks) {
      auto &chunk = pair.second;
      if (!chunk.HasMesh()) chunk.BuildMesh();

      i32 dx = chunk.GetPos().x - avatarChunkPos.x
        , dz = chunk.GetPos().z - avatarChunkPos.z;
      if (-1 <= dx && dx <= 1 && -1 <= dz && dz <= 1)
        chunk.BuildCollision();
      else
        chunk.RemoveCollision();
    }
  }
}

// Build vertices for visible chunks.
META_REGISTER_FUNCTION_MEMBER(HeightMapChunkBarn, BuildScene)
META_DATA_MEMBER_FUNCTION(HeightMapChunkBarn, BuildScene, ArgName, "(levelName)")
void HeightMapChunkBarn::BuildScene(
  cstring levelName
) {
  if (!m_hasParams)
    return;

  // Map vertex and index buffers.
  TerrainDepthVertex *depthVtx = (TerrainDepthVertex *)m_depth.MapVtxBuffer();
  GrassShVertex *matVtx = (GrassShVertex *)m_mats.MapVtxBuffer();

  // Return if failed to map buffers.
  if (!depthVtx || !matVtx) {
    if (depthVtx) m_depth.UnmapVtxBuffer();
    if (matVtx) m_mats.UnmapVtxBuffer();
    return;
  }

  m_depth.ClearRenderChunk();
  m_mats.ClearRenderChunk();

  // Append all loaded chunks to render.
  {
    std::shared_lock<std::shared_mutex> lock(m_lock);

    u32 vtxCount = 0;
    for (const auto &pair: m_clientChunks) {
      auto &chunk = pair.second;
      if (!chunk.HasMesh())
        continue;

      memcpy(&depthVtx[vtxCount], chunk.m_dvtx, sizeof(TerrainDepthVertex) * kChunkVtxCount);
      memcpy(&matVtx[vtxCount], chunk.m_mvtx, sizeof(TerrainMaterialVertex) * kChunkVtxCount);

      m_depth.AddRenderChunk(0, kChunkIdxCount, vtxCount);
      m_mats.AddRenderChunk(0, kChunkIdxCount, vtxCount);

      vtxCount += kChunkVtxCount;
    }
  }

  // Unmap buffers.
  m_depth.UnmapVtxBuffer();
  m_mats.UnmapVtxBuffer();

  // Queue present.
  m_depth.Queue();
  m_mats.Queue();
}

META_REGISTER_FUNCTION_MEMBER(HeightMapChunkBarn, CreateParams)
META_DATA_MEMBER_FUNCTION(HeightMapChunkBarn, CreateParams, ObjectFactory_CreatesType, "HeightMapChunkSourceParams")
HeightMapChunkSourceParams *HeightMapChunkBarn::CreateParams(
  MetaClass *mc
) {
  Assert(!m_hasParams);
  m_hasParams = true;
  return &m_params;
}

void HeightMapChunkBarn::m_AddClientChunk(
  const ChunkPos &pos
) {
  std::lock_guard<std::shared_mutex> lock(m_lock);
  if (m_clientChunks.find(pos) != m_clientChunks.end())
    return;

  ClientChunk chunk;
  chunk.Initialize(m_heap, m_collisionGeoBarn, pos);
  m_clientChunks[pos] = chunk;
}

void HeightMapChunkBarn::m_RemoveClientChunk(
  const ChunkPos &pos
) {
  std::lock_guard<std::shared_mutex> lock(m_lock);

  auto it = m_clientChunks.find(pos);
  if (it == m_clientChunks.end())
    return;

  ClientChunk &chunk = it->second;
  chunk.Terminate();

  m_clientChunks.erase(it);
}

void HeightMapChunkBarn::m_ClearClientChunk() {
  std::lock_guard<std::shared_mutex> lock(m_lock);

  for (auto &it: m_clientChunks)
    it.second.Terminate();

  m_clientChunks.clear();
}

bool HeightMapChunkBarn::m_IsClientChunkAdded(
  const ChunkPos &pos
) {
  std::shared_lock<std::shared_mutex> lock(m_lock);
  return m_clientChunks.find(pos) != m_clientChunks.end();
}

void HeightMapChunkBarn::m_BuildClientChunkMesh(
  ClientChunk *chunk
) {
  std::lock_guard<std::shared_mutex> lock(m_lock);

  for (auto &it: m_clientChunks)
    it.second.Terminate();

  m_clientChunks.clear();
}
