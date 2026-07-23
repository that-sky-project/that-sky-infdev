#include <cstring>
#include "sky/skyPrivate.hpp"
#include "sky/skyRendererUtils.hpp"

void GpuBuffer::Initialize(
  cstring name,
  GfxBufferType type,
  GfxBind usage,
  u32 size,
  const void *data
) {
  // Reset status.
  m_writeCount = m_writeIndex = m_readCount = m_readIndex = 0;
  m_isMapped = m_isCpuCoherent = m_isSharedWritten = false;

  m_usage = usage;
  m_bufferSize = size;
  m_type = type;

  switch (usage) {
    case kGfxBind_UploadSingle:
      m_writeCount = 1;
      m_readCount = 1;
      break;
    case kGfxBind_UploadTriple:
    case kGfxBind_UploadFlexible:
      m_writeCount = 3;
      m_readCount = 3;
      break;
    case kGfxBind_DownloadTriple:
      m_readCount = 3;
      m_isSharedWritten = true;
      break;
    default:
      break;
  }

  // Vertex/Index buffers with triple upload fall back to a single readable
  // copy when the renderer reports it can source them directly (Unk_92).
  if (
    (type == kGfxBufferType_Vertex || type == kGfxBufferType_Index)
    && GetRenderer()->CanUseSingleGeometryCopy()
    && m_usage == kGfxBind_UploadTriple
  )
    m_readCount = 1;

  u32 alignment = GetRenderer()->GetBufferAlignmentRequirements(type);
  AssertMsg((alignment % 16) == 0, "Render is providing an alignment that is not a multiple of 16 bytes");
  AssertMsg((alignment / 16) < 256, "Render is providing an alignment that is too large to hold");

  m_alignment = alignment / 16;

  // Shared buffers are only attempted for Uniform/Vertex/Index single uploads
  // that carry initial data.
  bool isSharedBuffer = false;
  if (
    (type >= kGfxBufferType_Uniform && type <= kGfxBufferType_Index)
    && usage == kGfxBind_UploadSingle && data
  )
    isSharedBuffer = GetRenderer()->IsSharedBufferSupported();

  cstring tag = GetRenderer()->GetImmutableString(name);
  m_identifier = tag;

  // Round the requested size up to the alignment (in units of 16 bytes).
  u32 realSize = m_alignment
    ? (m_bufferSize + 16 * m_alignment - 1) & (-16 * m_alignment)
    : m_bufferSize;

  // Try the shared (unified read/write) buffer path first when eligible.
  if (isSharedBuffer) {
    Assert(m_readCount == 1);

    m_readableBuffer = GetRenderer()->CreateSharedBuffer(tag, type, realSize, data, size);
    if (
      m_readableBuffer >= 0
      && GetRenderer()->IsSharedBufferWritten(m_readableBuffer)
    ) {
      m_isSharedWritten = true;
      m_writeCount = 0;
      m_writableBuffer = -1;
      m_isCpuCoherent = GetRenderer()->IsBufferCPUCoherent(m_readableBuffer);
      return;
    }
  }

  // Fall back to a plain readable buffer when no shared buffer was produced.
  if (m_readableBuffer == -1) {
    // readCount >= 2 always maps to a triple upload; otherwise mirror usage.
    GfxBind readUsage = (m_readCount >= 2)
      ? kGfxBind_UploadTriple
      : m_usage == kGfxBind_UploadTriple
        ? kGfxBind_UploadTriple
        : kGfxBind_UploadSingle;
    m_readableBuffer = GetRenderer()->CreateBuffer(tag, type, readUsage, realSize * m_readCount);
  }

  m_isCpuCoherent = GetRenderer()->IsBufferCPUCoherent(m_readableBuffer);
  if (m_isCpuCoherent) {
    // Coherent memory can be written directly; alias the writable handle.
    m_writeCount = m_readCount;
    m_writableBuffer = m_readableBuffer;
  } else if (m_writeCount) {
    // Need a separate staging buffer to feed the device-local readable one.
    m_writableBuffer = GetRenderer()->CreateBuffer(
      tag,
      kGfxBufferType_Staging,
      kGfxBind_UploadTriple,
      realSize * m_writeCount);
  } else {
    m_writableBuffer = -1;
  }

  if (!data)
    return;

  // Upload the initial contents, zero-filling any alignment padding.
  void *mapped = MapBuffer();
  if (mapped) {
    memcpy(mapped, data, size);
    if (realSize > size)
      memset((u08 *)mapped + size, 0, realSize - size);
    UnmapBuffer();
  }
}

void *GpuBuffer::MapBuffer() {
  
}
