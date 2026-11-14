#include <chrono>
#include <thread>
#include <cstring>
#include <cstddef>
#include <cmath>
#include <algorithm>
#include <includes/htmodloader.h>
#include <Memory/Heap.hpp>
#include "sky/skyGfx.hpp"
#include "sky/skyScene.hpp"
#include "sky/skyAvatarBarn.hpp"
#include "sky/skyCollisionGeo.hpp"
#include "sky/skyMaterialDefBarn.hpp"
#include "sky/skyTypePlaceholders.hpp"
#include "world/voxelChunkBarn.hpp"

HEAP_TAG_REGISTER(tag_VoxelChunk)

// ----------------------------------------------------------------------------
// [SECTION] VoxelChunkSourceParams
// ----------------------------------------------------------------------------

META_REGISTER_CLASS(VoxelChunkSourceParams, MetaClassImpl<Object>::Must_call_META_REGISTER_CLASS)
META_DATA_CLASS(VoxelChunkSourceParams, Tool_Export, "true")
META_DATA_CLASS(VoxelChunkSourceParams, Infdev_LevelExport, "true")

META_REGISTER_SIMPLE_MEMBER(VoxelChunkSourceParams, seed)
META_DATA_MEMBER_VARIABLE(VoxelChunkSourceParams, seed, Tool_DefaultValue, "1196250184")

META_REGISTER_SIMPLE_MEMBER(VoxelChunkSourceParams, viewDistance)
META_DATA_MEMBER_VARIABLE(VoxelChunkSourceParams, viewDistance, Tool_DefaultValue, "4")
META_DATA_MEMBER_VARIABLE(VoxelChunkSourceParams, viewDistance, Tool_MinMaxValues, "(1, 6)")

// ----------------------------------------------------------------------------
// [SECTION] VoxelChunkSource
// ----------------------------------------------------------------------------

// The surface is sampled in grid space, where the voxels library uses Z as
// the up axis. The polygonizer swaps Y and Z in its output, so the positions
// it returns are already in the Y-up world space used by Sky.
void VoxelChunkSource::Initialize(
  i32 seed
) {
  Terminate();

  m_noise = new PerlinNoise();

  Random random = Random(seed);
  m_noise->Initialize(&random, 16);
}

void VoxelChunkSource::Terminate() {
  if (!m_noise)
    return;

  m_noise->Terminate();
  delete m_noise;
  m_noise = nullptr;
}

void VoxelChunkSource::GetSurface(
  float xStart,
  float xEnd,
  float xStep,
  float yStart,
  float yEnd,
  float yStep,
  float zStart,
  float zEnd,
  float zStep,
  float *output,
  unsigned char *materialid,
  unsigned char *blend
) {
  if (!m_noise || !output)
    return;

  u32 id = 0;
  for (f32 z = zStart; z < zEnd; z += zStep) {
    for (f32 y = yStart; y < yEnd; y += yStep) {
      for (f32 x = xStart; x < xEnd; x += xStep) {
        // Grid (x, y, z) -> world (x, z, y), with x/z shifted by the chunk
        // origin. `y` is the world Z axis and `z` is the world Y (up) axis.
        f32 wx = (f32)m_baseX + x;
        f32 wy = z;
        f32 wz = (f32)m_baseZ + y;

        output[id] = m_SampleDensity(wx, wy, wz);
        if (materialid)
          materialid[id] = 0;
        if (blend)
          blend[id] = 0;

        ++id;
      }
    }
  }
}

f32 VoxelChunkSource::m_SampleDensity(
  f32 wx,
  f32 wy,
  f32 wz
) const {
  // Same noise configuration as HeightMapChunkBarn, so both generators
  // describe the same terrain: low-frequency octaves dominate, and the
  // 1e-4 scale brings the accumulated amplitude down to a few blocks.
  static constexpr f32 kNoiseScale = 1368.824f;
  static constexpr f32 kNoiseAmplitude = 1e-4f;
  static constexpr f32 kBaseHeight = 2.0f;
  static constexpr f32 kPerturbation = 0.5f;

  f32 height = kBaseHeight
    + m_noise->GetValue(wx * kNoiseScale, 10.0f, wz * kNoiseScale) * kNoiseAmplitude;

  // Positive density is outside the surface, so the solid part is below the
  // height field. The 3D term turns the height field into pockets, overhangs
  // and caves.
  f32 density = wy - height;
  density += m_noise->GetValue(
    wx * kNoiseScale, wy * kNoiseScale, wz * kNoiseScale) * (kNoiseAmplitude * kPerturbation);

  return density;
}

