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
META_DATA_MEMBER_VARIABLE(VoxelChunkSourceParams, seed, Tool_DefaultValue, "1337")

META_REGISTER_SIMPLE_MEMBER(VoxelChunkSourceParams, viewDistance)
META_DATA_MEMBER_VARIABLE(VoxelChunkSourceParams, viewDistance, Tool_DefaultValue, "4")
META_DATA_MEMBER_VARIABLE(VoxelChunkSourceParams, viewDistance, Tool_MinMaxValues, "(1, 16)")

// ----------------------------------------------------------------------------
// [SECTION] VoxelChunkSource
// ----------------------------------------------------------------------------

void VoxelChunkSource::Initialize(
  u32 seed
) {
  Terminate();
  m_surface = new ValueNoiseSurface(seed);
}

void VoxelChunkSource::Terminate() {
  if (!m_surface)
    return;

  delete m_surface;
  m_surface = nullptr;
}

// The polygonizer returns Y-up positions: output = (gridX, gridZ, gridY).
// The analytic field is defined in grid coordinates (Z-up), so convert
// positions and gradients when crossing the boundary.
static f32 s_SurfaceDistance(
  const ValueNoiseSurface &surface,
  f32 x,
  f32 y,
  f32 z
) {
  return surface.Distance(x, z, y);
}

static ValueNoiseVector s_SurfaceGradient(
  const ValueNoiseSurface &surface,
  f32 x,
  f32 y,
  f32 z
) {
  const ValueNoiseVector gradient = surface.Gradient(x, z, y);
  return ValueNoiseVector{gradient.x, gradient.z, gradient.y};
}

static ValueNoiseVector s_SurfaceNormal(
  const ValueNoiseSurface &surface,
  f32 x,
  f32 y,
  f32 z
) {
  const ValueNoiseVector gradient = s_SurfaceGradient(surface, x, y, z);
  const f32 lengthSquared = gradient.x * gradient.x
    + gradient.y * gradient.y
    + gradient.z * gradient.z;

  if (lengthSquared < 1e-12f)
    return ValueNoiseVector{0.0f, 1.0f, 0.0f};

  const f32 inverse = 1.0f / sqrtf(lengthSquared);
  return ValueNoiseVector{
    gradient.x * inverse,
    gradient.y * inverse,
    gradient.z * inverse
  };
}

// The polygonizer works on an 8-bit distance field, so the raw mesh has small
// quantized terraces. A few Newton steps along the analytic field's gradient
// move each vertex onto the exact zero level set. This works for the 3D caves
// and overhangs too, unlike a heightfield-only Y snap.
static void s_ProjectToSurface(
  const ValueNoiseSurface &surface,
  TerrainMaterialVertex *vertex
) {
  f32 x = vertex->a_position[0];
  f32 y = vertex->a_position[1];
  f32 z = vertex->a_position[2];

  for (i32 iteration = 0; iteration < 3; ++iteration) {
    const f32 distance = s_SurfaceDistance(surface, x, y, z);
    if (fabsf(distance) < 0.005f)
      break;

    const ValueNoiseVector gradient = s_SurfaceGradient(surface, x, y, z);
    const f32 gradientLengthSquared = gradient.x * gradient.x
      + gradient.y * gradient.y
      + gradient.z * gradient.z;
    if (gradientLengthSquared < 1e-8f)
      break;

    f32 stepX = gradient.x * (distance / gradientLengthSquared);
    f32 stepY = gradient.y * (distance / gradientLengthSquared);
    f32 stepZ = gradient.z * (distance / gradientLengthSquared);

    // The distance field is clamped, so guard against a bad extrapolation.
    const f32 stepLength = sqrtf(stepX * stepX + stepY * stepY + stepZ * stepZ);
    const f32 maxStep = 1.0f;
    if (stepLength > maxStep) {
      const f32 scale = maxStep / stepLength;
      stepX *= scale;
      stepY *= scale;
      stepZ *= scale;
    }

    x -= stepX;
    y -= stepY;
    z -= stepZ;
  }

  vertex->a_position[0] = x;
  vertex->a_position[1] = y;
  vertex->a_position[2] = z;
}

// Returns true when the triangle is wound counter-clockwise as seen from the
// side its vertex normals point at. The polygonizer mirrors the winding when
// it swaps Y/Z, so it has to be corrected for Sky's renderer.
static bool s_IsFrontFacing(
  const TerrainMaterialVertex &v0,
  const TerrainMaterialVertex &v1,
  const TerrainMaterialVertex &v2,
  const ValueNoiseVector &n0,
  const ValueNoiseVector &n1,
  const ValueNoiseVector &n2
) {
  f32 e1x = v1.a_position[0] - v0.a_position[0];
  f32 e1y = v1.a_position[1] - v0.a_position[1];
  f32 e1z = v1.a_position[2] - v0.a_position[2];

  f32 e2x = v2.a_position[0] - v0.a_position[0];
  f32 e2y = v2.a_position[1] - v0.a_position[1];
  f32 e2z = v2.a_position[2] - v0.a_position[2];

  f32 cx = e1y * e2z - e1z * e2y;
  f32 cy = e1z * e2x - e1x * e2z;
  f32 cz = e1x * e2y - e1y * e2x;

  f32 nx = n0.x + n1.x + n2.x;
  f32 ny = n0.y + n1.y + n2.y;
  f32 nz = n0.z + n1.z + n2.z;

  return cx * nx + cy * ny + cz * nz >= 0.0f;
}

