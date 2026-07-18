#include <Windows.h>
#include "utils/rva.hpp"
#include "sky/skyVertex.hpp"

// ----------------------------------------------------------------------------
// [SECTION] VertexData
// ----------------------------------------------------------------------------

VertexData::VertexData() {
  using PFN = void (*)(VertexData *);
  static const Rva s_VertexData_ctor = 0x000FE2B0;
  ((PFN)s_VertexData_ctor())(this);
}

VertexData::~VertexData() {
  using PFN = void (*)(VertexData *);
  static const Rva s_VertexData_dtor = 0x000FE470;
  ((PFN)s_VertexData_dtor())(this);
}

void VertexData::BeginDefinition(
  cstring name,
  u32 maxVertices
) {
  using PFN = void (*)(VertexData *, cstring, u32);
  static const Rva s_VertexData_BeginDefinition = 0x000FE620;
  ((PFN)s_VertexData_BeginDefinition())(this, name, maxVertices);
}

void VertexData::AddVertexBuffer(
  u32 idx,
  const GfxType *types,
  const GfxAttr *attrs,
  u32 attrCount,
  GpuBuffer::Strategy strategy,
  u32 flags,
  const void *vtxData
) {
  using PFN = void (*)(
    VertexData *, u32, const GfxType *, const GfxAttr *, u32, GpuBuffer::Strategy, u32, const void *);
  static const Rva s_VertexData_AddVertexBuffer = 0x000FE890;
  ((PFN)s_VertexData_AddVertexBuffer())(
    this, idx, types, attrs, attrCount, strategy, flags, vtxData);
}

void VertexData::AddIndexBuffer(
  u32 idx,
  GfxType type,
  GpuBuffer::Strategy strategy,
  u32 count,
  const void *idxBuffer
) {
  using PFN = void (*)(
    VertexData *, u32, GfxType, GpuBuffer::Strategy, u32, const void *);
  static const Rva s_VertexData_AddIndexBuffer = 0x000FED50;
  ((PFN)s_VertexData_AddIndexBuffer())(
    this, idx, type, strategy, count, idxBuffer);
}

void VertexData::EndDefinition() {
  using PFN = void (*)(VertexData *);
  static const Rva s_VertexData_EndDefinition = 0x000FEEC0;
  ((PFN)s_VertexData_EndDefinition())(this);
}

// ----------------------------------------------------------------------------
// [SECTION] VertexRender
// ----------------------------------------------------------------------------

VertexRender::VertexRender() {
  using PFN = void (*)(VertexRender *);
  static const Rva s_VertexRender_ctor = 0x000FEEE0;
  ((PFN)s_VertexRender_ctor())(this);
}

PipelineInstance *VertexRender::GetPipelineInstance() {
  using PFN = PipelineInstance *(*)(VertexRender *);
  static const Rva s_VertexRender_GetPipelineInstance = 0x000E8C90;
  return ((PFN)s_VertexRender_GetPipelineInstance())(this);
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
    this, renderData, resources, resourceName, renderList, a5, a6);
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