// Returns true when the triangle is wound counter-clockwise as seen from the
// side its vertex normals point at. The polygonizer emits positions and
// normals in the same (Y-up) space, but its winding follows the internal Z-up
// grid, so it has to be corrected for Sky's renderer.
static bool s_IsFrontFacing(
  const Voxels::PolygonVertex &v0,
  const Voxels::PolygonVertex &v1,
  const Voxels::PolygonVertex &v2
) {
  f32 e1x = v1.Position.x - v0.Position.x;
  f32 e1y = v1.Position.y - v0.Position.y;
  f32 e1z = v1.Position.z - v0.Position.z;

  f32 e2x = v2.Position.x - v0.Position.x;
  f32 e2y = v2.Position.y - v0.Position.y;
  f32 e2z = v2.Position.z - v0.Position.z;

  f32 cx = e1y * e2z - e1z * e2y;
  f32 cy = e1z * e2x - e1x * e2z;
  f32 cz = e1x * e2y - e1y * e2x;

  f32 nx = v0.Normal.x + v1.Normal.x + v2.Normal.x;
  f32 ny = v0.Normal.y + v1.Normal.y + v2.Normal.y;
  f32 nz = v0.Normal.z + v1.Normal.z + v2.Normal.z;

  return cx * nx + cy * ny + cz * nz >= 0.0f;
}

bool VoxelChunkSource::Generate(
  const ChunkPos &pos,
  VoxelChunkMesh *mesh
) {
  if (!m_noise || !mesh)
    return false;

  mesh->Clear();

  m_baseX = pos.x * (i32)kChunkSize;
  m_baseZ = pos.z * (i32)kChunkSize;

  Voxels::Grid *grid = Voxels::Grid::Create(
    kChunkSize, kChunkSize, kChunkSize,
    0.0f, 0.0f, 0.0f, 1.0f,
    this);

  if (!grid) {
    HTTellText("§c[VoxelChunkBarn] Failed to create grid at (%d, %d)", pos.x, pos.z);
    return false;
  }

  Voxels::Polygonizer polygonizer;
  Voxels::PolygonSurface *surface = polygonizer.Execute(*grid, &m_materialMap);
  if (!surface) {
    HTTellText("§c[VoxelChunkBarn] Failed to polygonize chunk at (%d, %d)", pos.x, pos.z);
    grid->Destroy();
    return false;
  }

  // Only LOD 0 is rendered: 16^3 grids have a single LOD level, and mixing
  // LOD levels requires the transition meshes that are not used here.
  if (surface->GetLevelsCount() > 0) {
    const u32 blockCount = (u32)surface->GetBlocksForLevelCount(0);

    for (u32 blockIdx = 0; blockIdx < blockCount; ++blockIdx) {
      const Voxels::BlockPolygons *block = surface->GetBlockForLevel(0, blockIdx);
      if (!block)
        continue;

      Voxels::uint vtxCount = 0;
      const Voxels::PolygonVertex *verts = block->GetVertices(&vtxCount);

      Voxels::uint idxCount = 0;
      const unsigned *indices = block->GetIndices(&idxCount);

      if (!verts || !vtxCount || !indices || !idxCount)
        continue;

      const u32 baseVertex = (u32)mesh->materialVertices.size();
      const u32 vertexRoom = VoxelChunkMesh::kMaxVertices - baseVertex;
      const u32 vertexCopy = (u32)std::min<Voxels::uint>(vtxCount, vertexRoom);

      if (vertexCopy < vtxCount)
        mesh->truncated = true;

      for (u32 i = 0; i < vertexCopy; ++i) {
        const Voxels::PolygonVertex &src = verts[i];

        f32 wx = (f32)m_baseX + src.Position.x;
        f32 wy = src.Position.y;
        f32 wz = (f32)m_baseZ + src.Position.z;

        TerrainDepthVertex dvtx(wx, wy, wz);
        TerrainMaterialVertex mvtx(wx, wy, wz);

        mvtx.a_normal = (i32)R8G8B8A8_SNORM(src.Normal.x, src.Normal.y, src.Normal.z, 1.0f);
        mvtx.a_light0 = 0x407F7F7F;
        mvtx.a_light1 = 0x0000FFB3;
        mvtx.a_light2 = 0xFF80FF80;

        mesh->depthVertices.push_back(dvtx);
        mesh->materialVertices.push_back(mvtx);

        mesh->min = Vector4::minval(mesh->min, Vector4(wx, wy, wz, 0.0f));
        mesh->max = Vector4::maxval(mesh->max, Vector4(wx, wy, wz, 0.0f));
      }

      for (u32 i = 0; i + 2 < (u32)idxCount; i += 3) {
        u32 i0 = indices[i];
        u32 i1 = indices[i + 1];
        u32 i2 = indices[i + 2];

        if (i0 >= vertexCopy || i1 >= vertexCopy || i2 >= vertexCopy) {
          mesh->truncated = true;
          continue;
        }

        if (mesh->indices.size() + 3 > VoxelChunkMesh::kMaxIndices) {
          mesh->truncated = true;
          break;
        }

        if (!s_IsFrontFacing(verts[i0], verts[i1], verts[i2]))
          std::swap(i1, i2);

        mesh->indices.push_back((u16)(baseVertex + i0));
        mesh->indices.push_back((u16)(baseVertex + i1));
        mesh->indices.push_back((u16)(baseVertex + i2));
      }
    }
  }

  surface->Destroy();
  grid->Destroy();

  if (mesh->IsEmpty()) {
    mesh->Clear();
    return false;
  }

  return true;
}

