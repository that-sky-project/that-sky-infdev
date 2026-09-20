#ifndef __SKY_SKYVECTORMATH_HPP__
#define __SKY_SKYVECTORMATH_HPP__

#include <Utils/Types.h>
#include <intrin.h>

struct Vector2 {
private:
  struct Elements { f32 x, y; };

public:
  ~Vector2() = default;
  Vector2() = default;
  Vector2(const Vector2 &) = default;
  Vector2(Vector2 &&) = default;
  Vector2(f32 x, f32 y): val{x, y} { }
  explicit Vector2(f32 v): val{v, v} { }

  Vector2 &operator=(const Vector2 &) = default;

  // Component accessor.
  Vector2::Elements *operator->() { return reinterpret_cast<Elements *>(this); }
  const Vector2::Elements *operator->() const { return reinterpret_cast<const Elements *>(this); }

  // Underscore accessor.
  f32 &operator[](u08 n) { return val[n]; }
  const f32 &operator[](u08 n) const { return val[n]; }

  bool operator==(const Vector2 &rhs) const { return val[0] == rhs.val[0] && val[1] == rhs.val[1]; }

  operator __m64() { return m64; }

  union {
    f32 val[2] = {0, 0};
    __m64 m64;
  };
};

struct Vector3 {
private:
  struct Elements { f32 x, y, z; };

public:
  // Static arithmetic functions.
  static inline Vector3 minval(const Vector3 &a, const Vector3 &b) { return Vector3(_mm_min_ps(a.m128, b.m128)); }
  static inline Vector3 maxval(const Vector3 &a, const Vector3 &b) { return Vector3(_mm_max_ps(a.m128, b.m128)); }
  static inline f32 dot(const Vector3 &a, const Vector3 &b) { return _mm_cvtss_f32(_mm_dp_ps(a.m128, b.m128, 0x71)); }

  ~Vector3() = default;
  Vector3() = default;
  Vector3(const Vector3 &) = default;
  Vector3(Vector3 &&) = default;
  Vector3(f32 x, f32 y, f32 z): val{x, y, z} { }
  explicit Vector3(f32 v): val{v, v, v} { }
  explicit Vector3(__m128 v): m128(v) { }

  Vector3 &operator=(const Vector3 &) = default;

  // Component accessor.
  Vector3::Elements *operator->() { return reinterpret_cast<Elements *>(this); }
  const Vector3::Elements *operator->() const { return reinterpret_cast<const Elements *>(this); }

  // Underscore accessor.
  f32 &operator[](u08 n) { return val[n]; }
  const f32 &operator[](u08 n) const { return val[n]; }

  inline bool operator==(const Vector3 &rhs) const { __m128 cmp = _mm_cmpeq_ps(m128, rhs.m128); return _mm_movemask_ps(cmp) == 0x7; }
  inline bool operator!=(const Vector3 &rhs) const { return !(rhs == *this); }
  inline Vector3 &operator+=(const Vector3 &rhs) { m128 = _mm_add_ps(m128, rhs.m128); return *this; }
  inline Vector3 &operator-=(const Vector3 &rhs) { m128 = _mm_sub_ps(m128, rhs.m128); return *this; }
  inline Vector3 &operator*=(const Vector3 &rhs) { m128 = _mm_mul_ps(m128, rhs.m128); return *this; }
  inline Vector3 &operator/=(const Vector3 &rhs) { m128 = _mm_div_ps(m128, rhs.m128); return *this; }
  inline Vector3 operator-() const { return Vector3(_mm_xor_ps(m128, _mm_set1_ps(-0.0f))); }

  inline friend Vector3 operator+(Vector3 lhs, const Vector3 &rhs) { return lhs += rhs; }
  inline friend Vector3 operator-(Vector3 lhs, const Vector3 &rhs) { return lhs -= rhs; }
  inline friend Vector3 operator*(Vector3 lhs, const Vector3 &rhs) { return lhs *= rhs; }
  inline friend Vector3 operator/(Vector3 lhs, const Vector3 &rhs) { return lhs /= rhs; }

  inline friend Vector3 operator+(Vector3 lhs, f32 rhs) { return lhs += Vector3(rhs); }
  inline friend Vector3 operator-(Vector3 lhs, f32 rhs) { return lhs -= Vector3(rhs); }
  inline friend Vector3 operator*(Vector3 lhs, f32 rhs) { return lhs *= Vector3(rhs); }
  inline friend Vector3 operator/(Vector3 lhs, f32 rhs) { return lhs /= Vector3(rhs); }

  // Arithmetic functions.
  inline f32 length() const { return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(m128, m128, 0x71))); }

  operator __m128() { return m128; }

  union {
    f32 val[3] = {0, 0, 0};
    __m128 m128;
  };
};

