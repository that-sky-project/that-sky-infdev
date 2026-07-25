#include <Windows.h>
#include "utils/rva.hpp"
#include "sky/skyPrivate.hpp"
#include "sky/skyVertex.hpp"

// ----------------------------------------------------------------------------
// [SECTION] VertexData
// ----------------------------------------------------------------------------

u32 VertexData::GetTotalSize() {
  u32 result = 0;

  for (auto &it: m_vtxBuffers)
    result += it.GetTotalMemSize();
  for (auto &it: m_unknown)
    result += it.buffer.GetTotalMemSize();
  for (auto &it: m_idxBuffers)
    result += it.buffer.GetTotalMemSize();

  return result;
}

void VertexData::Release() {
  if (!m_complete)
    return;

  for (auto &it: m_vtxBuffers)
    it.Terminate();

  for (auto &it: m_unknown)
    it.Reset();
  for (auto &it: m_idxBuffers)
    it.Reset();
  for (auto &it: m_bufferInfo)
    it.Reset();
  for (auto &it: m_attributes)
    it.Reset();

  m_attributeCount = 0;
  m_maxInstances = 0;
  m_active = nullptr;
  m_complete = false;
}

void VertexData::BeginDefinition(
  cstring name,
  u32 maxVertices
) {
  AssertMsg(m_complete, "VertexData must be Release()'d before it can be redefined");
  m_maxVertices = maxVertices;
}

void VertexData::AddVertexBuffer(
  u32 idx,
  const GfxType *types,
  const GfxAttr *attrs,
  u32 attrCount,
  GpuBuffer::Strategy strategy,
  u32 perInstance,
  const void *data
) {
  Assert(idx < kMaxVertexBuffers);

  u32 baseAttr = m_attributeCount;
  Assert(baseAttr + attrCount <= kMaxShaderAttributes);

  // Append every attribute of this buffer to the shared format table,
  // accumulating the per-element stride as we go.
  u32 stride = 0;
  for (u32 i = 0; i < attrCount; ++i) {
    Attribute &attr = m_attributes[baseAttr + i];
    GfxType type = types[i];

    attr.attr = attrs[i];
    attr.type = type;
    attr.bufferIdx = (u16)idx;
    attr.offset = stride;

    u32 size = 0;
    u08 typeIdx = type - 1;
    if (typeIdx <= kGfxType_COUNT - 2)
      size = kGfxTypeSizes[typeIdx];
    stride += size;
  }

  m_attributeCount = baseAttr + attrCount;
  m_bufferInfo[idx].stride = (u16)stride;
  m_bufferInfo[idx].perInstance = (u16)perInstance;

  // The buffer holds one element per vertex, or per instance when flagged.
  u32 count = perInstance ? m_maxInstances : m_maxVertices;

  char buffer[64] = {0};
  snprintf(buffer, sizeof(buffer), "%s_%u", m_name, idx);

  m_vtxBuffers[idx].Initialize(
    buffer,
    kGfxBufferType_Vertex,
    strategy,
    count * stride,
    data);
}

void VertexData::AddIndexBuffer(
  u32 idx,
  GfxType type,
  GpuBuffer::Strategy strategy,
  u32 dataLength,
  const void *idxBuffer
) {
  Assert(idx < kMaxIndexBuffers);

  m_idxBuffers[idx].type = type;
  m_idxBuffers[idx].unk_3 = 0;
  m_idxBuffers[idx].capacity = dataLength;

  u32 elementSize = 0;
  if (type < kGfxType_COUNT)
    elementSize = kGfxTypeSizes[type];

  m_idxBuffers[idx].buffer.Initialize(
    m_name,
    kGfxBufferType_Index,
    strategy,
    elementSize * dataLength,
    idxBuffer);
}

void VertexData::EndDefinition() {
  m_complete = true;
  m_active = this;
}

// ----------------------------------------------------------------------------
// [SECTION] VertexRender
// ----------------------------------------------------------------------------

VertexRender::VertexRender() {
  using PFN = void (*)(VertexRender *);
  static const Rva s_VertexRender_ctor = 0x000FEEE0;
  ((PFN)s_VertexRender_ctor())(this);
}

void VertexRender::Initialize(
  VertexData *renderData,
  const ShaderProgram *shader,
  RenderList *renderList,
  u32 a5,
  void *a6
) {
  using PFN = void (*)(
    VertexRender *, VertexData *, const ShaderProgram *, RenderList *, u32, void *);
  static const Rva s_VertexRender_Initialize = 0x000FF470;
  ((PFN)s_VertexRender_Initialize())(
    this, renderData, shader, renderList, a5, a6);
}

void VertexRender::Initialize(
  VertexData *renderData,
  ResourceManager *resources,
  cstring resourceName,
  RenderList *renderList,
  u32 a5,
  void *a6
) {
  using PFN = void (*)(
    VertexRender *, VertexData *, ResourceManager *, cstring, RenderList *, u32, void *);
  static const Rva s_VertexRender_Initialize = 0x000FF140;
  ((PFN)s_VertexRender_Initialize())(
    this, renderData, resources, resourceName, renderList, a5, a6);sizeof(VertexRender);
}

void VertexRender::AllocVertexSparse(
  bool a2,
  Heap *heap,
  u32 a4
) {
  using PFN = void (*)(VertexRender *, bool, Heap *, u32);
  static const Rva s_VertexRender_AllocVertexSparse = 0x001014D0;
  ((PFN)s_VertexRender_AllocVertexSparse())(this, a2, heap, a4);
}

void VertexRender::Queue() {
  using PFN = void (*)(VertexRender *);
  static const Rva s_VertexRender_Queue = 0x001002A0;
  ((PFN)s_VertexRender_Queue())(this);
}
