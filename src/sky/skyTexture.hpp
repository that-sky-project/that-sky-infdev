#ifndef __SKY_SKYTEXTURE_HPP__
#define __SKY_SKYTEXTURE_HPP__

#include <Utils/Types.h>
#include "sky/skyGfx.hpp"

struct TextureDecriptor {

};

class Texture {
public:
  Texture() = default;
  ~Texture() = default;

  void Initialize2D(
    cstring name,
    GfxBind strategy,
    GfxTexelFormat format,
    u16 width,
    u16 height,
    u32 mipmapLevel,
    const void *data,
    u32 dataSize,
    u32 a10,
    bool a11);

private:
  u64 _align = 0;
  u08 _unk[104 - 8] = {0};
};

#endif
