#ifndef __SKY_SKYVERTEX_HPP__
#define __SKY_SKYVERTEX_HPP__

#include <Utils/Types.h>
#include "sky/skyGfx.hpp"
#include "sky/skyTypePlaceholders.hpp"

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
  explicit VertexRender();
  ~VertexRender() = default;

  PipelineInstance *GetPipelineInstance();

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
    bool a2,
    Heap *heap,
    u32 a4);

  void Queue();

private:
  __m128 _align;
  u08 _gap[528 - 16];
};

#endif
