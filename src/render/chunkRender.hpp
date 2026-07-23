#ifndef __RENDER_CHUNKRENDER_HPP__
#define __RENDER_CHUNKRENDER_HPP__

// ----------------------------------------------------------------------------
// - render/chunkRender.hpp
// Minecraft-style chunk renderer.
// ----------------------------------------------------------------------------

#include <functional>
#include <Utils/Types.h>
#include <Base/Meta.hpp>
#include "sky/skyTypePlaceholders.hpp"
#include "sky/skyGame.hpp"
#include "sky/skyVertex.hpp"
#include "sky/skyScene.hpp"
#include "sky/skyMaterialDefBarn.hpp"
#include "mc/posTypes.hpp"

using BlockRuntimeId = u32;

class BlockRuntimeIdBuilder {
public:
  inline static BlockRuntimeId Create(
    cstring name
  ) {
    static MetaStrHashMap<BlockRuntimeId> s_map = {};
    auto it = s_map.find(name);
    if (it != s_map.end())
      return it->second;
    return s_map[name] = std::hash<TgcString>()(name);
  }

  inline static BlockRuntimeId Create(
    TgcString name
  ) {
    return Create(name.c_str());
  }
};

class ChunkBlockData {
public:
  static constexpr u32 kChunkSizeX = 16;
  static constexpr u32 kChunkSizeY = 256;
  static constexpr u32 kChunkSizeZ = 16;

  ChunkBlockData(): m_blocks(kChunkSizeX * kChunkSizeY * kChunkSizeZ) { }
  ~ChunkBlockData() = default;

  BlockRuntimeId &at(
    const ChunkBlockPos &pos
  ) {
    return m_blocks[pos.y + (pos.z + pos.x * kChunkSizeZ) * kChunkSizeY];
  }

  const BlockRuntimeId &at(
    const ChunkBlockPos &pos
  ) const {
    return m_blocks[pos.y + (pos.z + pos.x * kChunkSizeZ) * kChunkSizeY];
  }

private:
  std::vector<BlockRuntimeId> m_blocks = {};
};

class ChunkRender {
public:
  ChunkRender() = default;
  ~ChunkRender() = default;

  // - Lifecycle functions.
  void Initialize(
    Game *game,
    Heap *heap,
    Scene *scene,
    ResourceManager *resources,
    MaterialDefBarn *materialDefBarn);
  void BuildScene();
  void Update();

  // - Operation functions.
  void SetBlockData(
    const ChunkBlockData &data);

private:
  // - Sky variables.
  Game *m_game = nullptr;
  Heap *m_heap = nullptr;
  Scene *m_scene = nullptr;
  ResourceManager *m_resources = nullptr;
  MaterialDefBarn *m_materialDefBarn = nullptr;

  // - Renderer variables.
  VertexRender m_depthR;
  VertexRender m_matR;
  VertexData m_depthD;
  VertexData m_matD;

  // - Chunk data.
  ChunkPos m_pos = {0, 0};
  BlockPos m_minPos = {0, 0, 0};
  BlockPos m_maxPos = {15, 255, 15};
  ChunkBlockData m_blocks = {};
  bool m_dirty = false;
};

#endif
