#ifndef __SKY_SKYVERTEX_HPP__
#define __SKY_SKYVERTEX_HPP__

#include <intrin.h>
#include <Utils/Types.h>
#include "sky/skyPrivate.hpp"
#include "sky/skyGfx.hpp"
#include "sky/skyAutoLister.hpp"
#include "sky/skyVectorMath.hpp"
#include "sky/skyRendererUtils.hpp"
#include "sky/skyTypePlaceholders.hpp"

class VertexData {
public:
  static constexpr u32 kMaxVertexBuffers = 8;
  static constexpr u32 kMaxIndexBuffers = 4;
  static constexpr u32 kMaxShaderAttributes = 16;

  // A single index buffer context slot.
  struct IndexBuffer {
    ~IndexBuffer() = default;
    IndexBuffer() = default;
    IndexBuffer(IndexBuffer &&) = delete;
    IndexBuffer(const IndexBuffer &) = delete;
    IndexBuffer &operator=(const IndexBuffer &) = delete;

    inline void Reset() {
      buffer.Terminate();
      type = kGfxType_UNKNOWN;
      unk_3 = 0;
      capacity = 0;
    }

    GpuBuffer buffer;
    u32 capacity = 0;
    GfxType type = kGfxType_UNKNOWN;
    u08 unk_3 = 0;
  };

  struct UnknownBuffer {
    ~UnknownBuffer() = default;
    UnknownBuffer() = default;
    UnknownBuffer(UnknownBuffer &&) = delete;
    UnknownBuffer(const UnknownBuffer &) = delete;
    UnknownBuffer &operator=(const UnknownBuffer &) = delete;

    inline void Reset() {
      buffer.Terminate();
      unk_1 = 0;
      unk_2 = 0;
    }

    u64 unk_1 = 0;
    GpuBuffer buffer;
    u64 unk_2 = 0;
  };

  // A single vertex attribute slot in the shared format table. One is appended
  // per attribute across every AddVertexBuffer() call.
  struct Attribute {
    ~Attribute() = default;
    Attribute() = default;
    Attribute(Attribute &&) = delete;
    Attribute(const Attribute &) = delete;
    Attribute &operator=(const Attribute &) = delete;

    inline void Reset() {
      attr = kGfxAttr_Undefined;
      type = kGfxType_UNKNOWN;
      bufferIdx = 0;
      offset = 0;
    }

    // Attribute semantic (position/normal/uv/...).
    GfxAttr attr = kGfxAttr_Undefined;
    // Element data format.
    GfxType type = kGfxType_UNKNOWN;
    // Index of the vertex buffer this lives in
    u16 bufferIdx = 0;
    // Byte offset within that buffer's vertex.
    u32 offset = 0;
  };

  // Per-vertex-buffer info.
  struct VertexBufferInfo {
    ~VertexBufferInfo() = default;
    VertexBufferInfo() = default;
    VertexBufferInfo(VertexBufferInfo &&) = delete;
    VertexBufferInfo(const VertexBufferInfo &) = delete;
    VertexBufferInfo &operator=(const VertexBufferInfo &) = delete;

    inline void Reset() {
      stride = 0;
      perInstance = 0;
    }

    // Bytes per element (sum of attribute sizes)
    u16 stride = 0;
    // TODO: Maybe instance count.
    u16 perInstance = 0;
  };

  VertexData() = default;
  inline ~VertexData() { Release(); }

  inline VertexData::IndexBuffer &GetIndexBuffer(u32 idx) { Assert(idx < kMaxIndexBuffers); return m_idxBuffers[idx]; }
  inline GpuBuffer &GetVertexBuffer(u32 idx) { Assert(idx < kMaxVertexBuffers); return m_vtxBuffers[idx]; }
  inline u32 GetIndexCapacity(u32 idx) const { Assert(idx < kMaxIndexBuffers); return m_idxBuffers[idx].capacity; }
  inline void SetInstanceCapacity(u32 capacity) { m_instanceCapacity = capacity; }

  u32 GetTotalSize();

  void Release();

  void BeginDefinition(
    cstring name,
    u32 maxVertices);

  void EndDefinition();

  void AddVertexBuffer(
    u32 idx,
    const GfxType *types,
    const GfxAttr *attrs,
    u32 attrCount,
    GpuBuffer::Strategy strategy,
    u32 perInstance,
    const void *data);

  void AddIndexBuffer(
    u32 idx,
    GfxType type,
    GpuBuffer::Strategy strategy,
    u32 dataLength,
    const void *data);

private:
  // Shared format table.
  VertexData::Attribute m_attributes[kMaxShaderAttributes];
  // Per-buffer stride/flags.
  VertexData::VertexBufferInfo m_bufferInfo[kMaxVertexBuffers];
  // Attributes appended.
  u32 m_attributeCount = 0;
  // Element count for per-vertex buffers.
  u32 m_vertexCapacity = 0;
  // Element count for per-instance buffers.
  u32 m_instanceCapacity = 0;
  // Vertex buffers.
  GpuBuffer m_vtxBuffers[kMaxVertexBuffers];
  // TODO:
  VertexData::UnknownBuffer m_unknown[4];
  // Index buffer contexts.
  VertexData::IndexBuffer m_idxBuffers[kMaxIndexBuffers];
  bool m_complete = false;
  AutoLister<VertexData> m_lister = {};
  // TODO: Active vertex data pointer.
  VertexData *m_active = nullptr;
  // Name/Identifier.
  const char *m_name = "";
};

class VertexRender {
private:
  VertexRender(const VertexRender &) = default;
  VertexRender(VertexRender &&) = default;
  VertexRender &operator=(const VertexRender &) = default;

public:
  enum: u08 {
    kPrimitiveType_PointList = 0,
    kPrimitiveType_LineList,
    kPrimitiveType_LineStrip = 3,
    kPrimitiveType_TriangleList,
    kPrimitiveType_TriangleStrip,
    kPrimitiveType_TriangleFan,
  };

  VertexRender();
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

  inline void SetPrimitiveCapacity(i32 count) { Assert(!m_queued); m_maxPrimitive = count; }

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

  void Release();

  void Queue();
  void Dequeue();

protected:
  VertexData *m_vertexData = nullptr;
  RenderList *m_renderList = nullptr;
  ShaderProgram *m_shaderProgram = nullptr;
  cstring m_debugName = nullptr;
  Matrix4 m_matrix;
  u08 unk_2[84] = {0};
  i32 m_renderPipeline = 0;
  PipelineInstance m_pipelineInstance = {};
  // Index of currently active index buffer.
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
};

class VertexRenderSparse: public VertexRender {
private:
  VertexRenderSparse(const VertexRenderSparse &) = default;
  VertexRenderSparse(VertexRenderSparse &&) = default;
  VertexRenderSparse &operator=(const VertexRenderSparse &) = default;

private:
  struct Chunk {
    u32 idxOffset = 0;
    u32 idxCount = 0;
    u32 vtxOffset = 0;
    u32 vtxCount = 0;
    u32 unk_1 = 0;
  };

public:
  VertexRenderSparse() = default;
  ~VertexRenderSparse() = default;

  void AllocVertexSparse(
    bool useChunks,
    Heap *heap,
    u32 maxChunkCount);

protected:
  Heap *m_heap = nullptr;
  VertexRenderSparse::Chunk *m_chunkData = nullptr;
  u16 m_chunkCount = 0;
  u16 m_chunkCapacity = 0;
  u08 unk_9[12] = {0};
};

#endif
