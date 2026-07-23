#ifndef __SKY_SKYVECTORMATH_HPP__
#define __SKY_SKYVECTORMATH_HPP__

#include <Utils/Types.h>
#include <intrin.h>

struct Vector4 {
  struct Elements {
    f32 x, y, z, w;
  };

  Vector4() = default;
  ~Vector4() = default;

  Vector4(f32 x, f32 y, f32 z, f32 w) {
    val[0] = x;
    val[1] = y;
    val[2] = z;
    val[3] = w;
  };

  explicit Vector4(f32 v) {
    val[0] = val[1] = val[2] = val[3] = v;
  }

  Vector4::Elements *operator->() { return reinterpret_cast<Elements *>(this); }
  const Vector4::Elements *operator->() const { return reinterpret_cast<const Elements *>(this); }

  f32 &operator[](u08 n) { return val[n]; }
  const f32 &operator[](u08 n) const { return val[n]; }

  union {
    __m128 m128;
    f32 val[4];
  };
};

struct Matrix4 {
  Matrix4() = default;
  ~Matrix4() = default;

  explicit Matrix4(f32 v) {
    vec[0] = vec[1] = vec[2] = vec[3] = Vector4(0);
    vec[0][0] = vec[1][1] = vec[2][2] = vec[3][3] = v;
  }

  Vector4 &operator[](u08 n) { return vec[n]; }
  const Vector4 &operator[](u08 n) const { return vec[n]; }

  Vector4 vec[4];
};

#endif