// ----------------------------------------------------------------------------
// [SECTION] VoxelChunkBarn::RenderData
// ----------------------------------------------------------------------------

void VoxelChunkBarn::RenderData::Initialize(
  cstring tag,
  const GfxType *types,
  const GfxAttr *attrs,
  u32 attrCount,
  ResourceManager *resources,
  cstring shader,
  RenderList *renderList,
  Heap *heap,
  u32 chunkCapacity,
  u32 vertexCapacity,
  u32 indexCapacity
) {
  data.BeginDefinition(tag, vertexCapacity);

  // The index buffer is shared by all chunks and rewritten every time the
  // scene is built, because every voxel chunk has a different topology.
  data.AddIndexBuffer(0, kGfxType_SHORT, kGfxBind_UploadTriple, indexCapacity, nullptr);
  data.AddVertexBuffer(0, types, attrs, attrCount, kGfxBind_UploadTriple, 0, nullptr);
  data.EndDefinition();

  render.Initialize(&data, resources, shader, renderList, 0, nullptr);

  // Allocate one indirect draw sub-command per chunk.
  render.AllocVertexSparse(true, heap, chunkCapacity);
}

void VoxelChunkBarn::RenderData::Terminate() {
  render.Dequeue();
  render.AllocVertexSparse(false, nullptr, 0);
  render.Release();
  data.Release();
}

// ----------------------------------------------------------------------------
// [SECTION] VoxelChunkBarn::ClientChunk
// ----------------------------------------------------------------------------

void VoxelChunkBarn::ClientChunk::Initialize(
  CollisionGeoBarn *collisionGeoBarn,
  const ChunkPos &pos
) {
  m_collisionGeoBarn = collisionGeoBarn;
  m_pos = pos;
  m_mesh.Clear();
  m_hasMesh = false;
  m_hasBuiltGeo = false;
  m_geoIdx = 0;
  m_geoInst = nullptr;
}

void VoxelChunkBarn::ClientChunk::Terminate() {
  RemoveCollision();

  m_mesh.Clear();
  m_hasMesh = false;
}

