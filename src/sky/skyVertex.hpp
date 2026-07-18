#ifndef __SKY_SKYVERTEX_HPP__
#define __SKY_SKYVERTEX_HPP__

#include <intrin.h>
#include <Utils/Types.h>
#include "sky/skyGfx.hpp"
#include "sky/skyTypePlaceholders.hpp"

class PipelineInstance {
private:
  u64 _align;
  u08 _gap[272 - 8];
};

class GpuBuffer {
public:
  enum Strategy: u08 { };
};

class VertexData {
public:
  static constexpr u32 kMaxVertexBuffers = 8;
  static constexpr u32 kMaxIndexBuffers = 4;

  explicit VertexData();
  ~VertexData();

  void BeginDefinition(
    cstring name,
    u32 maxIndices);

  void EndDefinition();

  void AddVertexBuffer(
    u32 idx,
    const GfxType *types,
    const GfxAttr *attrs,
    u32 attrCount,
    GpuBuffer::Strategy strategy,
    u32 flags,
    const void *vtxData);

  void AddIndexBuffer(
    u32 idx,
    GfxType type,
    GpuBuffer::Strategy strategy,
    u32 flags,
    const void *idxBuffer);

private:
  __m128 _align;
  u08 _gap[824 - 16];
};

class VertexRender {
public:
  enum: u08 {
    kPrimitiveType_PointList = 0,
    kPrimitiveType_LineList,
    kPrimitiveType_LineStrip = 3,
    kPrimitiveType_TriangleList,
    kPrimitiveType_TriangleStrip,
    kPrimitiveType_TriangleFan,
  };

  explicit VertexRender();
  ~VertexRender() = default;

  inline bool IsInitialized() { return m_isInitialized; }
  inline bool IsQueued() { return m_queued; }
  inline void SetPrimitiveType(u08 type) { m_primitiveType = type; }

  // Access to the PipelineInstance object in the VertexRender. Must be called
  // after VertexRender::Initialize().
  inline PipelineInstance *GetPipelineInstance() {
    Assert(IsInitialized());
    return &m_pipelineInstance;
  }

  void Initialize(
    VertexData *renderData,
    const ShaderProgram *shader,
    RenderList *renderList,
    u32 a5,
    void *a6);

  void Initialize(
    VertexData *renderData,
    ResourceManager *resources,
    cstring resourceName,
    RenderList *renderList,
    u32 a5,
    void *a6);

  void AllocVertexSparse(
    bool useChunks,
    Heap *heap,
    u32 maxChunkCount);

  void Queue();

protected:
  VertexData *m_vertexData = nullptr;
  RenderList *m_renderList = nullptr;
  ShaderProgram *m_shaderProgram = nullptr;
  cstring m_debugName = nullptr;
  __m128 m_matrix[4] = {0};
  u08 unk_2[84] = {0};
  i32 m_renderPipeline = 0;
  PipelineInstance m_pipelineInstance = {};
  u32 m_indexIdx = 0;
  i32 m_maxPrimitive = 0;
  i32 m_maxInstance = 0;
  i32 m_vertexOffset = 0;
  i32 m_indexOffset = 0;
  i32 unk_4 = 0;
  u08 m_primitiveType = kPrimitiveType_TriangleList;
  bool m_useChunk = false;
  bool unk_5 = false;
  bool m_queued = false;
  bool unk_6 = false;
  bool m_isInitialized = false;
  u08 unk_7[10] = {0};
  Heap *m_heap = nullptr;
  void *m_chunkData = nullptr;
  u16 m_chunkCount = 0;
  u16 m_maxChunkCount = 0;
  u08 unk_9[12] = {0};
};

#endif
