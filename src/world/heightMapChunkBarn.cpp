#include <chrono>
#include <thread>
#include <cstring>
#include "sky/skyGfx.hpp"
#include "sky/skyScene.hpp"
#include "sky/skyTypePlaceholders.hpp"
#include "render/vertexArrayElements.hpp"
#include "world/heightMapChunkBarn.hpp"

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
    chunk->GetPos().x,
    chunk->GetPos().z,
    17, 17,
    1368.824, 1368.824);
  chunk->SetHeights((f32 *)heights);
}

// ----------------------------------------------------------------------------
// [SECTION] HeightMapChunkBarn
// ----------------------------------------------------------------------------

META_REGISTER_CLASS(HeightMapChunkBarn, MetaClassImpl<Module>::Must_call_META_REGISTER_CLASS)

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

META_REGISTER_FUNCTION_MEMBER(HeightMapChunkBarn, Initialize)
void HeightMapChunkBarn::Initialize() {
  // Initialize world generator.
  m_chunkSource->Initialize(m_seed);
  m_running = true;
  m_chunkUpdateThread = std::thread(ms_ChunkUpdateThread, this);
}

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
}

META_REGISTER_FUNCTION_MEMBER(HeightMapChunkBarn, OnLevelLoad)
META_DATA_MEMBER_FUNCTION(HeightMapChunkBarn, OnLevelLoad, ArgName, "(levelName)")
void HeightMapChunkBarn::OnLevelLoad(
  ResourceManager *resources,
  Scene *scene,
  cstring levelName
) {
  if (strcmp(levelName, "Infdev_TestLevel"))
    return;
  // Initialize renderer.

  // 17 * 17 vertices per chunk.
  m_vertexData.BeginDefinition("Infdev", 17 * 17 * 169);
  m_vertexData.AddVertexBuffer(
    0,
    GrassShVertex::kTypes,
    GrassShVertex::kAttrs,
    GrassShVertex::kNumAttrs,
    kGfxBind_UploadTriple,
    0,
    nullptr);
  m_vertexData.AddIndexBuffer(
    0,
    kGfxType_UINT,
    kGfxBind_UploadTriple,
    3 * 2 * 16 * 16 * 169,
    0);
  m_vertexData.EndDefinition();

  RenderList *rl = scene->GetRenderListByName("Opaque");
  m_vertexRender.Initialize(
    &m_vertexData,
    resources,
    "GrassSh",
    rl,
    0,
    nullptr);

  m_vertexRender.SetPrimitiveCapacity(0);
}

META_REGISTER_FUNCTION_MEMBER(HeightMapChunkBarn, OnLevelUnload)
META_DATA_MEMBER_FUNCTION(HeightMapChunkBarn, OnLevelLoad, ArgName, "(levelName)")
void HeightMapChunkBarn::OnLevelUnload(
  cstring levelName
) {
  if (strcmp(levelName, "Infdev_TestLevel"))
    return;

  // Deinitialize renderer.
  m_vertexRender.Dequeue();
  m_vertexRender.Release();

  m_vertexData.Release();
}

META_REGISTER_FUNCTION_MEMBER(HeightMapChunkBarn, Update)
void HeightMapChunkBarn::Update(
  Game *game,
  AvatarBarn *avatarBarn
) {
  ChunkPos avatarChunkPos = {0, 0};

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

META_REGISTER_FUNCTION_MEMBER(HeightMapChunkBarn, BuildScene)
void HeightMapChunkBarn::BuildScene() {
  std::shared_lock<std::shared_mutex> lock(m_lock);

  // TODO: Implement scene building logic
  // This will iterate through m_loadedChunks and build rendering data
}

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
