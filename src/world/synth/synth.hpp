#ifndef __WORLD_SYNTH_SYNTH_HPP__
#define __WORLD_SYNTH_SYNTH_HPP__

#include <Utils/Types.h>

// Base class for world-generation noise synthesizers, ported from the
// Minecraft Pocket Edition codebase leakage.
class Synth {
public:
  Synth() = default;
  virtual ~Synth() = default;

  virtual f32 GetValue(f32 x, f32 y) = 0;
  virtual f32 GetValue(f32 x, f32 y, f32 z) = 0;

  inline i32 GetDataSize(
    i32 width,
    i32 height
  ) {
    return width * height * (i32)sizeof(f32);
  }

  inline void Create(
    f32 *result,
    i32 width,
    i32 height
  ) {
    for (i32 y = 0; y < height; y++) {
      for (i32 x = 0; x < width; x++) {
        result[x + y * width] = GetValue((f32)x, (f32)y);
      }
    }
  }

  inline void Create(
    f32 *result,
    i32 width,
    i32 height,
    i32 depth
  ) {
    for (i32 z = 0; z < depth; z++) {
      for (i32 y = 0; y < height; y++) {
        for (i32 x = 0; x < width; x++) {
          result[x + width * (y + height * z)] = GetValue((f32)x, (f32)y, (f32)z);
        }
      }
    }
  }
};

#endif