void VoxelChunkBarn::ClientChunk::BuildCollision() {
  if (!m_collisionGeoBarn || m_hasBuiltGeo || !m_hasMesh || m_mesh.IsEmpty())
    return;

  Matrix4 transform = Matrix4(1);
  Material material = kMaterial_Grass;

  CollisionGeoMeshData meshData;
  meshData.tag = "VoxelChunk";

  meshData.idxBuffer = m_mesh.indices.data();
  meshData.idxCount = (u32)m_mesh.indices.size();
  meshData.idxStride = sizeof(u16);

  meshData.vtxBuffer = m_mesh.materialVertices.data();
  meshData.vtxCount = (u32)m_mesh.materialVertices.size();
  meshData.vtxStride = sizeof(TerrainMaterialVertex);

  meshData.min = m_mesh.min - 0.1f;
  meshData.max = m_mesh.max + 0.1f;

  m_geoIdx = m_collisionGeoBarn->AddGeo(meshData);

  CollisionGeoInstanceData instData;

  instData.mtrlData = &material;
  instData.mtrlType = kGfxType_UBYTE;
  instData.mtrlStride = 0;

  instData.lightData = (u08 *)m_mesh.materialVertices.data()
    + offsetof(TerrainMaterialVertex, a_light0);
  instData.lightType = kGfxType_UBYTE4;
  instData.lightStride = sizeof(TerrainMaterialVertex);

  // Mask == 0x10, terrain.
  instData.mask = 0x10;
  instData.unk_1 = 1.0f;

  m_geoInst = m_collisionGeoBarn->AddInstance(m_geoIdx, transform, instData, nullptr);

  m_hasBuiltGeo = true;

  HTTellText("§a[VoxelChunkBarn] (%d, %d)", m_pos.x, m_pos.z);
}

void VoxelChunkBarn::ClientChunk::RemoveCollision() {
  if (!m_collisionGeoBarn)
    return;

  if (m_geoInst)
    m_collisionGeoBarn->RemoveInstance(m_geoInst);
  m_geoInst = nullptr;

  if (m_geoIdx)
    m_collisionGeoBarn->RemoveGeo(m_geoIdx);
  m_geoIdx = 0;

  m_hasBuiltGeo = false;
}

// ----------------------------------------------------------------------------
// [SECTION] VoxelChunkBarn
// ----------------------------------------------------------------------------

META_REGISTER_CLASS(VoxelChunkBarn, MetaClassImpl<Module>::Must_call_META_REGISTER_CLASS)

u32 VoxelChunkBarn::ms_CountChunks(
  u32 radius
) {
  const i32 r = (i32)radius;
  u32 count = 0;
  for (i32 dx = -r; dx <= r; ++dx) {
    for (i32 dz = -r; dz <= r; ++dz) {
      if (dx * dx + dz * dz <= r * r)
        ++count;
    }
  }
  return count;
}

