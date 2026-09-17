#ifndef __WORLD_SYNTH_IMPROVEDNOISE_HPP__
#define __WORLD_SYNTH_IMPROVEDNOISE_HPP__

#include <Utils/Types.h>
#include "world/synth/synth.hpp"
#include "world/synth/random.hpp"

// 3D Perlin value noise, ported from the Minecraft Pocket Edition codebase
// leakage (net.minecraft.world.level.levelgen.synth.ImprovedNoise).
class ImprovedNoise: public Synth {
public:
  ImprovedNoise() = default;
  ~ImprovedNoise() = default;

  inline void Initialize(Random *random) {
    xo = random->NextFloat() * 256.0f;
    yo = random->NextFloat() * 256.0f;
    zo = random->NextFloat() * 256.0f;
    for (i32 i = 0; i < 256; i++) {
      m_p[i] = i;
    }
    for (i32 i = 0; i < 256; i++) {
      i32 j = random->NextInt(256 - i) + i;
      i32 tmp = m_p[i];
      m_p[i] = m_p[j];
      m_p[j] = tmp;
      m_p[i + 256] = m_p[i];
    }
  }

  inline f32 Noise(f32 _x, f32 _y, f32 _z) {
    f32 x = _x + xo;
    f32 y = _y + yo;
    f32 z = _z + zo;

    i32 xf = (i32)x;
    i32 yf = (i32)y;
    i32 zf = (i32)z;

    if (x < xf) xf--;
    if (y < yf) yf--;
    if (z < zf) zf--;

    // Find unit cube that contains point.
    i32 X = xf & 255;
    i32 Y = yf & 255;
    i32 Z = zf & 255;

    // Find relative x, y, z of point in cube.
    x -= xf;
    y -= yf;
    z -= zf;

    // Compute fade curves for each of x, y, z.
    f32 u = x * x * x * (x * (x * 6 - 15) + 10);
    f32 v = y * y * y * (y * (y * 6 - 15) + 10);
    f32 w = z * z * z * (z * (z * 6 - 15) + 10);

    // Hash coordinates of the 8 cube corners.
    i32 A = m_p[X] + Y, AA = m_p[A] + Z, AB = m_p[A + 1] + Z;
    i32 B = m_p[X + 1] + Y, BA = m_p[B] + Z, BB = m_p[B + 1] + Z;

    // Add blended results from the 8 corners of the cube.
    return Lerp(w,
      Lerp(v,
        Lerp(u, Grad(m_p[AA], x, y, z), Grad(m_p[BA], x - 1, y, z)),
        Lerp(u, Grad(m_p[AB], x, y - 1, z), Grad(m_p[BB], x - 1, y - 1, z))),
      Lerp(v,
        Lerp(u, Grad(m_p[AA + 1], x, y, z - 1), Grad(m_p[BA + 1], x - 1, y, z - 1)),
        Lerp(u, Grad(m_p[AB + 1], x, y - 1, z - 1), Grad(m_p[BB + 1], x - 1, y - 1, z - 1))));
  }

  inline f32 Lerp(f32 t, f32 a, f32 b) {
    return a + t * (b - a);
  }

  inline f32 Grad2(i32 hash, f32 x, f32 z) {
    // Convert lo 4 bits of hash code into gradient directions.
    i32 h = hash & 15;
    f32 u = (1 - ((h & 8) >> 3)) * x;
    f32 v = h < 4 ? 0 : (h == 12 || h == 14) ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
  }

  inline f32 Grad(i32 hash, f32 x, f32 y, f32 z) {
    // Convert lo 4 bits of hash code into gradient directions.
    i32 h = hash & 15;
    f32 u = h < 8 ? x : y;
    f32 v = h < 4 ? y : (h == 12 || h == 14) ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
  }

  inline virtual f32 GetValue(f32 x, f32 y) override {
    return Noise(x, y, 0);
  }

  inline virtual f32 GetValue(f32 x, f32 y, f32 z) override {
    return Noise(x, y, z);
  }

