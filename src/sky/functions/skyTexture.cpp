#include "utils/rva.hpp"
#include "sky/skyRendererUtils.hpp"
#include "sky/skyTexture.hpp"

u32 Texture::GetTotalMemSize() const {

}

u32 Texture::GetImageMemSize(
  bool isBaseMipOnly,
  bool isAligned
) const {

}

void Texture::Initialize2D(
  cstring name,
  Texture::Strategy strategy,
  GfxTexelFormat format,
  u16 width,
  u16 height,
  u32 mipmapLevel,
  const void *data,
  u32 dataSize,
  bool isAutoMipChain,
  bool isRenderTarget
) {
  m_usage = strategy;
  m_descriptor.type = kGfxTextureType_2D;
  m_descriptor.sampleCount = strategy == kGfxBind_UploadTriple ? 2 : 1;
  m_descriptor.format = format;
  m_descriptor.width = width;
  m_descriptor.height = height;

  if (!mipmapLevel)
    mipmapLevel = CalculateMipCount(width, height, m_descriptor.depth);
  m_descriptor.mipmapLevel = (u08)mipmapLevel;

  m_descriptor.unk_2 = 4 * (u08)(isRenderTarget | (strategy == kGfxBind_UploadSingle));
  if (isAutoMipChain && mipmapLevel > 1) {
    m_autoMipChain = true;
    m_descriptor.unk_2 |= 0x40;
  }

  m_writeCount = m_readCount = (strategy == kGfxBind_UploadTriple ? 3 : 1);
  m_writeIndex = m_readIndex = 0;

  m_imageMemSize = GetImageMemSize(m_autoMipChain, true);
  AssertMsg(
    !data || GetImageMemSize(m_autoMipChain, false) == dataSize,
    "Texture %s was passed an incorrect amount of data (%u vs %u required), one"
    " of us is confused about size and alignment requirements",
    name,
    dataSize,
    m_imageMemSize
  );

  Initialize(name, data);
}

void Texture::Initialize(
  cstring name,
  const void *data
) {
  m_debugInfo.Initialize(this);

  unk_7 = 0;
  strncpy(m_name, name, sizeof(m_name));
  unk_6 = m_descriptor.unk_2 ? 4 : 0;

  // Create textures.
  for (i32 i = 0; i < m_readCount; i++)
    m_textures[i] = GetRenderer()->CreateTexture(name, m_descriptor);

  u32 eachBuffer = m_imageMemSize;
  AssertMsg(
    !m_writeCount || eachBuffer > 0,
    "There was a problem creating %s because we couldn't calculate its size",
    name
  );

  // Create intermediate writable buffer.
  m_buffer = Renderer::kInvalidHandle;
  if (m_writeCount) {
    m_buffer = GetRenderer()->CreateBuffer(
      name,
      kGfxBufferType_Staging,
      m_usage == kGfxBind_UploadTriple ? kGfxBind_UploadTriple : kGfxBind_UploadSingle,
      m_writeCount * eachBuffer
    );
  }

  // Prefill initial data.
  if (data) {
    void *mapped = MapBuffer();
    if (mapped) {
      memcpy(mapped, data, GetImageMemSize(m_autoMipChain, 0));
      UnmapBuffer(nullptr, 0);
    }
  }
}

void Texture::Terminate() {
  m_debugInfo.Terminate();
  m_usage = kGfxBind_Undefined;

  // Reset textures.
  if (!m_isExternalTexture && (m_isMapped || m_isUnmapped)) {
    for (i32 i = 0; i < m_readCount; i++) {
      if (m_textures[i] != Renderer::kInvalidHandle)
        GetRenderer()->ReleaseTexture(m_textures[i]);
      m_textures[i] = Renderer::kInvalidHandle;
    }
  }

  // Reset buffer.
  if (m_buffer != Renderer::kInvalidHandle)
    GetRenderer()->ReleaseBuffer(m_buffer);
  m_buffer = Renderer::kInvalidHandle;

  // Reset descriptor.
  m_descriptor.Reset();

  // Reset flags.
  m_readCount = m_readIndex = m_writeCount = m_writeIndex = 0;
  m_isMapped = m_isUnmapped = m_autoMipChain = m_isExternalTexture = false;
}

void *Texture::MapBuffer() {
  m_isMapped = true;
  m_writeIndex = (m_writeIndex + 1) % m_writeCount;

  void *mapped = GetRenderer()->MapBuffer(m_buffer);
  if (!mapped)
    return nullptr;
  return (u08 *)mapped + m_imageMemSize * m_writeIndex;
}
