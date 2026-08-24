#ifndef __WORLD_SYNTH_PERLINNOISE_HPP__
#define __WORLD_SYNTH_PERLINNOISE_HPP__

#include <Utils/Types.h>
#include "world/synth/synth.hpp"
#include "world/synth/random.hpp"
#include "world/synth/improvedNoise.hpp"

// Layered (multi-octave) Perlin noise, ported from the Minecraft Pocket
// Edition codebase leakage (net.minecraft.world.level.levelgen.synth.PerlinNoise).
class PerlinNoise: public Synth {
public:
  PerlinNoise(i32 levels) {
    m_rndPtr = &m_random;
    Initialize(levels);
  }

  PerlinNoise(Random *random, i32 levels) {
    m_rndPtr = random;
    Initialize(levels);
  }

  ~PerlinNoise() {
    for (i32 i = 0; i < m_levels; ++i)
      delete m_noiseLevels[i];
    delete[] m_noiseLevels;
  }

  inline void Initialize(i32 levels) {
    m_levels = levels;
    m_noiseLevels = new ImprovedNoise *[levels];
    for (i32 i = 0; i < levels; i++) {
      m_noiseLevels[i] = new ImprovedNoise(m_rndPtr);
    }
  }

  inline virtual f32 GetValue(
    f32 x,
    f32 y
  ) override {
    f32 value = 0;
    f32 pow = 1;
    for (i32 i = 0; i < m_levels; i++) {
      value += m_noiseLevels[i]->GetValue(x * pow, y * pow) / pow;
      pow /= 2;
    }
    return value;
  }

  inline virtual f32 GetValue(
    f32 x,
    f32 y,
    f32 z
  ) override {
    f32 value = 0;
    f32 pow = 1;
    for (i32 i = 0; i < m_levels; i++) {
      value += m_noiseLevels[i]->GetValue(x * pow, y * pow, z * pow) / pow;
      pow /= 2;
    }
    return value;
  }

  inline f32 *GetRegion(
    f32 *buffer,
    f32 x,
    f32 y,
    f32 z,
    i32 xSize,
    i32 ySize,
    i32 zSize,
    f32 xScale,
    f32 yScale,
    f32 zScale
  ) {
    const i32 size = xSize * ySize * zSize;
    if (buffer == nullptr) {
      buffer = new f32[size];
    }
    for (i32 i = 0; i < size; i++)
      buffer[i] = 0;

    f32 pow = 1;
    for (i32 i = 0; i < m_levels; i++) {
      m_noiseLevels[i]->Add(buffer, x, y, z, xSize, ySize, zSize, xScale * pow, yScale * pow, zScale * pow, pow);
      pow /= 2;
    }

    return buffer;
  }

  inline f32 *GetRegion(
    f32 *sr,
    i32 x,
    i32 z,
    i32 xSize,
    i32 zSize,
    f32 xScale,
    f32 zScale,
    f32 pow
  ) {
    return GetRegion(sr, (f32)x, 10.0f, (f32)z, xSize, 1, zSize, xScale, 1, zScale);
  }

  inline i32 HashCode() {
    i32 x = 4711;
    for (i32 i = 0; i < m_levels; ++i)
      x *= m_noiseLevels[i]->HashCode();
    return x;
  }

private:
  ImprovedNoise **m_noiseLevels;
  i32 m_levels;

  Random m_random;
  Random *m_rndPtr;
};

#endif
