#include <chrono>
#include <thread>
#include <cstring>
#include <cmath>
#include <includes/htmodloader.h>
#include <Memory/Heap.hpp>
#include "sky/skyGfx.hpp"
#include "sky/skyScene.hpp"
#include "sky/skyMaterialDefBarn.hpp"
#include "sky/skyTypePlaceholders.hpp"
#include "render/vertexArrayElements.hpp"
#include "world/heightMapChunkBarn.hpp"

HEAP_TAG_REGISTER(tag_HeightMapChunk)

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
  f32 heights[17][17];
  m_noise->GetRegion(
    (f32 *)heights,
    chunk->GetPos().x * 16,
    chunk->GetPos().z * 16,
    17, 17,
    1368.824f, 1368.824f);
  for (u32 z = 0; z < 17; z++) {
    for (u32 x = 0; x < 17; x++) {
      heights[x][z] *= 1e-4f;
      heights[x][z] += 2.0f;
    }
  }
  chunk->SetHeights((f32 *)heights);
}

// ----------------------------------------------------------------------------
// [SECTION] HeightMapChunkBarn::RenderData
// ----------------------------------------------------------------------------

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
    sizeof(u16) * kChunkIdxCount * kMaxChunks,
    tag_HeightMapChunk,
    alignof(u16));

  // Vertices and indices (x-major order):
  //       (x)      (x+1)
  // (z)    v0 ------ v1
  //        |      /  |
  //        |    /    |
  //        |  /      |
  // (z+1)  v2 ------ v3
  //
  // TODO: Optimize with SIMD.
  for (u32 c = 0; c < kMaxChunks; c++) {
    u32 chunkIdxOffset = c * kChunkIdxCount;
    for (u32 z = 0; z < 16; z++) {
      for (u32 x = 0; x < 16; x++) {
        u32 quadIdx = z * 16 + x;
        u32 baseIdx = quadIdx * 6;

        u32 v0 = x * 17 + z;
        u32 v1 = v0 + 17;
        u32 v2 = v0 + 1;
        u32 v3 = v1 + 1;

        // Triangle 1: v0, v2, v1
        indices[chunkIdxOffset + baseIdx + 0] = v0;
        indices[chunkIdxOffset + baseIdx + 1] = v2;
        indices[chunkIdxOffset + baseIdx + 2] = v1;

        // Triangle 2: v1, v2, v3
        indices[chunkIdxOffset + baseIdx + 3] = v1;
        indices[chunkIdxOffset + baseIdx + 4] = v2;
        indices[chunkIdxOffset + baseIdx + 5] = v3;
      }
    }
  }

  data.AddIndexBuffer(
    0,
    kGfxType_SHORT,
    kGfxBind_UploadSingle,
    kChunkIdxCount * kMaxChunks,
    indices);
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
// [SECTION] HeightMapChunkBarn
// ----------------------------------------------------------------------------

META_REGISTER_CLASS(HeightMapChunkBarn, MetaClassImpl<Module>::Must_call_META_REGISTER_CLASS)

