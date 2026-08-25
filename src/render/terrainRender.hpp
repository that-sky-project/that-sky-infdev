#ifndef __RENDER_CHUNKRENDER_HPP__
#define __RENDER_CHUNKRENDER_HPP__

// ----------------------------------------------------------------------------
// - render/terrainReder.hpp
// Sky-style chunk renderer.
// ----------------------------------------------------------------------------

#include <functional>
#include <Utils/Types.h>
#include <Base/Meta.hpp>
#include "sky/skyTypePlaceholders.hpp"
#include "sky/skyGame.hpp"
#include "sky/skyVertex.hpp"
#include "sky/skyScene.hpp"
#include "sky/skyMaterialDefBarn.hpp"
#include "sky/skyVectorMath.hpp"
#include "mc/posTypes.hpp"

class TerrainChunkHeightMap {
public:
  static constexpr u32 kChunkSize = 16;
  // Contains points of the neighbor chunks to generate full mesh.
  static constexpr u32 kPointCount = kChunkSize + 1;

  TerrainChunkHeightMap(): m_height(kPointCount * kPointCount) { }
  ~TerrainChunkHeightMap() = default;

  inline TerrainChunkHeightMap &operator=(
    const TerrainChunkHeightMap &that
  ) {
    m_height = that.m_height;
    return *this;
  }

  f32 &at(
    u32 x,
    u32 z
  ) {
    x %= kPointCount;
    z %= kPointCount;
    return m_height[z + x * kPointCount];
  }

  const f32 &at(
    u32 x,
    u32 z
  ) const {
    x %= kPointCount;
    z %= kPointCount;
    return m_height[z + x * kPointCount];
  }

private:
  std::vector<f32> m_height = {};
};

class TerrainRenderChunk {
public:
  static constexpr u32 kChunkSize = TerrainChunkHeightMap::kChunkSize;
  static constexpr u32 kPointCount = TerrainChunkHeightMap::kPointCount;

  TerrainRenderChunk() = default;
  ~TerrainRenderChunk() = default;
  TerrainRenderChunk(TerrainRenderChunk &&) = default;

  TerrainRenderChunk(const TerrainRenderChunk &) = delete;
  TerrainRenderChunk &operator=(const TerrainRenderChunk &) = delete;

  // - Lifecycle functions.
  void Initialize(
    Game *game,
    Heap *heap,
    Scene *scene,
    ResourceManager *resources,
    MaterialDefBarn *materialDefBarn);
  void BuildScene();

  // - Operation functions.
  void SetPos(
    const ChunkPos &pos);
  TerrainChunkHeightMap &GetHeightMap();
  void SetHeightMap(
    const TerrainChunkHeightMap &data);
  void Submit();

private:
  // - Sky variables.
  Game *m_game = nullptr;
  Heap *m_heap = nullptr;
  Scene *m_scene = nullptr;
  ResourceManager *m_resources = nullptr;
  MaterialDefBarn *m_materialDefBarn = nullptr;

  // - Renderer variables.
  VertexRenderSparse m_depthR;
  VertexRenderSparse m_matR;
  VertexData m_depthD;
  VertexData m_matD;

  // - Chunk data.
  ChunkPos m_pos = {0, 0};
  TerrainChunkHeightMap m_heightMap = {};
  bool m_initialized = false;
  bool m_submitted = false;
};

class TerrainChunkBarn {
private:
  struct Render {
    VertexData *data = nullptr;
    VertexRender *render = nullptr;
    Vector4 min = Vector4(0);
    Vector4 max = Vector4(0);
    u32 vertices = 0;
    u32 indices = 0;
  };

public:
  // - Job functions.
  static void LoadChunkJob();

  // - Constructor and destructor functions.
  TerrainChunkBarn() = default;
  ~TerrainChunkBarn() = default;

  TerrainChunkBarn(TerrainChunkBarn &&) = default;

  TerrainChunkBarn(const TerrainChunkBarn &) = delete;
  TerrainChunkBarn &operator=(const TerrainChunkBarn &) = delete;

  // - Lifecycle functions.
  void Initialize(
    Game *game,
    Heap *heap,
    Scene *scene,
    ResourceManager *resources,
    MaterialDefBarn *materialDefBarn);
  void Update(
    AvatarBarn *avatarBarn);
  void BuildScene();

private:
  void RebuildMesh();

  // - Sky variables.
  Game *m_game = nullptr;
  Heap *m_heap = nullptr;
  Scene *m_scene = nullptr;
  ResourceManager *m_resources = nullptr;
  MaterialDefBarn *m_materialDefBarn = nullptr;

  // - Data.
  bool m_dirty = false;
  i32 m_viewDist = 3;
  ChunkPos m_lastInside = ChunkPos(0, 0);
  std::unordered_map<ChunkPos, TerrainRenderChunk *> m_chunks = {};
  std::list<Render> m_render = {};
};

#endif
