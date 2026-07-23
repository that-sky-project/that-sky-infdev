#include <math.h>
#include <vector>
#include "sky/skyPrivate.hpp"
#include "render/vertexArrayElements.hpp"
#include "render/terrainRender.hpp"

// ----------------------------------------------------------------------------
// [SECTION] TerrainRenderChunk
// ----------------------------------------------------------------------------

void TerrainRenderChunk::Initialize(
  Game *game,
  Heap *heap,
  Scene *scene,
  ResourceManager *resources,
  MaterialDefBarn *materialDefBarn
) {
  m_game = game;
  m_heap = heap;
  m_scene = scene;
  m_resources = resources;
  m_materialDefBarn = materialDefBarn;
  m_initialized = true;
}

void TerrainRenderChunk::BuildScene() {
  Assert(m_submitted && m_initialized);
  m_depthR.Queue();
  m_matR.Queue();
}

void TerrainRenderChunk::Submit() {
  constexpr i32 vtxCount = kPointCount * kPointCount;
  constexpr i32 idxCount = kChunkSize * kChunkSize * 2 * 3;

  std::vector<GrassShVertex> matVertices;
  std::vector<TerrainDepthVertex> depthVertices;
  std::vector<u16> idxBuffer;

  Assert(!m_submitted && m_initialized);

  matVertices.reserve(vtxCount);
  depthVertices.reserve(vtxCount);
  idxBuffer.reserve(idxCount);

  // Build vertex buffer.
  BlockPos base = BlockPos(m_pos.x << 4, 0, m_pos.z << 4);
  for (i32 xC = 0; xC < kPointCount; xC++) {
    for (i32 zC = 0; zC < kPointCount; zC++) {
      // Set the height of the vertex.
      matVertices[zC + xC * kPointCount] = GrassShVertex(
        xC + base.x,
        m_heightMap.at(xC + base.x, zC + base.z) + 1,
        zC + base.z);
      depthVertices[zC + xC * kPointCount] = TerrainDepthVertex(
        xC + base.x,
        m_heightMap.at(xC + base.x, zC + base.z) + 1,
        zC + base.z);
    }
  }

  // Build triangle faces.
  for (i32 xC = 0; xC < kChunkSize; xC++) {
    for (i32 zC = 0; zC < kChunkSize; zC++) {
      idxBuffer.push_back(zC + xC * kPointCount);
      idxBuffer.push_back(zC + 1 + xC * kPointCount);
      idxBuffer.push_back(zC + (xC + 1) * kPointCount);

      idxBuffer.push_back(zC + 1 + (xC + 1) * kPointCount);
      idxBuffer.push_back(zC + (xC + 1) * kPointCount);
      idxBuffer.push_back(zC + 1 + xC * kPointCount);
    }
  }

  // Submit TerrainDepth data.
  m_depthD.BeginDefinition("TerrainDepth", vtxCount);
  m_depthD.AddVertexBuffer(
    0,
    TerrainDepthVertex::kTypes,
    TerrainDepthVertex::kAttrs,
    TerrainDepthVertex::kNumAttrs,
    (GpuBuffer::Strategy)1,
    0,
    depthVertices.data());
  m_depthD.AddIndexBuffer(
    0, kGfxType_SHORT, (GpuBuffer::Strategy)1, idxCount, idxBuffer.data());
  m_depthD.EndDefinition();

  // Submit TerrainMats (material) render data.
  m_matD.BeginDefinition("TerrainGeo", vtxCount);
  m_matD.AddVertexBuffer(
    0,
    GrassShVertex::kTypes,
    GrassShVertex::kAttrs,
    GrassShVertex::kNumAttrs,
    (GpuBuffer::Strategy)1,
    0,
    matVertices.data());
  m_matD.AddIndexBuffer(
    0, kGfxType_SHORT, (GpuBuffer::Strategy)1, idxCount, idxBuffer.data());
  m_matD.EndDefinition();

  // Setup TerrainDepth render.
  m_depthR.Initialize(
    &m_depthD,
    m_resources,
    "TerrainDepth",
    m_scene->GetRenderListByName("TerrainDepth"),
    0,
    nullptr);
  MaterialDefBarn::SetMaterialShaderUniforms(
    m_depthR.GetPipelineInstance(),
    m_materialDefBarn->GetDef(kMaterial_None),
    m_resources);
  m_depthR.AllocVertexSparse(0, nullptr, 0x400);

  // Setup TerrainMats render.
  m_matR.Initialize(
    &m_matD,
    m_resources,
    "GrassSh",
    m_scene->GetRenderListByName("TerrainMats"),
    0,
    nullptr);
  MaterialDefBarn::SetMaterialShaderUniforms(
    m_matR.GetPipelineInstance(),
    m_materialDefBarn->GetDef(kMaterial_Grass),
    m_resources);
  m_matR.AllocVertexSparse(0, nullptr, 0x400);

  m_submitted = true;
}

void TerrainRenderChunk::SetPos(
  const ChunkPos &pos
) {
  Assert(!m_submitted);
  m_pos = pos;
}

void TerrainRenderChunk::SetHeightMap(
  const TerrainChunkHeightMap &data
) {
  Assert(!m_submitted);
  m_heightMap = data;
}

TerrainChunkHeightMap &TerrainRenderChunk::GetHeightMap() {
  return m_heightMap;
}

// ----------------------------------------------------------------------------
// [SECTION] TerrainChunkBarn
// ----------------------------------------------------------------------------

void TerrainChunkBarn::Initialize(
  Game *game,
  Heap *heap,
  Scene *scene,
  ResourceManager *resources,
  MaterialDefBarn *materialDefBarn
) {
  m_game = game;
  m_heap = heap;
  m_scene = scene;
  m_resources = resources;
  m_materialDefBarn = materialDefBarn;
}

void TerrainChunkBarn::Update(
  AvatarBarn *avatarBarn
) {
  // Avatar *avatar = MetaCall<Avatar *(AvatarBarn::*)(bool)>(avatarBarn)["TryGetLocalAvatar"](true);
  // Matrix4 transform = MetaCall<Matrix4 (Avatar::*)()>(avatar)["GetTransform"]();
  Matrix4 transform = Matrix4(1);
  Vector4 pos = transform[3];
  ChunkPos inside = ChunkPos((i32)pos->x >> 4, (i32)pos->z >> 4);

  if (inside == m_lastInside)
    // Skip the update if the position didn't changed.
    return;

  std::vector<ChunkPos> chunksToRemove;
  for (const auto &itChunk: m_chunks) {
    i32 dist = abs(inside.x - itChunk.first.x) + abs(inside.z - itChunk.first.z);
    if (dist > m_viewDist)
      chunksToRemove.push_back(itChunk.first);
  }

  std::vector<ChunkPos> chunksToAdd;
}
