#ifndef __WORLD_SYNTH_VALUENOISE_HPP__
#define __WORLD_SYNTH_VALUENOISE_HPP__

// Value-noise volume generator ported from
// `voxels/test/opengl3_value_noise/ValueNoise.h`.
//
// The Voxels grid uses a Z-up coordinate system. GetSurface() is called for
// every 16x16x16 block and must return a signed distance for each voxel:
//   positive = air, negative = solid.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <Voxels/Voxels.h>
#include <Utils/Types.h>

inline f32 ValueNoiseLerp(
  f32 a,
  f32 b,
  f32 t
) {
  return a + (b - a) * t;
}

// Quintic fade curve - smoother than the classic cubic smoothstep.
inline f32 ValueNoiseFade(
  f32 t
) {
  return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

class ValueNoise {
public:
  explicit ValueNoise(u32 seed = 1337u): m_seed(seed) { }
  ~ValueNoise() = default;

  inline void SetSeed(u32 seed) { m_seed = seed; }

  // Pseudo-random lattice value in [-1, 1].
  inline f32 Lattice(
    i32 x,
    i32 y,
    i32 z
  ) const {
    u32 h = m_seed;
    h += (u32)x * 0x8da6b343u;
    h = (h << 13) | (h >> 19);
    h += (u32)y * 0xd8163841u;
    h = (h << 17) | (h >> 15);
    h += (u32)z * 0xcb1ab31fu;
    h *= 0x27d4eb2fu;
    h ^= h >> 15;
    h *= 0x85ebca6bu;
    h ^= h >> 13;
    return (f32)(h & 0x00ffffffu) * (2.0f / 16777215.0f) - 1.0f;
  }

  // Trilinearly interpolated value noise in [-1, 1].
  inline f32 Noise(
    f32 x,
    f32 y,
    f32 z
  ) const {
    const f32 fx = floorf(x);
    const f32 fy = floorf(y);
    const f32 fz = floorf(z);

    const i32 ix = (i32)fx;
    const i32 iy = (i32)fy;
    const i32 iz = (i32)fz;

    const f32 ux = ValueNoiseFade(x - fx);
    const f32 uy = ValueNoiseFade(y - fy);
    const f32 uz = ValueNoiseFade(z - fz);

    const f32 c000 = Lattice(ix,     iy,     iz);
    const f32 c100 = Lattice(ix + 1, iy,     iz);
    const f32 c010 = Lattice(ix,     iy + 1, iz);
    const f32 c110 = Lattice(ix + 1, iy + 1, iz);
    const f32 c001 = Lattice(ix,     iy,     iz + 1);
    const f32 c101 = Lattice(ix + 1, iy,     iz + 1);
    const f32 c011 = Lattice(ix,     iy + 1, iz + 1);
    const f32 c111 = Lattice(ix + 1, iy + 1, iz + 1);

    const f32 c00 = ValueNoiseLerp(c000, c100, ux);
    const f32 c10 = ValueNoiseLerp(c010, c110, ux);
    const f32 c01 = ValueNoiseLerp(c001, c101, ux);
    const f32 c11 = ValueNoiseLerp(c011, c111, ux);

    const f32 c0 = ValueNoiseLerp(c00, c10, uy);
    const f32 c1 = ValueNoiseLerp(c01, c11, uy);

    return ValueNoiseLerp(c0, c1, uz);
  }

  // Fractal Brownian motion built from 2D value noise.
  inline f32 Fbm(
    f32 x,
    f32 y,
    i32 octaves = 5
  ) const {
    f32 total = 0.0f;
    f32 amplitude = 1.0f;
    f32 frequency = 1.0f;
    f32 normalization = 0.0f;

    for (i32 i = 0; i < octaves; ++i) {
      // Offsetting Z per octave decorrelates the octaves.
      total += amplitude * Noise(
        x * frequency,
        y * frequency,
        0.37f + (f32)i * 19.19f);
      normalization += amplitude;
      amplitude *= 0.5f;
      frequency *= 2.0f;
    }

    return total / normalization;
  }

  // Full 3D fBm - used to carve large caves, arches and overhangs.
  inline f32 Fbm3(
    f32 x,
    f32 y,
    f32 z,
    i32 octaves = 4
  ) const {
    f32 total = 0.0f;
    f32 amplitude = 1.0f;
    f32 frequency = 1.0f;
    f32 normalization = 0.0f;

    for (i32 i = 0; i < octaves; ++i) {
      total += amplitude * Noise(
        x * frequency,
        y * frequency,
        z * frequency);
      normalization += amplitude;
      amplitude *= 0.5f;
      frequency *= 2.0f;
    }

    return total / normalization;
  }

private:
  u32 m_seed = 1337u;
};

struct ValueNoiseVector {
  f32 x = 0.0f;
  f32 y = 0.0f;
  f32 z = 0.0f;
};

class ValueNoiseSurface: public Voxels::VoxelSurface {
public:
  explicit ValueNoiseSurface(u32 seed): m_noise(seed) { }
  ~ValueNoiseSurface() override = default;

  // Large-scale landform. Grid coordinate convention: X/Y horizontal, Z up.
  inline f32 BaseHeight(
    f32 x,
    f32 y
  ) const {
    const f32 continents = m_noise.Fbm(x * 0.0065f, y * 0.0065f, 3);
    const f32 hills = m_noise.Fbm(x * 0.018f + 13.0f, y * 0.018f - 9.0f, 2);
    return 60.0f + 30.0f * continents + 10.0f * hills;
  }

  // Signed distance of the final 3D surface. The 3D fBm term is what makes
  // this a volume instead of a heightmap: it produces caves, arches and
  // overhangs that a 2D heightfield cannot represent.
  inline f32 Distance(
    f32 x,
    f32 y,
    f32 z
  ) const {
    return VolumeDistance(x, y, z, BaseHeight(x, y));
  }

  // Gradient of the distance field. It points towards increasing distance,
  // i.e. towards the air. The normalized gradient is the smooth analytic
  // surface normal.
  inline ValueNoiseVector Gradient(
    f32 x,
    f32 y,
    f32 z
  ) const {
    const f32 epsilon = 0.25f;
    const f32 inverse = 1.0f / (2.0f * epsilon);
    return ValueNoiseVector{
      (Distance(x + epsilon, y, z) - Distance(x - epsilon, y, z)) * inverse,
      (Distance(x, y + epsilon, z) - Distance(x, y - epsilon, z)) * inverse,
      (Distance(x, y, z + epsilon) - Distance(x, y, z - epsilon)) * inverse
    };
  }

  inline ValueNoiseVector Normal(
    f32 x,
    f32 y,
    f32 z
  ) const {
    const ValueNoiseVector gradient = Gradient(x, y, z);
    const f32 lengthSquared = gradient.x * gradient.x
      + gradient.y * gradient.y
      + gradient.z * gradient.z;

    if (lengthSquared < 1e-12f)
      return ValueNoiseVector{0.0f, 1.0f, 0.0f};

    const f32 inverse = 1.0f / sqrtf(lengthSquared);
    return ValueNoiseVector{
      gradient.x * inverse,
      gradient.y * inverse,
      gradient.z * inverse
    };
  }

  void GetSurface(
    float xStart,
    float xEnd,
    float xStep,
    float yStart,
    float yEnd,
    float yStep,
    float zStart,
    float zEnd,
    float zStep,
    float *output,
    unsigned char *materialid,
    unsigned char *blend
  ) override {
    const i32 nx = std::max(1, (i32)lroundf((xEnd - xStart) / xStep));
    const i32 ny = std::max(1, (i32)lroundf((yEnd - yStart) / yStep));
    const i32 nz = std::max(1, (i32)lroundf((zEnd - zStart) / zStep));

    // The broad landform only depends on X/Y, so evaluate it once per
    // column. The 3D component has to be evaluated for every voxel.
    std::vector<f32> baseHeights((size_t)nx * (size_t)ny);
    for (i32 iy = 0; iy < ny; ++iy) {
      const f32 y = yStart + (f32)iy * yStep;
      for (i32 ix = 0; ix < nx; ++ix) {
        const f32 x = xStart + (f32)ix * xStep;
        baseHeights[(size_t)iy * (size_t)nx + (size_t)ix] = BaseHeight(x, y);
      }
    }

    size_t id = 0;
    for (i32 iz = 0; iz < nz; ++iz) {
      const f32 z = zStart + (f32)iz * zStep;
      for (i32 iy = 0; iy < ny; ++iy) {
        const f32 y = yStart + (f32)iy * yStep;
        for (i32 ix = 0; ix < nx; ++ix) {
          const f32 x = xStart + (f32)ix * xStep;
          const f32 base = baseHeights[(size_t)iy * (size_t)nx + (size_t)ix];
          output[id] = VolumeDistance(x, y, z, base);

          if (materialid)
            materialid[id] = 0;
          if (blend)
            blend[id] = 0;

          ++id;
        }
      }
    }
  }

private:
  inline f32 VolumeDistance(
    f32 x,
    f32 y,
    f32 z,
    f32 baseHeight
  ) const {
    // Keep the octave count and frequencies low enough for the padded
    // 48^3 grid: high-frequency terms only add voxel-scale noise and visible
    // faceting.
    const f32 large = m_noise.Fbm3(
      x * 0.014f + 3.1f,
      y * 0.014f - 5.7f,
      z * 0.020f + 1.3f, 3);
    const f32 detail = m_noise.Fbm3(
      x * 0.050f - 17.0f,
      y * 0.050f + 23.0f,
      z * 0.060f + 7.0f, 2);

    return (z - baseHeight) + 48.0f * large + 2.0f * detail;
  }

  ValueNoise m_noise;
};

#endif
