#ifndef __SKY_SKYTEXTURE_HPP__
#define __SKY_SKYTEXTURE_HPP__

#include <cmath>
#include <Utils/Types.h>
#include "sky/skyAutoLister.hpp"
#include "sky/skyGfx.hpp"

class Texture;

struct TextureDebugInfo {
  inline void Initialize(Texture *p) { active = p; }
  inline void Terminate() { active = nullptr; }

  TextureDebugInfo *next = nullptr;
  TextureDebugInfo *prev = nullptr;
  Texture *active = nullptr;
};

struct TextureDescriptor {
  inline void Reset() {
    type = kGfxTextureType_Undefined;
    format = kGfxTexelFormat_Undefined;

    sampleCount = 0;
    unk_1 = 0;
    unk_2 = 0;

    width = 1;
    height = 1;
    depth = 1;
    mipmapLevel = 1;

    unk_3 = 1;
    layerCount = 1;

    unk_4[0] = 1;
    memset(&unk_4[1], 0, 8);
  }

  GfxTextureType type = kGfxTextureType_Undefined;
  GfxTexelFormat format = kGfxTexelFormat_Undefined;

  u08 sampleCount = 0;
  char unk_1 = 0;
  u16 unk_2 = 0;

  u16 width = 1;
  u16 height = 1;
  u16 depth = 1;
  u08 mipmapLevel = 1;

  char unk_3 = 1;
  u08 layerCount = 1;
  char unk_4[9] = {1, 0, 0, 0, 0, 0, 0, 0, 0};
};

class Texture {
private:
  inline static u32 CalculateMipCount(
    u16 width,
    u16 height,
    u16 depth
  ) {
    u16 dim = width;
    if (dim < height) dim = height;
    if (dim < depth) dim = depth;
    return (u32)floor(log2f((f32)dim)) + 1;
  }

public:
  using Strategy = GfxBind;

  ~Texture() = default;
  Texture()
    : m_writeCount(0), m_writeIndex(0)
    , m_readCount(0), m_readIndex(0)
    , m_isMapped(false), m_isUnmapped(false)
    , m_autoMipChain(false), m_isExternalTexture(false) { }
  Texture(Texture &&) = delete;
  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;

  inline i32 GetReadableTexture(i32 offset) const { return m_textures[(offset + m_readIndex) % m_readCount]; }

  u32 GetTotalMemSize() const;

  u32 GetImageMemSize(
    bool isBaseMipOnly,
    bool isAligned
  ) const;

  void Initialize2D(
    cstring name,
    Texture::Strategy strategy,
    GfxTexelFormat format,
    u16 width,
    u16 height,
    u32 mipmapLevel,
    const void *data,
    u32 dataSize,
    bool isAutoMipChain,
    bool isRenderTarget);

  void Initialize(
    cstring name,
    const void *data);

  void Terminate();

  void *MapBuffer();
  void UnmapBuffer(const BatchedPixels *pixels, u32 count);

private:
  TextureDescriptor m_descriptor = {};
  i32 m_textures[3] = {-1, -1, -1};
  i32 m_buffer = -1;
  u32 m_imageMemSize = 0;
  GfxBind m_usage = kGfxBind_Undefined;

  u08 m_readCount: 2;
  u08 m_readIndex: 2;
  u08 m_writeCount: 2;
  u08 m_writeIndex: 2;

  bool m_isMapped: 1;
  bool m_isUnmapped: 1;
  bool m_autoMipChain: 1;
  bool m_isExternalTexture: 1;

  TextureDebugInfo m_debugInfo = {};
  char m_name[29] = {0};
  u08 unk_6 = 0;
  u16 unk_7 = 0;
};

#endif