  inline void Add(
    f32* buffer,
    f32 _x,
    f32 _y,
    f32 _z,
    i32 xSize,
    i32 ySize,
    i32 zSize,
    f32 xs,
    f32 ys,
    f32 zs,
    f32 pow
  ) {
    if (ySize == 1) {
      i32 A = 0, AA = 0, B = 0, BA = 0;
      f32 vv0 = 0, vv2 = 0;
      i32 pp = 0;
      f32 scale = 1.0f / pow;
      for (i32 xx = 0; xx < xSize; xx++) {
        f32 x = (_x + xx) * xs + xo;
        i32 xf = (i32)x;
        if (x < xf) xf--;
        i32 X = xf & 255;
        x -= xf;
        f32 u = x * x * x * (x * (x * 6 - 15) + 10);

        for (i32 zz = 0; zz < zSize; zz++) {
          f32 z = (_z + zz) * zs + zo;
          i32 zf = (i32)z;
          if (z < zf) zf--;
          i32 Z = zf & 255;
          z -= zf;
          f32 w = z * z * z * (z * (z * 6 - 15) + 10);

          A = m_p[X] + 0;
          AA = m_p[A] + Z;
          B = m_p[X + 1] + 0;
          BA = m_p[B] + Z;
          vv0 = Lerp(u, Grad2(m_p[AA], x, z), Grad(m_p[BA], x - 1, 0, z));
          vv2 = Lerp(u, Grad(m_p[AA + 1], x, 0, z - 1), Grad(m_p[BA + 1], x - 1, 0, z - 1));

          f32 val = Lerp(w, vv0, vv2);
          buffer[pp++] += val * scale;
        }
      }
      return;
    }

    i32 pp = 0;
    f32 scale = 1.0f / pow;
    i32 yOld = -1;
    i32 A = 0, AA = 0, AB = 0, B = 0, BA = 0, BB = 0;
    f32 vv0 = 0, vv1 = 0, vv2 = 0, vv3 = 0;

    for (i32 xx = 0; xx < xSize; xx++) {
      f32 x = (_x + xx) * xs + xo;
      i32 xf = (i32)x;
      if (x < xf) xf--;
      i32 X = xf & 255;
      x -= xf;
      f32 u = x * x * x * (x * (x * 6 - 15) + 10);

      for (i32 zz = 0; zz < zSize; zz++) {
        f32 z = (_z + zz) * zs + zo;
        i32 zf = (i32)z;
        if (z < zf) zf--;
        i32 Z = zf & 255;
        z -= zf;
        f32 w = z * z * z * (z * (z * 6 - 15) + 10);

        for (i32 yy = 0; yy < ySize; yy++) {
          f32 y = (_y + yy) * ys + yo;
          i32 yf = (i32)y;
          if (y < yf) yf--;
          i32 Y = yf & 255;
          y -= yf;
          f32 v = y * y * y * (y * (y * 6 - 15) + 10);

          if (yy == 0 || Y != yOld) {
            yOld = Y;
            A = m_p[X] + Y;
            AA = m_p[A] + Z;
            AB = m_p[A + 1] + Z;
            B = m_p[X + 1] + Y;
            BA = m_p[B] + Z;
            BB = m_p[B + 1] + Z;
            vv0 = Lerp(u, Grad(m_p[AA], x, y, z), Grad(m_p[BA], x - 1, y, z));
            vv1 = Lerp(u, Grad(m_p[AB], x, y - 1, z), Grad(m_p[BB], x - 1, y - 1, z));
            vv2 = Lerp(u, Grad(m_p[AA + 1], x, y, z - 1), Grad(m_p[BA + 1], x - 1, y, z - 1));
            vv3 = Lerp(u, Grad(m_p[AB + 1], x, y - 1, z - 1), Grad(m_p[BB + 1], x - 1, y - 1, z - 1));
          }

          f32 v0 = Lerp(v, vv0, vv1);
          f32 v1 = Lerp(v, vv2, vv3);
          f32 val = Lerp(w, v0, v1);

          buffer[pp++] += val * scale;
        }
      }
    }
  }

  inline i32 HashCode() {
    i32 x = 4711;
    for (i32 i = 0; i < 512; ++i)
      x = x * 37 + m_p[i];
    return x;
  }

  f32 scale = 0.0f;
  f32 xo = 0.0f;
  f32 yo = 0.0f;
  f32 zo = 0.0f;

private:
  i32 m_p[512] = {0};
};

#endif