// Chunk generator thread.
void HeightMapChunkBarn::ms_ChunkUpdateThread(
  HeightMapChunkBarn *heightMapChunkBarn
) {
  while (heightMapChunkBarn->m_running) {
    ChunkPos nextChunkPos = {0, 0};
    bool hasChunk = false;

    {
      std::shared_lock<std::shared_mutex> lock(heightMapChunkBarn->m_lock);
      if (!heightMapChunkBarn->m_queuedChunks.empty()) {
        nextChunkPos = *heightMapChunkBarn->m_queuedChunks.begin();
        hasChunk = true;
      }
    }

    if (hasChunk) {
      HeightMapChunk *chunk = new HeightMapChunk();
      chunk->Initialize(nextChunkPos);
      heightMapChunkBarn->m_chunkSource->LoadChunk(chunk);
      heightMapChunkBarn->m_LoadChunk(chunk);
      heightMapChunkBarn->m_DequeueChunk(nextChunkPos);
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }
}

// Initialize the object.
META_REGISTER_FUNCTION_MEMBER(HeightMapChunkBarn, Initialize)
void HeightMapChunkBarn::Initialize() {
  // Create Heap object.
  m_heightMapVertexHeap = new Heap();
  m_heightMapVertexHeap->Initialize(
    new char[0x2000000],
    0x2000000, // ~33MiB
    "HeightMapChunkBarn",
    true,
    false
  );

  // Initialize world generator.
  m_chunkSource->Initialize(m_seed);
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

  {
    std::lock_guard<std::shared_mutex> lock(m_lock);
    for (auto &pair : m_loadedChunks) {
      delete pair.second;
    }
    m_loadedChunks.clear();
    m_queuedChunks.clear();
  }

  m_chunkSource->Terminate();

  // NOTE: A more stable implementation would be to first take out `m_base` and
  // release it. However, many other objects in Sky (at least in my view) do not
  // do this, so let's just release the `Heap` itself.
  m_heightMapVertexHeap->Terminate();
  delete m_heightMapVertexHeap;
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
  if (strcmp(levelName, kTestInfdevLevel))
    return;

  // Initialize renderer.
  m_depth.Initialize(
    "Infdev_DEPTH",
    TerrainDepthVertex::kTypes,
    TerrainDepthVertex::kAttrs,
    TerrainDepthVertex::kNumAttrs,
    resources,
    "TerrainDepth",
    scene->GetRenderListByName("TerrainDepth"),
    m_heightMapVertexHeap
  );
  m_mats.Initialize(
    "Infdev_GRASSSH",
    GrassShVertex::kTypes,
    GrassShVertex::kAttrs,
    GrassShVertex::kNumAttrs,
    resources,
    "GrassSh",
    scene->GetRenderListByName("TerrainMats"),
    m_heightMapVertexHeap
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
  if (strcmp(levelName, kTestInfdevLevel))
    return;

  // Deinitialize renderer.
  m_depth.Terminate();
  m_mats.Terminate();
}

// Load and unload chunks by the position of local avatar.
META_REGISTER_FUNCTION_MEMBER(HeightMapChunkBarn, Update)
META_DATA_MEMBER_FUNCTION(HeightMapChunkBarn, Update, ArgName, "(levelName)")
void HeightMapChunkBarn::Update(
  Game *game,
  AvatarBarn *avatarBarn,
  cstring levelName
) {
  if (strcmp(levelName, kTestInfdevLevel))
    return;

  ChunkPos avatarChunkPos = {0, 0};

  if (m_lastPos == avatarChunkPos)
    return;

  m_lastPos = avatarChunkPos;

  i32 viewDist = (i32)m_viewDistance;
  std::vector<ChunkPos> chunksToUnload;

  {
    std::shared_lock<std::shared_mutex> lock(m_lock);
    for (const auto &pair: m_loadedChunks) {
      const ChunkPos &chunkPos = pair.first;
      i32 dx = chunkPos.x - avatarChunkPos.x;
      i32 dz = chunkPos.z - avatarChunkPos.z;

      if (dx * dx + dz * dz > viewDist * viewDist) {
        chunksToUnload.push_back(chunkPos);
      }
    }
  }

  for (const ChunkPos &pos: chunksToUnload) {
    m_DequeueChunk(pos);
    m_UnloadChunk(pos);
  }

  for (i32 dx = -viewDist; dx <= viewDist; ++dx) {
    for (i32 dz = -viewDist; dz <= viewDist; ++dz) {
      if (dx * dx + dz * dz > viewDist * viewDist)
        continue;

      ChunkPos chunkPos = {avatarChunkPos.x + dx, avatarChunkPos.z + dz};

      if (!m_IsChunkLoaded(chunkPos) && !m_IsChunkQueued(chunkPos)) {
        m_QueueChunk(chunkPos);
      }
    }
  }
}

// Build vertices for visible chunks.
META_REGISTER_FUNCTION_MEMBER(HeightMapChunkBarn, BuildScene)
META_DATA_MEMBER_FUNCTION(HeightMapChunkBarn, BuildScene, ArgName, "(levelName)")
void HeightMapChunkBarn::BuildScene(
  cstring levelName
) {
  if (strcmp(levelName, kTestInfdevLevel))
    return;

  std::shared_lock<std::shared_mutex> lock(m_lock);

  // Map vertex and index buffers.
  TerrainDepthVertex *depthVtx = (TerrainDepthVertex *)m_depth.MapVtxBuffer();
  GrassShVertex *grassVtx = (GrassShVertex *)m_mats.MapVtxBuffer();

  // Return if failed to map buffers.
  if (!grassVtx || !depthVtx) {
    if (grassVtx) m_mats.UnmapVtxBuffer();
    if (depthVtx) m_depth.UnmapVtxBuffer();
    return;
  }

  u32 currentVtxOffset = 0
    , currentIdxOffset = 0
    , processedChunks = 0;
  for (const auto &pair: m_loadedChunks) {
    const ChunkPos &chunkPos = pair.first;
    const HeightMapChunk *chunk = pair.second;
    // Get height data.
    const f32 *heights = chunk->GetHeights();

    // Generate vertices with positions and normals.
    for (u32 z = 0; z < 17; ++z) {
      for (u32 x = 0; x < 17; ++x) {
        // Note: GetRegion stores data in x-major order: buffer[x * zSize + z]
        u32 idx = x * 17 + z;
        f32 height = heights[idx];

        // World position.
        f32 worldX = chunkPos.x * 16.0f + x;
        f32 worldZ = chunkPos.z * 16.0f + z;

        GrassShVertex vtx = {worldX, height, worldZ};
        TerrainDepthVertex vtx2 = {worldX, height, worldZ};

        // Calculate normal using neighboring heights.
        // Data is stored in x-major order: heights[x * 17 + z]
        f32 hL = (x > 0)  ? heights[(x - 1) * 17 + z] : height;
        f32 hR = (x < 16) ? heights[(x + 1) * 17 + z] : height;
        f32 hD = (z > 0)  ? heights[x * 17 + (z - 1)] : height;
        f32 hU = (z < 16) ? heights[x * 17 + (z + 1)] : height;

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
        vtx.a_light0 = 0x7F7F7F7F;
        vtx.a_light1 = 0x0000FFB3;
        vtx.a_light2 = 0xFF80FF80;
        grassVtx[currentVtxOffset + idx] = vtx;
        depthVtx[currentVtxOffset + idx] = vtx2;
      }
    }

    currentVtxOffset += kChunkVtxCount;
    processedChunks++;

    if (processedChunks >= 8)
      break;
  }

  // Unmap buffers.
  m_depth.UnmapVtxBuffer();
  m_mats.UnmapVtxBuffer();

  m_depth.ClearRenderChunk();
  m_mats.ClearRenderChunk();

  for (i32 i = 0; i < processedChunks; i++) {
    m_depth.AddRenderChunk(kChunkIdxCount * i, kChunkIdxCount, kChunkVtxCount * i);
    m_mats.AddRenderChunk(kChunkIdxCount * i, kChunkIdxCount, kChunkVtxCount * i);
  }

  m_depth.Queue();
  m_mats.Queue();
}

//META_REGISTER_FUNCTION_MEMBER(HeightMapChunkBarn, CreateParams)
//META_DATA_MEMBER_FUNCTION(HeightMapChunkBarn, CreateParams, ObjectFactory_CreatesType, "HeightMapChunkSourceParams")
//HeightMapChunkSourceParams *HeightMapChunkBarn::CreateParams(
//  MetaClass *mc
//) {
//
//}

bool HeightMapChunkBarn::m_IsChunkQueued(
  const ChunkPos &pos
) {
  std::shared_lock<std::shared_mutex> lock(m_lock);
  return m_queuedChunks.find(pos) != m_queuedChunks.end();
}

void HeightMapChunkBarn::m_QueueChunk(
  const ChunkPos &pos
) {
  std::lock_guard<std::shared_mutex> lock(m_lock);
  m_queuedChunks.insert(pos);
}

void HeightMapChunkBarn::m_DequeueChunk(
  const ChunkPos &pos
) {
  std::lock_guard<std::shared_mutex> lock(m_lock);
  m_queuedChunks.erase(pos);
}

bool HeightMapChunkBarn::m_IsChunkLoaded(
  const ChunkPos &pos
) {
  std::shared_lock<std::shared_mutex> lock(m_lock);
  return m_loadedChunks.find(pos) != m_loadedChunks.end();
}

void HeightMapChunkBarn::m_LoadChunk(
  const HeightMapChunk *chunk
) {
  std::lock_guard<std::shared_mutex> lock(m_lock);
  if (m_loadedChunks.find(chunk->GetPos()) != m_loadedChunks.end())
    return;
  m_loadedChunks[chunk->GetPos()] = chunk;
}

void HeightMapChunkBarn::m_UnloadChunk(
  const ChunkPos &pos
) {
  std::lock_guard<std::shared_mutex> lock(m_lock);
  auto it = m_loadedChunks.find(pos);
  if (it == m_loadedChunks.end())
    return;
  const HeightMapChunk *chunk = it->second;
  m_loadedChunks.erase(it);

  delete chunk;
}