// Chunk generator thread.
void VoxelChunkBarn::ms_ChunkUpdateThread(
  VoxelChunkBarn *voxelChunkBarn
) {
  while (voxelChunkBarn->m_running) {
    // Pick a chunk that still needs a mesh. The selected position is stable
    // while the generation runs; it is looked up again before installing.
    ChunkPos pos = {};
    bool hasWork = false;
    {
      std::shared_lock<std::shared_mutex> lock(voxelChunkBarn->m_lock);
      for (auto &it: voxelChunkBarn->m_clientChunks) {
        if (!it.second.HasMesh()) {
          pos = it.second.GetPos();
          hasWork = true;
          break;
        }
      }
    }

    if (!hasWork) {
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
      continue;
    }

    VoxelChunkMesh mesh;
    bool generated = false;
    {
      // Grid creation and polygonization use the chunk source, which is
      // initialized and terminated on the main thread.
      std::lock_guard<std::mutex> lock(voxelChunkBarn->m_sourceLock);
      if (voxelChunkBarn->m_sourceReady && voxelChunkBarn->m_running)
        generated = voxelChunkBarn->m_chunkSource->Generate(pos, &mesh);
      else
        continue;
    }

    {
      std::lock_guard<std::shared_mutex> lock(voxelChunkBarn->m_lock);
      auto it = voxelChunkBarn->m_clientChunks.find(pos);
      if (it != voxelChunkBarn->m_clientChunks.end() && !it->second.HasMesh()) {
        // Empty/void chunks are marked as done as well, so they are not
        // retried on every pass.
        if (generated) {
          if (mesh.truncated) {
            HTTellText(
              "§e[VoxelChunkBarn] Chunk (%d, %d) exceeded the mesh budget and was clipped",
              pos.x, pos.z);
          }
          it->second.m_mesh = std::move(mesh);
        }
        it->second.m_hasMesh = true;
      }
    }

    if (!generated)
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

// Initialize the object.
META_REGISTER_FUNCTION_MEMBER(VoxelChunkBarn, Initialize)
void VoxelChunkBarn::Initialize(
  CollisionGeoBarn *collisionGeoBarn
) {
  m_collisionGeoBarn = collisionGeoBarn;
  m_chunkSource = new VoxelChunkSource();

  // Create Heap object.
  m_heap = new Heap();
  m_heap->Initialize(
    new char[0x2000000],
    0x2000000, // 32MiB
    "VoxelChunkBarn",
    true,
    false
  );

  // The voxels library must be initialized before any of its types are used.
  m_voxelsReady = InitializeVoxels(VOXELS_VERSION, nullptr, nullptr) == Voxels::IE_Ok;
  if (!m_voxelsReady)
    HTTellText("§c[VoxelChunkBarn] Failed to initialize the voxels library");

  // Initialize world generator.
  m_running = true;
  m_chunkUpdateThread = std::thread(ms_ChunkUpdateThread, this);
}

// Deinitialize the object.
META_REGISTER_FUNCTION_MEMBER(VoxelChunkBarn, Terminate)
void VoxelChunkBarn::Terminate() {
  m_running = false;
  if (m_chunkUpdateThread.joinable())
    m_chunkUpdateThread.join();

  m_ClearClientChunk();

  {
    std::lock_guard<std::mutex> lock(m_sourceLock);
    m_sourceReady = false;
    if (m_chunkSource) {
      m_chunkSource->Terminate();
      delete m_chunkSource;
      m_chunkSource = nullptr;
    }
  }

  if (m_voxelsReady) {
    DeinitializeVoxels();
    m_voxelsReady = false;
  }

  if (m_heap) {
    m_heap->Terminate();
    delete m_heap;
    m_heap = nullptr;
  }
}

// Initialize chunk renderer contexts.
META_REGISTER_FUNCTION_MEMBER(VoxelChunkBarn, OnLevelLoad)
META_DATA_MEMBER_FUNCTION(VoxelChunkBarn, OnLevelLoad, ArgName, "(levelName)")
void VoxelChunkBarn::OnLevelLoad(
  Game *game,
  ResourceManager *resources,
  Scene *scene,
  MaterialDefBarn *materialDefBarn,
  cstring levelName
) {
  if (!m_hasParams || !m_voxelsReady || !m_chunkSource)
    return;

  // Set chunk generator params.
  m_seed = m_params.seed;
  m_viewDistance = std::max(1u, std::min(m_params.viewDistance, kMaxViewDistance));

  // Force the next update to build the chunk set for the new level.
  m_lastPos = {-2147483647, -2147483647};

  // Chunks within view distance + 1 may briefly coexist while the avatar
  // moves between chunks, so size the buffers for that wider set.
  m_chunkCapacity = ms_CountChunks(m_viewDistance + 1) + 8;

  // Initialize chunk source.
  {
    std::lock_guard<std::mutex> lock(m_sourceLock);
    m_chunkSource->Initialize(m_seed);
    m_sourceReady = true;
  }

  const u32 maxVertices = m_chunkCapacity * VoxelChunkMesh::kMaxVertices;
  const u32 maxIndices = m_chunkCapacity * VoxelChunkMesh::kMaxIndices;

  // Initialize renderer.
  m_depth.Initialize(
    "Infdev_VOXEL_DEPTH",
    TerrainDepthVertex::kTypes,
    TerrainDepthVertex::kAttrs,
    TerrainDepthVertex::kNumAttrs,
    resources,
    "TerrainDepth",
    scene->GetRenderListByName("TerrainDepth"),
    m_heap,
    m_chunkCapacity,
    maxVertices,
    maxIndices
  );
  m_mats.Initialize(
    "Infdev_VOXEL_GRASSSH",
    GrassShVertex::kTypes,
    GrassShVertex::kAttrs,
    GrassShVertex::kNumAttrs,
    resources,
    "GrassSh",
    scene->GetRenderListByName("TerrainMats"),
    m_heap,
    m_chunkCapacity,
    maxVertices,
    maxIndices
  );

  // Add material uniforms.
  MaterialDefBarn::SetMaterialShaderUniforms(
    m_mats.render.GetPipelineInstance(),
    materialDefBarn->GetDef(kMaterial_Grass),
    resources);

  m_levelActive = true;
}

// Deinitialize chunk renderer contexts.
META_REGISTER_FUNCTION_MEMBER(VoxelChunkBarn, OnLevelUnload)
META_DATA_MEMBER_FUNCTION(VoxelChunkBarn, OnLevelUnload, ArgName, "(levelName)")
void VoxelChunkBarn::OnLevelUnload(
  cstring levelName
) {
  if (m_levelActive) {
    m_levelActive = false;

    // Deinitialize renderer.
    m_depth.Terminate();
    m_mats.Terminate();

    // Remove all client chunks.
    m_ClearClientChunk();
  }

  {
    std::lock_guard<std::mutex> lock(m_sourceLock);
    m_sourceReady = false;
    if (m_chunkSource)
      m_chunkSource->Terminate();
  }

  m_hasParams = false;
}

// Load and unload chunks by the position of local avatar.
META_REGISTER_FUNCTION_MEMBER(VoxelChunkBarn, Update)
META_DATA_MEMBER_FUNCTION(VoxelChunkBarn, Update, ArgName, "(levelName)")
void VoxelChunkBarn::Update(
  Game *game,
  AvatarBarn *avatarBarn,
  cstring levelName
) {
  if (!m_levelActive)
    return;

  Avatar *avatar = avatarBarn->TryGetLocalAvatar(false);
  if (!avatar)
    return;

  Vector4 avatarPos = avatar->GetTransform()[3];

  ChunkPos avatarChunkPos = {
    (i32)floorf(avatarPos->x / (f32)kChunkSize),
    (i32)floorf(avatarPos->z / (f32)kChunkSize)
  };

  // Only update chunks when avatar position changes.
  if (m_lastPos != avatarChunkPos) {
    m_lastPos = avatarChunkPos;

    const i32 r = (i32)m_viewDistance;
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

  // Build collision for the chunks directly around the avatar.
  {
    std::lock_guard<std::shared_mutex> lock(m_lock);
    for (auto &pair: m_clientChunks) {
      auto &chunk = pair.second;

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
META_REGISTER_FUNCTION_MEMBER(VoxelChunkBarn, BuildScene)
META_DATA_MEMBER_FUNCTION(VoxelChunkBarn, BuildScene, ArgName, "(levelName)")
void VoxelChunkBarn::BuildScene(
  cstring levelName
) {
  if (!m_levelActive)
    return;

  void *depthVtx = m_depth.MapVtxBuffer();
  void *matVtx = m_mats.MapVtxBuffer();
  void *depthIdx = m_depth.MapIdxBuffer();
  void *matIdx = m_mats.MapIdxBuffer();

  if (!depthVtx || !matVtx || !depthIdx || !matIdx) {
    if (depthVtx) m_depth.UnmapVtxBuffer();
    if (matVtx) m_mats.UnmapVtxBuffer();
    if (depthIdx) m_depth.UnmapIdxBuffer();
    if (matIdx) m_mats.UnmapIdxBuffer();
    return;
  }

  m_depth.ClearRenderChunk();
  m_mats.ClearRenderChunk();

  const u32 maxVertices = m_chunkCapacity * VoxelChunkMesh::kMaxVertices;
  const u32 maxIndices = m_chunkCapacity * VoxelChunkMesh::kMaxIndices;

  u32 vtxOffset = 0;
  u32 idxOffset = 0;
  u32 chunkCount = 0;

  // Append all generated chunks to the render buffers.
  {
    std::shared_lock<std::shared_mutex> lock(m_lock);

    for (const auto &pair: m_clientChunks) {
      const auto &chunk = pair.second;
      if (!chunk.HasMesh() || chunk.m_mesh.IsEmpty())
        continue;

      const u32 vtxCount = (u32)chunk.m_mesh.materialVertices.size();
      const u32 idxCount = (u32)chunk.m_mesh.indices.size();

      if (
        chunkCount >= m_chunkCapacity
        || vtxOffset + vtxCount > maxVertices
        || idxOffset + idxCount > maxIndices
      )
        break;

      memcpy(
        (u08 *)depthVtx + (size_t)vtxOffset * sizeof(TerrainDepthVertex),
        chunk.m_mesh.depthVertices.data(),
        sizeof(TerrainDepthVertex) * vtxCount);
      memcpy(
        (u08 *)matVtx + (size_t)vtxOffset * sizeof(TerrainMaterialVertex),
        chunk.m_mesh.materialVertices.data(),
        sizeof(TerrainMaterialVertex) * vtxCount);

      memcpy(
        (u08 *)depthIdx + (size_t)idxOffset * sizeof(u16),
        chunk.m_mesh.indices.data(),
        sizeof(u16) * idxCount);
      memcpy(
        (u08 *)matIdx + (size_t)idxOffset * sizeof(u16),
        chunk.m_mesh.indices.data(),
        sizeof(u16) * idxCount);

      m_depth.AddRenderChunk(idxOffset, idxCount, (i32)vtxOffset);
      m_mats.AddRenderChunk(idxOffset, idxCount, (i32)vtxOffset);

      vtxOffset += vtxCount;
      idxOffset += idxCount;
      ++chunkCount;
    }
  }

  // Unmap buffers.
  m_depth.UnmapVtxBuffer();
  m_mats.UnmapVtxBuffer();
  m_depth.UnmapIdxBuffer();
  m_mats.UnmapIdxBuffer();

  // Queue present.
  m_depth.Queue();
  m_mats.Queue();
}

META_REGISTER_FUNCTION_MEMBER(VoxelChunkBarn, CreateParams)
META_DATA_MEMBER_FUNCTION(VoxelChunkBarn, CreateParams, ObjectFactory_CreatesType, "VoxelChunkSourceParams")
VoxelChunkSourceParams *VoxelChunkBarn::CreateParams(
  MetaClass *mc
) {
  Assert(!m_hasParams);
  m_hasParams = true;
  return &m_params;
}

void VoxelChunkBarn::m_AddClientChunk(
  const ChunkPos &pos
) {
  std::lock_guard<std::shared_mutex> lock(m_lock);

  auto result = m_clientChunks.try_emplace(pos);
  if (!result.second)
    return;

  result.first->second.Initialize(m_collisionGeoBarn, pos);
}

void VoxelChunkBarn::m_RemoveClientChunk(
  const ChunkPos &pos
) {
  std::lock_guard<std::shared_mutex> lock(m_lock);

  auto it = m_clientChunks.find(pos);
  if (it == m_clientChunks.end())
    return;

  it->second.Terminate();
  m_clientChunks.erase(it);
}

void VoxelChunkBarn::m_ClearClientChunk() {
  std::lock_guard<std::shared_mutex> lock(m_lock);

  for (auto &it: m_clientChunks)
    it.second.Terminate();

  m_clientChunks.clear();
}

bool VoxelChunkBarn::m_IsClientChunkAdded(
  const ChunkPos &pos
) {
  std::shared_lock<std::shared_mutex> lock(m_lock);
  return m_clientChunks.find(pos) != m_clientChunks.end();
}
