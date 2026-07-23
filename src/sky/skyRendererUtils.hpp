#ifndef __SKY_SKYRENDERERUTILS_HPP__
#define __SKY_SKYRENDERERUTILS_HPP__

#include <Utils/Types.h>
#include "sky/skyGfx.hpp"

using RendererConfiguration = void *;
using RendererCallbacks = void *;
using RendererCache = void *;

// ----------------------------------------------------------------------------
// [SECTION] RendererUtils/Renderer
// ----------------------------------------------------------------------------

class Renderer {
public:
  Renderer() = default;
  virtual ~Renderer() = default;

  virtual void Initialize(
    const RendererConfiguration &config,
    const RendererCallbacks &callbacks,
    const RendererCache &cache
  ) = 0;

  virtual void Unk_2() = 0;
  virtual void Unk_3() = 0;
  virtual void Unk_4() = 0;
  virtual void Unk_5() = 0;
  virtual void Unk_6() = 0;
  virtual void Unk_7() = 0;
  virtual void Unk_8() = 0;
  virtual void Unk_9() = 0;
  virtual void Unk_10() = 0;
  virtual void Unk_11() = 0;
  virtual void Unk_12() = 0;
  virtual void Unk_13() = 0;
  virtual void Unk_14() = 0;
  virtual void Unk_15() = 0;
  virtual void Unk_16() = 0;
  virtual void Unk_17() = 0;
  virtual void Unk_18() = 0;
  virtual void Unk_19() = 0;
  virtual void Unk_20() = 0;
  virtual void Unk_21() = 0;

  virtual u32 GetBufferAlignmentRequirements(GfxBufferType type) = 0;

  virtual void Unk_23() = 0;
  virtual void Unk_24() = 0;
  virtual void Unk_25() = 0;
  virtual void Unk_26() = 0;
  virtual void Unk_27() = 0;
  virtual void Unk_28() = 0;
  virtual void Unk_29() = 0;
  virtual void Unk_30() = 0;
  virtual void Unk_31() = 0;
  virtual void Unk_32() = 0;
  virtual void Unk_33() = 0;
  virtual void Unk_34() = 0;
  virtual void Unk_35() = 0;
  virtual void Unk_36() = 0;
  virtual void Unk_37() = 0;
  virtual void Unk_38() = 0;
  virtual void Unk_39() = 0;
  virtual void Unk_40() = 0;
  virtual void Unk_41() = 0;
  virtual void Unk_42() = 0;
  virtual void Unk_43() = 0;
  virtual void Unk_44() = 0;
  virtual void Unk_45() = 0;
  virtual void Unk_46() = 0;
  virtual void Unk_47() = 0;
  virtual void Unk_48() = 0;
  virtual void Unk_49() = 0;
  virtual void Unk_50() = 0;
  virtual void Unk_51() = 0;
  virtual void Unk_52() = 0;
  virtual void Unk_53() = 0;
  virtual void Unk_54() = 0;
  virtual void Unk_55() = 0;
  virtual void Unk_56() = 0;
  virtual void Unk_57() = 0;
  virtual void Unk_58() = 0;
  virtual void Unk_59() = 0;
  virtual void Unk_60() = 0;
  virtual void Unk_61() = 0;
  virtual void Unk_62() = 0;
  virtual void Unk_63() = 0;
  virtual void Unk_64() = 0;
  virtual void Unk_65() = 0;
  virtual void Unk_66() = 0;
  virtual void Unk_67() = 0;
  virtual void Unk_68() = 0;

  // - Buffer Functions

  virtual i32 CreateBuffer(
    cstring tag,
    GfxBufferType type,
    GfxBind usage,
    u32 size
  ) = 0;
  virtual i32 CreateSharedBuffer(
    cstring tag,
    GfxBufferType type,
    u32 size,
    const void *initial,
    u32 paddedSize
  ) = 0;
  virtual bool IsSharedBufferSupported() = 0;
  virtual bool IsSharedBufferWritten(i32 buffer) const = 0;
  virtual void ReleaseBuffer(i32 buffer) = 0;
  virtual void *MapBuffer(i32 buffer) = 0;
  virtual void UnmapBuffer(i32 buffer) = 0;
  virtual void CopyBufferToImage(
    i32 dstImage,
    const void *regions,
    u32 regionCount,
    i32 srcBuffer,
    u32 srcOffset
  ) = 0;
  virtual void CopyBuffer(
    i32 srcBuffer,
    u32 srcSize,
    i32 dstBuffer,
    u32 dstSize,
    u32 copySize
  ) = 0;
  virtual u32 GetBufferSize(i32 buffer) = 0;
  virtual u32 GetPaddedBufferSize(i32 buffer) = 0;
  virtual bool IsBufferCPUCoherent(i32 buffer) = 0;

  // - Texture Functions

  virtual void CreateTexture() = 0;
  virtual void Unk_82() = 0;
  virtual void Unk_83() = 0;
  virtual void Unk_84() = 0;
  virtual void Unk_85() = 0;
  virtual void Unk_86() = 0;
  virtual void Unk_87() = 0;
  virtual void Unk_88() = 0;
  virtual void Unk_89() = 0;
  virtual void Unk_90() = 0;
  virtual void Unk_91() = 0;

  virtual bool CanUseSingleGeometryCopy() = 0;

  virtual void Unk_93() = 0;
  virtual void Unk_94() = 0;
  virtual void Unk_95() = 0;
  virtual void Unk_96() = 0;
  virtual void Unk_97() = 0;
  virtual void Unk_98() = 0;
  virtual void Unk_99() = 0;
  virtual void Unk_100() = 0;
  virtual void Unk_101() = 0;

  virtual cstring GetImmutableString(cstring s) = 0;

  virtual void Unk_103() = 0;
};

Renderer *GetRenderer();
void SetRenderer(Renderer *renderer);

// ----------------------------------------------------------------------------
// [SECTION] RendererUtils/GpuBuffer
// ----------------------------------------------------------------------------

class GpuBuffer {
public:
  enum Strategy: u08 { };

  GpuBuffer() = default;
  ~GpuBuffer() = default;

  void Initialize(
    cstring name,
    GfxBufferType type,
    GfxBind usage,
    u32 size,
    const void *data = nullptr);
  void Terminate();

  void *MapBuffer();
  void UnmapBuffer();

  inline u32 GetPaddedSize() { return (m_bufferSize + 16 * m_alignment - 1) & (-16 * m_alignment); }
  inline u32 GetSize() { return m_bufferSize; }

private:
  i32 m_readableBuffer = -1;
  i32 m_writableBuffer = -1;
  u32 m_bufferSize = 0;
  GfxBind m_usage = kGfxBind_Undefined;
  GfxBufferType m_type = kGfxBufferType_Undefined;

  u08 m_writeCount: 2;
  u08 m_writeIndex: 2;
  u08 m_readCount: 2;
  u08 m_readIndex: 2;

  bool m_isMapped: 1;
  bool m_isSharedWritten: 1;
  bool m_isCpuCoherent: 1;

  u08 m_alignment = 0;
  cstring m_identifier = nullptr;
};

#endif