bool VoxelChunkSource::Generate(
  const VoxelChunkPos &pos,
  VoxelChunkMesh *mesh
) {
  if (!m_surface || !mesh)
    return false;

  mesh->Clear();

  // ChunkKey is in output space (Y-up); the grid expects Z-up coordinates.
  // The grid is larger than the core chunk: one block of padding on every
  // side makes the boundary cells use the neighbour's real samples instead
  // of the clamped edge of the grid. Without this, adjacent chunks produce
  // slightly different boundary geometry and visible cracks.
  const f32 startX = (f32)(pos.x * (i32)kChunkSize - (i32)kChunkPadding);
  const f32 startY = (f32)(pos.z * (i32)kChunkSize - (i32)kChunkPadding);
  const f32 startZ = (f32)(pos.y * (i32)kChunkSize - (i32)kChunkPadding);

  Voxels::Grid *grid = Voxels::Grid::Create(
    kGridSize, kGridSize, kGridSize,
    startX, startY, startZ, 1.0f,
    m_surface);

  if (!grid) {
    HTTellText("§c[VoxelChunkBarn] Failed to create grid at (%d, %d, %d)", pos.x, pos.y, pos.z);
    return false;
  }

  Voxels::Polygonizer polygonizer;
  Voxels::PolygonSurface *surface = polygonizer.Execute(*grid, &m_materialMap);
  if (!surface) {
    HTTellText("§c[VoxelChunkBarn] Failed to polygonize chunk at (%d, %d, %d)", pos.x, pos.y, pos.z);
    grid->Destroy();
    return false;
  }

  // The polygonizer returns padded-grid-local positions; shift them into
  // world space before projection so the analytic field is evaluated at the
  // correct global coordinates.
  const f32 worldOffsetX = startX;
  const f32 worldOffsetY = startZ;
  const f32 worldOffsetZ = startY;

  // Keep only the blocks that belong to the core chunk; the outer 16-voxel
  // ring was only sampled to make the boundary cells correct.
  const f32 minAllowed = (f32)kChunkPadding;
  const f32 maxAllowed = (f32)(kChunkPadding + kChunkSize - 16);

  std::vector<ValueNoiseVector> vertexNormals;
  vertexNormals.reserve(VoxelChunkMesh::kMaxVertices);

  // Only LOD 0 is rendered: the outer LOD levels would require transition
  // meshes, which are not used here.
  if (surface->GetLevelsCount() > 0) {
    const u32 blockCount = (u32)surface->GetBlocksForLevelCount(0);

    for (u32 blockIdx = 0; blockIdx < blockCount; ++blockIdx) {
      const Voxels::BlockPolygons *block = surface->GetBlockForLevel(0, blockIdx);
      if (!block)
        continue;

      const Voxels::float3 minCorner = block->GetMinimalCorner();
      if (minCorner.x < minAllowed || minCorner.x > maxAllowed
        || minCorner.y < minAllowed || minCorner.y > maxAllowed
        || minCorner.z < minAllowed || minCorner.z > maxAllowed)
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

        f32 wx = worldOffsetX + src.Position.x;
        f32 wy = worldOffsetY + src.Position.y;
        f32 wz = worldOffsetZ + src.Position.z;

        TerrainMaterialVertex mvtx(wx, wy, wz);

        // Snap the vertex to the exact zero level set and use the analytic
        // gradient as its normal. This removes the 8-bit terraces and keeps
        // normals continuous across chunk boundaries.
        s_ProjectToSurface(*m_surface, &mvtx);
        const ValueNoiseVector normal = s_SurfaceNormal(
          *m_surface,
          mvtx.a_position[0],
          mvtx.a_position[1],
          mvtx.a_position[2]);

        mvtx.a_normal = (i32)R8G8B8A8_SNORM(normal.x, normal.y, normal.z, 1.0f);
        mvtx.a_light0 = 0x407F7F7F;
        mvtx.a_light1 = 0x0000FFB3;
        mvtx.a_light2 = 0xFF80FF80;

        TerrainDepthVertex dvtx(
          mvtx.a_position[0],
          mvtx.a_position[1],
          mvtx.a_position[2]);

        mesh->depthVertices.push_back(dvtx);
        mesh->materialVertices.push_back(mvtx);
        vertexNormals.push_back(normal);

        mesh->min = Vector4::minval(mesh->min, Vector4(
          mvtx.a_position[0], mvtx.a_position[1], mvtx.a_position[2], 0.0f));
        mesh->max = Vector4::maxval(mesh->max, Vector4(
          mvtx.a_position[0], mvtx.a_position[1], mvtx.a_position[2], 0.0f));
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

        if (!s_IsFrontFacing(
          mesh->materialVertices[baseVertex + i0],
          mesh->materialVertices[baseVertex + i1],
          mesh->materialVertices[baseVertex + i2],
          vertexNormals[baseVertex + i0],
          vertexNormals[baseVertex + i1],
          vertexNormals[baseVertex + i2]))
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
  const VoxelChunkPos &pos
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
    // Pick the closest chunk that still needs a mesh, so the terrain around
    // the avatar appears before the outer ring. The selected position is
    // stable while generation runs; it is looked up again before installing.
    VoxelChunkPos pos = {};
    bool hasWork = false;
    i64 bestDistance = 0;
    {
      std::shared_lock<std::shared_mutex> lock(voxelChunkBarn->m_lock);
      const ChunkPos &center = voxelChunkBarn->m_generationCenter;
      for (auto &it: voxelChunkBarn->m_clientChunks) {
        if (it.second.HasMesh())
          continue;

        const VoxelChunkPos &chunkPos = it.second.GetPos();
        const i64 dx = chunkPos.x - center.x;
        const i64 dz = chunkPos.z - center.z;
        const i64 distance = dx * dx + dz * dz;

        if (!hasWork || distance < bestDistance) {
          pos = chunkPos;
          bestDistance = distance;
          hasWork = true;
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
              "§e[VoxelChunkBarn] Chunk (%d, %d, %d) exceeded the mesh budget and was clipped",
              pos.x, pos.y, pos.z);
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
  m_seed = (u32)m_params.seed;
  m_viewDistance = std::max(1u, std::min(m_params.viewDistance, kMaxViewDistance));

  // Force the next update to build the chunk set for the new level.
  m_lastPos = {-2147483647, -2147483647};

  // Every chunk column contains a fixed number of vertical layers, and the
  // chunk set is rebuilt exactly on movement, so this is an upper bound.
  m_chunkCapacity = ms_CountChunks(m_viewDistance) * (u32)kVerticalChunkCount + 8;

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

  const i32 avatarChunkX = (i32)floorf(avatarPos->x / (f32)kChunkSize);
  const i32 avatarChunkY = (i32)floorf(avatarPos->y / (f32)kChunkSize);
  const i32 avatarChunkZ = (i32)floorf(avatarPos->z / (f32)kChunkSize);

  ChunkPos avatarChunkPos = {avatarChunkX, avatarChunkZ};

  // Only update chunks when the horizontal avatar position changes.
  if (m_lastPos != avatarChunkPos) {
    m_lastPos = avatarChunkPos;

    {
      std::lock_guard<std::shared_mutex> lock(m_lock);
      m_generationCenter = avatarChunkPos;
    }

    const i32 r = (i32)m_viewDistance;
    std::vector<VoxelChunkPos> chunksToUnload;

    // Unload chunks outside the desired column/layer set.
    {
      std::shared_lock<std::shared_mutex> lock(m_lock);
      for (const auto &pair: m_clientChunks) {
        const VoxelChunkPos &chunkPos = pair.first;
        i32 dx = chunkPos.x - avatarChunkX
          , dz = chunkPos.z - avatarChunkZ;

        if (dx * dx + dz * dz > r * r
          || chunkPos.y < kFirstChunkY
          || chunkPos.y > kLastChunkY)
          chunksToUnload.push_back(chunkPos);
      }
    }

    for (const VoxelChunkPos &pos: chunksToUnload)
      m_RemoveClientChunk(pos);

    // Add chunks.
    for (i32 dx = -r; dx <= r; ++dx) {
      for (i32 dz = -r; dz <= r; ++dz) {
        if (dx * dx + dz * dz > r * r)
          continue;

        for (i32 y = kFirstChunkY; y <= kLastChunkY; ++y) {
          VoxelChunkPos chunkPos = {avatarChunkX + dx, y, avatarChunkZ + dz};

          if (!m_IsClientChunkAdded(chunkPos))
            m_AddClientChunk(chunkPos);
        }
      }
    }
  }

  // Build collision for the chunks directly around the avatar.
  {
    const i32 collisionChunkY = std::max(kFirstChunkY, std::min(avatarChunkY, kLastChunkY));

    std::lock_guard<std::shared_mutex> lock(m_lock);
    for (auto &pair: m_clientChunks) {
      auto &chunk = pair.second;
      const VoxelChunkPos &chunkPos = chunk.GetPos();

      i32 dx = chunkPos.x - avatarChunkX
        , dy = chunkPos.y - collisionChunkY
        , dz = chunkPos.z - avatarChunkZ;

      if (-1 <= dx && dx <= 1 && -1 <= dy && dy <= 1 && -1 <= dz && dz <= 1)
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
  const VoxelChunkPos &pos
) {
  std::lock_guard<std::shared_mutex> lock(m_lock);

  auto result = m_clientChunks.try_emplace(pos);
  if (!result.second)
    return;

  result.first->second.Initialize(m_collisionGeoBarn, pos);
}

void VoxelChunkBarn::m_RemoveClientChunk(
  const VoxelChunkPos &pos
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
  const VoxelChunkPos &pos
) {
  std::shared_lock<std::shared_mutex> lock(m_lock);
  return m_clientChunks.find(pos) != m_clientChunks.end();
}