struct Vector4 {
private:
  struct Elements { f32 x, y, z, w; };

public:
  // Static arithmetic functions.
  static inline Vector4 minval(const Vector4 &a, const Vector4 &b) { return Vector4(_mm_min_ps(a.m128, b.m128)); }
  static inline Vector4 maxval(const Vector4 &a, const Vector4 &b) { return Vector4(_mm_max_ps(a.m128, b.m128)); }
  static inline f32 dot(const Vector4 &a, const Vector4 &b) { return _mm_cvtss_f32(_mm_dp_ps(a.m128, b.m128, 0xFF)); }

  ~Vector4() = default;
  Vector4() = default;
  Vector4(const Vector4 &) = default;
  Vector4(Vector4 &&) = default;
  Vector4(f32 x, f32 y, f32 z, f32 w): val{x, y, z, w} { }
  explicit Vector4(f32 v): val{v, v, v, v} { }
  explicit Vector4(__m128 v): m128(v) { }

  inline Vector4 &operator=(const Vector4 &) = default;

  // Component accessor.
  inline Vector4::Elements *operator->() { return reinterpret_cast<Elements *>(this); }
  inline const Vector4::Elements *operator->() const { return reinterpret_cast<const Elements *>(this); }

  // Underscore accessor.
  inline f32 &operator[](u08 n) { return val[n]; }
  inline const f32 &operator[](u08 n) const { return val[n]; }

  // Arithmetic operators.
  inline bool operator==(const Vector4 &rhs) const { __m128 cmp = _mm_cmpeq_ps(m128, rhs.m128); return _mm_movemask_ps(cmp) == 0xF; }
  inline bool operator!=(const Vector4 &rhs) const { return !(rhs == *this); }
  inline Vector4 &operator+=(const Vector4 &rhs) { m128 = _mm_add_ps(m128, rhs.m128); return *this; }
  inline Vector4 &operator-=(const Vector4 &rhs) { m128 = _mm_sub_ps(m128, rhs.m128); return *this; }
  inline Vector4 &operator*=(const Vector4 &rhs) { m128 = _mm_mul_ps(m128, rhs.m128); return *this; }
  inline Vector4 &operator/=(const Vector4 &rhs) { m128 = _mm_div_ps(m128, rhs.m128); return *this; }
  inline Vector4 operator-() const { return Vector4(_mm_xor_ps(m128, _mm_set1_ps(-0.0f))); }

  inline friend Vector4 operator+(Vector4 lhs, const Vector4 &rhs) { return lhs += rhs; }
  inline friend Vector4 operator-(Vector4 lhs, const Vector4 &rhs) { return lhs -= rhs; }
  inline friend Vector4 operator*(Vector4 lhs, const Vector4 &rhs) { return lhs *= rhs; }
  inline friend Vector4 operator/(Vector4 lhs, const Vector4 &rhs) { return lhs /= rhs; }

  inline friend Vector4 operator+(Vector4 lhs, f32 rhs) { return lhs += Vector4(rhs); }
  inline friend Vector4 operator-(Vector4 lhs, f32 rhs) { return lhs -= Vector4(rhs); }
  inline friend Vector4 operator*(Vector4 lhs, f32 rhs) { return lhs *= Vector4(rhs); }
  inline friend Vector4 operator/(Vector4 lhs, f32 rhs) { return lhs /= Vector4(rhs); }

  // Arithmetic functions.
  inline f32 length() const { return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(m128, m128, 0xFF))); }
  /*inline f32 length() const {
    __m128 sum = _mm_mul_ps(m128, m128);
    sum = _mm_add_ps(sum, _mm_shuffle_ps(sum, sum, _MM_SHUFFLE(2, 3, 0, 1)));
    sum = _mm_add_ps(sum, _mm_shuffle_ps(sum, sum, _MM_SHUFFLE(1, 0, 3, 2)));
    return _mm_cvtss_f32(_mm_sqrt_ss(sum));
  }*/

  // Type casts.
  inline operator __m128() { return m128; }

  union {
    f32 val[4] = {0, 0, 0, 0};
    __m128 m128;
  };
};

struct Matrix3 {
  Matrix3() = default;
  ~Matrix3() = default;

  explicit Matrix3(f32 v) {
    vec[0] = vec[1] = vec[2] = Vector3(0);
    vec[0][0] = vec[1][1] = vec[2][2] = v;
  }

  // Underscore accessor.
  Vector3 &operator[](u08 n) { return vec[n]; }
  const Vector3 &operator[](u08 n) const { return vec[n]; }

  Vector3 vec[3];
};

struct Matrix4 {
  Matrix4() = default;
  ~Matrix4() = default;

  explicit Matrix4(f32 v) {
    vec[0] = vec[1] = vec[2] = vec[3] = Vector4(0);
    vec[0][0] = vec[1][1] = vec[2][2] = vec[3][3] = v;
  }

  // Underscore accessor.
  Vector4 &operator[](u08 n) { return vec[n]; }
  const Vector4 &operator[](u08 n) const { return vec[n]; }

  Vector4 vec[4];
};

#endif
