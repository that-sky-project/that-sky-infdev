#ifndef __WORLD_HEIGHTMAPCHUNKBARN_HPP__
#define __WORLD_HEIGHTMAPCHUNKBARN_HPP__

#include <unordered_set>
#include <shared_mutex>
#include <cstring>
#include <vector>
#include <limits>
#include <Utils/Types.h>
#include <Base/Meta.hpp>
#include "mc/posTypes.hpp"
#include "mod/moduleBarnExt.hpp"
#include "sky/skyGame.hpp"
#include "sky/skyVertex.hpp"
#include "sky/skyAvatarBarn.hpp"
#include "sky/skyVectorMath.hpp"
#include "sky/skyCollisionGeo.hpp"
#include "sky/skyTypePlaceholders.hpp"
#include "world/synth/perlinNoise.hpp"
#include "render/vertexArrayElements.hpp"

// ----------------------------------------------------------------------------
// [SECTION] HeightMapChunkSourceParams
// ----------------------------------------------------------------------------

class HeightMapChunkSourceParams;
META_DECLARE_CLASS(HeightMapChunkSourceParams)

class HeightMapChunkSourceParams: public Object {
public:
  HeightMapChunkSourceParams() { m_metaClassId = MetaClassId(HeightMapChunkSourceParams); }
  ~HeightMapChunkSourceParams() = default;

  i32 seed = 0;
  u32 viewDistance = 0;
};

// ----------------------------------------------------------------------------
// [SECTION] HeightMapChunk
// ----------------------------------------------------------------------------

class HeightMapChunk {
public:
  // Faces (blocks) the chunk covered.
  static constexpr u32 kTileSize = 16;
  // To calculate the edge triangles of the chunk, we need external size from
  // neighboring chunk.
  static constexpr u32 kRealSize = kTileSize + 1;
  // To calculate the normal, we need to extend by 1 more unit.
  static constexpr u32 kDataSize = 1 + kRealSize + 1;

  HeightMapChunk() = default;
  ~HeightMapChunk() = default;

  inline void Initialize(const ChunkPos &pos) { m_pos = pos; }
  inline void Terminate() { }

  inline const ChunkPos &GetPos() const { return m_pos; }
  inline const f32 *GetHeights() const { return (const f32 *)m_heights; }
  inline f32 Index(i32 xCL, i32 zCL) const { return m_heights[(u32)(xCL + 1)][(u32)(zCL + 1)]; }
  inline void SetHeights(const f32 *heights) { std::memcpy(m_heights, heights, sizeof(m_heights)); }

private:
  ChunkPos m_pos = {};
  f32 m_heights[kDataSize][kDataSize] = {0};
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
  static constexpr u32 kChunkVtxCount = HeightMapChunk::kRealSize * HeightMapChunk::kRealSize;
  static constexpr u32 kChunkIdxCount = HeightMapChunk::kTileSize * HeightMapChunk::kTileSize * 6;
  static constexpr u32 kCollisionChunkSize = HeightMapChunk::kRealSize + 1;
  static constexpr u32 kCollisionChunkVtxCount = kCollisionChunkSize * kCollisionChunkSize;
  static constexpr u32 kCollisionChunkIdxCount = HeightMapChunk::kRealSize * HeightMapChunk::kRealSize * 6;
  static constexpr cstring kTestInfdevLevel = "Infdev_INF";

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

  struct ClientChunk {
    void Initialize(Heap *heap, CollisionGeoBarn *collisionGeoBarn, const ChunkPos &pos);
    void Terminate();

    void BuildMesh();

    void BuildCollision();
    void RemoveCollision();

    inline bool HasMesh() const { return m_hasBuiltMesh; }
    inline const ChunkPos &GetPos() const { return m_data.GetPos(); }

    // Contexts.
    Heap *m_heap = nullptr;
    CollisionGeoBarn *m_collisionGeoBarn = nullptr;

    // Height map data.
    HeightMapChunk m_data = {};

    // Flags.
    bool m_hasLoaded = false;
    bool m_hasBuiltMesh = false;
    bool m_hasBuiltGeo = false;

    // Vertex buffers.
    TerrainDepthVertex *m_dvtx = nullptr;
    TerrainMaterialVertex *m_mvtx = nullptr;

    // CollisionGeo data.
    Vector4 m_min = Vector4(std::numeric_limits<float>::max());
    Vector4 m_max = Vector4(-std::numeric_limits<float>::max());
    u32 m_geoIdx = 0;
    CollisionGeoInstance *m_geoInst = nullptr;
  };

private:
  static void ms_ChunkUpdateThread(HeightMapChunkBarn *heightMapChunkBarn);

public:
  HeightMapChunkBarn(): m_chunkSource(new HeightMapChunkSource()) { m_metaClassId = MetaClassId(HeightMapChunkBarn); }
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

  HeightMapChunkSourceParams *CreateParams(MetaClass *mc);

private:
  void m_AddClientChunk(const ChunkPos &pos);
  void m_RemoveClientChunk(const ChunkPos &pos);
  void m_ClearClientChunk();
  bool m_IsClientChunkAdded(const ChunkPos &pos);

  void m_BuildClientChunkMesh(ClientChunk *chunk);

private:
  // Multi-thread contexts.
  std::shared_mutex m_lock = {};
  std::thread m_chunkUpdateThread = {};
  bool m_running = false;

  // Configurations.
  bool m_hasParams = false;
  HeightMapChunkSourceParams m_params = {};
  i32 m_seed = 1196250184;
  u32 m_viewDistance = 17;

  // Game contexts.
  Heap *m_heap = nullptr;
  HeightMapChunkSource *m_chunkSource = nullptr;
  CollisionGeoBarn *m_collisionGeoBarn = nullptr;

  // Chunk data.
  ChunkPos m_lastPos = {-2147483647, -2147483647};
  std::unordered_map<ChunkPos, ClientChunk> m_clientChunks = {};

  // Render data.
  RenderData m_depth = {};
  RenderData m_mats = {};
};

#endif
