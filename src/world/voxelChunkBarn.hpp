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
#include "world/synth/perlinNoise.hpp"
#include "render/vertexArrayElements.hpp"

// ----------------------------------------------------------------------------
// [SECTION] VoxelChunkMesh
// ----------------------------------------------------------------------------

// CPU-side polygonized data of a single voxel chunk. Both the render and the
// collision paths consume this structure, so it is kept alive after
// generation until the chunk is unloaded.
struct VoxelChunkMesh {
  // Hard per-chunk limits. They bound the GPU buffers allocated up-front.
  static constexpr u32 kMaxVertices = 8192;
  static constexpr u32 kMaxIndices = 16384;

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

class VoxelChunkSource: public Voxels::VoxelSurface {
public:
  // Voxels per chunk axis. The voxels library works with 16^3 blocks.
  static constexpr u32 kChunkSize = 16;

  VoxelChunkSource() = default;
  ~VoxelChunkSource() override = default;

  void Initialize(i32 seed);
  void Terminate();

  // Polygonizes the chunk at `pos` into `mesh`.
  // Returns false if the voxels library could not produce a surface.
  bool Generate(
    const ChunkPos &pos,
    VoxelChunkMesh *mesh);

  // Voxels::VoxelSurface implementation. The grid is created with local
  // coordinates [0, kChunkSize) and this callback maps them to world space.
  void GetSurface(
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
    unsigned char *blend) override;

private:
  f32 m_SampleDensity(
    f32 wx,
    f32 wy,
    f32 wz) const;

  PerlinNoise *m_noise = nullptr;
  VoxelChunkMaterialMap m_materialMap = {};

  // Base world coordinates of the chunk currently being generated. Grid
  // creation is single-threaded here, so no synchronization is required.
  i32 m_baseX = 0;
  i32 m_baseZ = 0;
};

// ----------------------------------------------------------------------------
// [SECTION] VoxelChunkBarn
// ----------------------------------------------------------------------------

class VoxelChunkBarn;
META_DECLARE_CLASS(VoxelChunkBarn)

class VoxelChunkBarn: public Module {
private:
  static constexpr u32 kChunkSize = VoxelChunkSource::kChunkSize;
  static constexpr u32 kMaxViewDistance = 6;

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
      const ChunkPos &pos);
    void Terminate();

    void BuildCollision();
    void RemoveCollision();

    inline bool HasMesh() const { return m_hasMesh; }
    inline const ChunkPos &GetPos() const { return m_pos; }

    // Contexts.
    CollisionGeoBarn *m_collisionGeoBarn = nullptr;

    // Chunk data.
    ChunkPos m_pos = {};
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
  void m_AddClientChunk(const ChunkPos &pos);
  void m_RemoveClientChunk(const ChunkPos &pos);
  void m_ClearClientChunk();
  bool m_IsClientChunkAdded(const ChunkPos &pos);

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
  i32 m_seed = 1196250184;
  u32 m_viewDistance = 4;
  u32 m_chunkCapacity = 0;

  // Game contexts.
  Heap *m_heap = nullptr;
  VoxelChunkSource *m_chunkSource = nullptr;
  CollisionGeoBarn *m_collisionGeoBarn = nullptr;

  // Chunk data.
  ChunkPos m_lastPos = {-2147483647, -2147483647};
  std::unordered_map<ChunkPos, ClientChunk> m_clientChunks = {};

  // Render data.
  RenderData m_depth = {};
  RenderData m_mats = {};
};

#endif
