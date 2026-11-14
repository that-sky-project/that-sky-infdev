#ifndef __WORLD_VOXELCHUNKBARN_HPP__
#define __WORLD_VOXELCHUNKBARN_HPP__

#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include <limits>
#include <Voxels/Voxels.h>
#include <Utils/Types.h>
#include <Base/Meta.hpp>
#include "world/mc/posTypes.hpp"
#include "mod/moduleBarnExt.hpp"
#include "sky/skyGame.hpp"
#include "sky/skyVertex.hpp"
#include "sky/skyAvatarBarn.hpp"
#include "sky/skyVectorMath.hpp"
#include "sky/skyCollisionGeo.hpp"
#include "sky/skyTypePlaceholders.hpp"
#include "world/synth/valueNoise.hpp"
#include "render/vertexArrayElements.hpp"

// ----------------------------------------------------------------------------
// [SECTION] VoxelChunkPos
// ----------------------------------------------------------------------------

// A voxel chunk is a cube in world space; the Y component is the vertical
// chunk index, so the terrain can span several chunks vertically.
struct VoxelChunkPos {
  VoxelChunkPos() = default;
  ~VoxelChunkPos() = default;

  VoxelChunkPos(
    i32 x,
    i32 y,
    i32 z
  )
    : x(x)
    , y(y)
    , z(z)
  { }

  bool operator==(const VoxelChunkPos &that) const { return x == that.x && y == that.y && z == that.z; }
  bool operator!=(const VoxelChunkPos &that) const { return !(that == *this); }

  i32 x = 0;
  i32 y = 0;
  i32 z = 0;
};

namespace std {

template<>
struct hash<VoxelChunkPos> {
  size_t operator()(const VoxelChunkPos &k) const noexcept {
    size_t h = std::hash<i32>{}(k.x);
    h ^= std::hash<i32>{}(k.y) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<i32>{}(k.z) + 0x9e3779b9 + (h << 6) + (h >> 2);
    return h;
  }
};

}

// ----------------------------------------------------------------------------
// [SECTION] VoxelChunkMesh
// ----------------------------------------------------------------------------

// CPU-side polygonized data of a single voxel chunk. Both the render and the
// collision paths consume this structure, so it is kept alive after
// generation until the chunk is unloaded.
struct VoxelChunkMesh {
  // Hard per-chunk limits. They bound the GPU buffers allocated up-front.
  // A padded 64^3 chunk normally produces fewer than ~10k vertices.
  static constexpr u32 kMaxVertices = 12288;
  static constexpr u32 kMaxIndices = 65535;

  inline bool IsEmpty() const { return indices.empty(); }
  inline void Clear() {
    depthVertices.clear();
    materialVertices.clear();
    indices.clear();
    min = Vector4(std::numeric_limits<f32>::max());
    max = Vector4(-std::numeric_limits<f32>::max());
    truncated = false;
  }

  std::vector<TerrainDepthVertex> depthVertices = {};
  std::vector<TerrainMaterialVertex> materialVertices = {};
  std::vector<u16> indices = {};

  Vector4 min = Vector4(std::numeric_limits<f32>::max());
  Vector4 max = Vector4(-std::numeric_limits<f32>::max());

  // Set when the mesh exceeds the per-chunk budget and had to be clipped.
  bool truncated = false;
};

// ----------------------------------------------------------------------------
// [SECTION] VoxelChunkSourceParams
// ----------------------------------------------------------------------------

class VoxelChunkSourceParams;
META_DECLARE_CLASS(VoxelChunkSourceParams)

class VoxelChunkSourceParams: public Object {
public:
  VoxelChunkSourceParams() { m_metaClassId = MetaClassId(VoxelChunkSourceParams); }
  ~VoxelChunkSourceParams() = default;

  i32 seed = 0;
  u32 viewDistance = 0;
};

// ----------------------------------------------------------------------------
// [SECTION] VoxelChunkSource
// ----------------------------------------------------------------------------

class VoxelChunkMaterialMap: public Voxels::MaterialMap {
public:
  VoxelChunkMaterialMap() = default;
  ~VoxelChunkMaterialMap() = default;

  Voxels::MaterialMap::Material *GetMaterial(
    unsigned char id) const override {
    return const_cast<Voxels::MaterialMap::Material *>(&m_material);
  }

private:
  Voxels::MaterialMap::Material m_material = {};
};

class VoxelChunkSource {
public:
  // Core chunk size and the one-block padding that is sampled around it.
  // Padding makes boundary cells use the neighbouring chunk's real samples
  // instead of the clamped edge of the grid, which removes visible cracks.
  static constexpr u32 kChunkSize = 64;
  static constexpr u32 kChunkPadding = 16;
  static constexpr u32 kGridSize = kChunkSize + 2 * kChunkPadding;

  VoxelChunkSource() = default;
  ~VoxelChunkSource() = default;

  void Initialize(u32 seed);
  void Terminate();

