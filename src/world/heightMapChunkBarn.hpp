#ifndef __WORLD_HEIGHTMAPCHUNKBARN_HPP__
#define __WORLD_HEIGHTMAPCHUNKBARN_HPP__

#include <unordered_set>
#include <shared_mutex>
#include <cstring>
#include <vector>
#include <Utils/Types.h>
#include <Base/Meta.hpp>
#include "mc/posTypes.hpp"
#include "mod/moduleBarnExt.hpp"
#include "sky/skyGame.hpp"
#include "sky/skyVertex.hpp"
#include "sky/skyAvatarBarn.hpp"
#include "render/vertexArrayElements.hpp"
#include "sky/skyCollisionGeo.hpp"
#include "sky/skyTypePlaceholders.hpp"
#include "world/synth/perlinNoise.hpp"

// ----------------------------------------------------------------------------
// [SECTION] HeightMapChunk
// ----------------------------------------------------------------------------

class HeightMapChunk {
public:
  HeightMapChunk() = default;
  ~HeightMapChunk() = default;

  inline void Initialize(const ChunkPos &pos) { m_pos = pos; }
  inline void Terminate() { }

  inline const ChunkPos &GetPos() const { return m_pos; }
  inline const f32 *GetHeights() const { return (const f32 *)m_heights; }
  inline void SetHeights(const f32 *heights) { std::memcpy(m_heights, heights, sizeof(m_heights)); }

private:
  ChunkPos m_pos = {};
  f32 m_heights[17][17] = {0};
};

// ----------------------------------------------------------------------------
// [SECTION] HeightMapChunkSource
// ----------------------------------------------------------------------------

class HeightMapChunkSource {
public:
  HeightMapChunkSource(): m_noise(new PerlinNoise()) { }
  ~HeightMapChunkSource() { delete m_noise; }

  void Initialize(i32 seed);
  void Terminate();

  void LoadChunk(HeightMapChunk *chunk);

private:
  i32 m_seed = 0;
  PerlinNoise *m_noise = nullptr;
};

// ----------------------------------------------------------------------------
// [SECTION] HeightMapChunkBarn
// ----------------------------------------------------------------------------

class HeightMapChunkBarn;
META_DECLARE_CLASS(HeightMapChunkBarn)

class HeightMapChunkBarn: public Module {
private:
  static constexpr u32 kMaxChunks = 49 * 49;
  // Each chunk: 17x17 vertices, 16x16 quads = 32x16 triangles.
  static constexpr u32 kChunkVtxCount = 17 * 17;
  static constexpr u32 kChunkIdxCount = 16 * 16 * 6;
  static constexpr cstring kTestInfdevLevel = "CandleSpace";

  struct RenderData {
    void Initialize(
      cstring tag,
      const GfxType *types,
      const GfxAttr *attrs,
      u32 attrCount,
      ResourceManager *resources,
      cstring shader,
      RenderList *renderList,
      Heap *heap);
    void Terminate();

    inline void SetPrimitiveCount(u32 count) { render.SetPrimitiveCapacity(count); }
    inline void ClearRenderChunk() { render.ClearRenderChunk(); }
    inline void AddRenderChunk(u32 idxOffset, u32 idxCount, i32 vtxOffset) { render.AddRenderChunk(idxOffset, idxCount, vtxOffset); }

    inline void *MapVtxBuffer() { return data.GetVertexBuffer(0).MapBuffer(); }
    inline void UnmapVtxBuffer() { data.GetVertexBuffer(0).UnmapBuffer(); }

    inline void Queue() { render.Queue(); }
    inline void Dequeue() { render.Dequeue(); }

    VertexData data = {};
    VertexRenderSparse render = {};
  };

  /*struct RenderChunk {
    bool used = false;
    ChunkPos pos = {};
    u32 vtxOffset = 0;
    TerrainDepthVertex *v1 = nullptr;
    TerrainMaterialVertex *v2 = nullptr;
  };

  struct CollisionChunk {
    bool used = false;
    ChunkPos pos = {};
    u32 geoIndex = 0;
    CollisionGeoInstance *geoInstance = nullptr;
  };*/

private:
  static void ms_ChunkUpdateThread(HeightMapChunkBarn *heightMapChunkBarn);

public:
  HeightMapChunkBarn(): m_chunkSource(new HeightMapChunkSource()) { }
  ~HeightMapChunkBarn() { delete m_chunkSource; }

  void Initialize(CollisionGeoBarn *collisionGeoBarn);
  void Terminate();
  void OnLevelLoad(
    Game *game,
    ResourceManager *resources,
    Scene *scene,
    MaterialDefBarn *materialDefBarn,
    cstring levelName);
  void OnLevelUnload(
    cstring levelName);
  void Update(
    Game *game,
    AvatarBarn *avatarBarn,
    cstring levelName);
  void BuildScene(
    cstring levelName);

  //HeightMapChunkSourceParams *CreateParams(MetaClass *mc);

private:
  bool m_IsChunkQueued(const ChunkPos &pos);
  void m_QueueChunk(const ChunkPos &pos);
  void m_DequeueChunk(const ChunkPos &pos);

  bool m_IsChunkLoaded(const ChunkPos &pos);
  void m_LoadChunk(const HeightMapChunk *chunk);
  void m_UnloadChunk(const ChunkPos &pos);

private:
  std::shared_mutex m_lock = {};
  std::thread m_chunkUpdateThread = {};
  bool m_running = false;
  i32 m_seed = 1196250184;
  u32 m_viewDistance = 1;
  HeightMapChunkSource *m_chunkSource = nullptr;
  ChunkPos m_lastPos = {-2147483647, -2147483647};

  std::unordered_set<ChunkPos> m_queuedChunks = {};
  std::unordered_map<ChunkPos, const HeightMapChunk *> m_loadedChunks = {};
  //std::unordered_map<ChunkPos, CollisionChunk> m_collisionChunks = {};

  //RenderChunk *m_renderChunks = nullptr;

  Heap *m_heightMapVertexHeap = nullptr;
  CollisionGeoBarn *m_collisionGeoBarn = nullptr;
  RenderData m_depth = {};
  RenderData m_mats = {};
};

#endif
