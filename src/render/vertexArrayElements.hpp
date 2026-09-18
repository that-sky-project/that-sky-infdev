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
    : x((i08)(255.0f * (x + 1.0f) / 2.0f))
    , y((i08)(255.0f * (y + 1.0f) / 2.0f))
    , z((i08)(255.0f * (z + 1.0f) / 2.0f))
    , w((i08)(255.0f * (w + 1.0f) / 2.0f))
  { }

  inline operator u32() { return ((u32)(u08)x) | (((u32)(u08)y) << 8) | (((u32)(u08)z) << 16) | (((u32)(u08)w) << 24); }
  inline operator i32() { return (i32)(operator u32()); }

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
    : x((i08)(255.0f * x))
    , y((i08)(255.0f * y))
    , z((i08)(255.0f * z))
    , w((i08)(255.0f * w))
  { }

  inline operator u32() { return ((u32)x) | (((u32)y) << 8) | (((u32)z) << 16) | (((u32)w) << 24); }
  inline operator i32() { return (i32)(operator u32()); }

  u08 x;
  u08 y;
  u08 z;
  u08 w;
};

struct TerrainMaterialVertex {
  static constexpr u32 kNumAttrs = 5;
  static constexpr GfxType kTypes[kNumAttrs] = {kGfxType_FLOAT3, kGfxType_BYTE4, kGfxType_UBYTE4, kGfxType_UBYTE4, kGfxType_UBYTE4};
  static constexpr GfxAttr kAttrs[kNumAttrs] = {kGfxAttr_Position, kGfxAttr_Normal, kGfxAttr_Light0, kGfxAttr_Light1, kGfxAttr_Light2};

  ~TerrainMaterialVertex() = default;
  TerrainMaterialVertex() = default;

  TerrainMaterialVertex(
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

using GrassShVertex = TerrainMaterialVertex;

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

struct GrassShMeshVertex {
  static constexpr u32 kNumAttrs = 6;
  static constexpr GfxType kTypes[kNumAttrs] = {kGfxType_FLOAT3, kGfxType_BYTE4, kGfxType_UBYTE4, kGfxType_UBYTE4, kGfxType_UBYTE4, kGfxType_UBYTE4};
  static constexpr GfxAttr kAttrs[kNumAttrs] = {kGfxAttr_Position, kGfxAttr_Normal, kGfxAttr_Light0, kGfxAttr_Light1, kGfxAttr_Light2, kGfxAttr_Color};

  ~GrassShMeshVertex() = default;
  GrassShMeshVertex() = default;

  GrassShMeshVertex(
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
  u32 a_color  = 0xFFFFFFFF;
};

#endif
