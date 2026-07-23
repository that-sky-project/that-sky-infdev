#ifndef __RENDER_VERTEXARRAYELEMENTS_HPP__
#define __RENDER_VERTEXARRAYELEMENTS_HPP__

#include <Utils/Types.h>
#include "sky/skyGfx.hpp"

struct R8G8B8A8_SNORM {
  ~R8G8B8A8_SNORM() = default;
  R8G8B8A8_SNORM(): x(0), y(0), z(0), w(0) { }
  R8G8B8A8_SNORM(
    f32 x,
    f32 y,
    f32 z,
    f32 w
  )
    : x(255 * (x + 1) / 2)
    , y(255 * (y + 1) / 2)
    , z(255 * (z + 1) / 2)
    , w(255 * (w + 1) / 2)
  { }

  i08 x;
  i08 y;
  i08 z;
  i08 w;
};

struct R8G8B8A8_UNORM {
  ~R8G8B8A8_UNORM() = default;
  R8G8B8A8_UNORM(): x(0), y(0), z(0), w(0) { }
  R8G8B8A8_UNORM(
    f32 x,
    f32 y,
    f32 z,
    f32 w
  )
    : x(255 * x)
    , y(255 * y)
    , z(255 * z)
    , w(255 * w)
  { }

  u08 x;
  u08 y;
  u08 z;
  u08 w;
};

struct GrassShVertex {
  static constexpr u32 kNumAttrs = 5;
  static constexpr GfxType kTypes[kNumAttrs] = {kGfxType_FLOAT3, kGfxType_BYTE4, kGfxType_UBYTE4, kGfxType_UBYTE4, kGfxType_UBYTE4};
  static constexpr GfxAttr kAttrs[kNumAttrs] = {kGfxAttr_Position, kGfxAttr_Normal, kGfxAttr_Light0, kGfxAttr_Light1, kGfxAttr_Light2};

  ~GrassShVertex() = default;
  GrassShVertex() = default;

  GrassShVertex(
    f32 x,
    f32 y,
    f32 z
  ) {
    a_position[0] = x;
    a_position[1] = y;
    a_position[2] = z;
  }

  f32 a_position[3] = {0};
  u32 a_normal = 0x000000FF;
  u32 a_light0 = 0x7F7F7F7F;
  u32 a_light1 = 0x0000FFB3;
  u32 a_light2 = 0xFF80FF80;
};

struct TerrainDepthVertex {
  static constexpr u32 kNumAttrs = 1;
  static constexpr GfxType kTypes[kNumAttrs] = {kGfxType_FLOAT3};
  static constexpr GfxAttr kAttrs[kNumAttrs] = {kGfxAttr_Position};

  ~TerrainDepthVertex() = default;
  TerrainDepthVertex() = default;

  TerrainDepthVertex(
    f32 x,
    f32 y,
    f32 z
  ) {
    a_position[0] = x;
    a_position[1] = y;
    a_position[2] = z;
  }

  f32 a_position[3] = {0};
};

#endif