  // Polygonizes the chunk at `pos` into `mesh`.
  // Returns false if the voxels library could not produce a surface.
  bool Generate(
    const VoxelChunkPos &pos,
    VoxelChunkMesh *mesh);

private:
  ValueNoiseSurface *m_surface = nullptr;
  VoxelChunkMaterialMap m_materialMap = {};
};

// ----------------------------------------------------------------------------
// [SECTION] VoxelChunkBarn
// ----------------------------------------------------------------------------

class VoxelChunkBarn;
META_DECLARE_CLASS(VoxelChunkBarn)

class VoxelChunkBarn: public Module {
private:
  static constexpr u32 kChunkSize = VoxelChunkSource::kChunkSize;
  static constexpr i32 kFirstChunkY = 0;
  static constexpr i32 kLastChunkY = 1;
  static constexpr i32 kVerticalChunkCount = kLastChunkY - kFirstChunkY + 1;
  static constexpr u32 kMaxViewDistance = 4;

  struct RenderData {
    void Initialize(
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
      u32 indexCapacity);
    void Terminate();

    inline void *MapVtxBuffer() { return data.GetVertexBuffer(0).MapBuffer(); }
    inline void UnmapVtxBuffer() { data.GetVertexBuffer(0).UnmapBuffer(); }

    inline void *MapIdxBuffer() { return data.GetIndexBuffer(0).buffer.MapBuffer(); }
    inline void UnmapIdxBuffer() { data.GetIndexBuffer(0).buffer.UnmapBuffer(); }

    inline void ClearRenderChunk() { render.ClearRenderChunk(); }
    inline void AddRenderChunk(u32 idxOffset, u32 idxCount, i32 vtxOffset) { render.AddRenderChunk(idxOffset, idxCount, vtxOffset); }

    inline void Queue() { render.Queue(); }
    inline void Dequeue() { render.Dequeue(); }

    VertexData data = {};
    VertexRenderSparse render = {};
  };

  struct ClientChunk {
    void Initialize(
      CollisionGeoBarn *collisionGeoBarn,
      const VoxelChunkPos &pos);
    void Terminate();

    void BuildCollision();
    void RemoveCollision();

    inline bool HasMesh() const { return m_hasMesh; }
    inline const VoxelChunkPos &GetPos() const { return m_pos; }

    // Contexts.
    CollisionGeoBarn *m_collisionGeoBarn = nullptr;

    // Chunk data.
    VoxelChunkPos m_pos = {};
    VoxelChunkMesh m_mesh = {};

    // Flags.
    bool m_hasMesh = false;
    bool m_hasBuiltGeo = false;

    // CollisionGeo data.
    u32 m_geoIdx = 0;
    CollisionGeoInstance *m_geoInst = nullptr;
  };

private:
  static void ms_ChunkUpdateThread(VoxelChunkBarn *voxelChunkBarn);
  static u32 ms_CountChunks(u32 radius);

public:
  VoxelChunkBarn() { m_metaClassId = MetaClassId(VoxelChunkBarn); }
  ~VoxelChunkBarn() = default;

  void Initialize(CollisionGeoBarn *collisionGeoBarn);
  void Terminate();
  void OnLevelLoad(
    Game *game,
    ResourceManager *resources,
    Scene *scene,
    MaterialDefBarn *materialDefBarn,
    cstring levelName);
  void OnLevelUnload(cstring levelName);
  void Update(
    Game *game,
    AvatarBarn *avatarBarn,
    cstring levelName);
  void BuildScene(cstring levelName);

  VoxelChunkSourceParams *CreateParams(MetaClass *mc);

private:
  void m_AddClientChunk(const VoxelChunkPos &pos);
  void m_RemoveClientChunk(const VoxelChunkPos &pos);
  void m_ClearClientChunk();
  bool m_IsClientChunkAdded(const VoxelChunkPos &pos);

private:
  // Multi-thread contexts.
  std::shared_mutex m_lock = {};
  std::mutex m_sourceLock = {};
  std::thread m_chunkUpdateThread = {};
  std::atomic<bool> m_running = false;

  // Configurations.
  bool m_hasParams = false;
  bool m_levelActive = false;
  bool m_sourceReady = false;
  bool m_voxelsReady = false;
  VoxelChunkSourceParams m_params = {};
  u32 m_seed = 1337;
  u32 m_viewDistance = 2;
  u32 m_chunkCapacity = 0;

  // Game contexts.
  Heap *m_heap = nullptr;
  VoxelChunkSource *m_chunkSource = nullptr;
  CollisionGeoBarn *m_collisionGeoBarn = nullptr;

  // Chunk data.
  ChunkPos m_lastPos = {-2147483647, -2147483647};
  // Horizontal center used by the worker to generate nearby chunks first.
  // Guarded by m_lock.
  ChunkPos m_generationCenter = {};
  std::unordered_map<VoxelChunkPos, ClientChunk> m_clientChunks = {};

  // Render data.
  RenderData m_depth = {};
  RenderData m_mats = {};
};

#endif
