#ifndef __WORLD_SYNTH_RANDOM_HPP__
#define __WORLD_SYNTH_RANDOM_HPP__

// ---------------------------------------------------------------------
//        A random generator class based on Mersenne-Twister.
//
// "UPDATE"
// http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/MT2002/elicense.html
// ---------------------------------------------------------------------
//
// Commercial Use of Mersenne Twister
// 2001/4/6
//
// Until 2001/4/6, MT had been distributed under GNU Public License, but
// after 2001/4/6, we decided to let MT be used for any purpose, including
// commercial use. 2002-versions mt19937ar.c, mt19937ar-cok.c are considered
// to be usable freely.
//
// ---------------------------------------------------------------------
//
// Port from Random.h of Minecraft Pocket Edition codebase leakage, by
// HTMonkeyG.
// 2026/08/25
//
// ---------------------------------------------------------------------

#include <cmath>
#include <Utils/Types.h>

class Random {
private:
  // - Period parameters
  static constexpr i32 kN = 624;
  static constexpr i32 kM = 397;
  static constexpr u32 kMatrixA = 0x9908B0DFul;   // constant vector a
  static constexpr u32 kUpperMask = 0x80000000ul; // most significant w-r bits
  static constexpr u32 kLowerMask = 0x7FFFFFFFul; // least significant r bits

public:
  ~Random() = default;
  Random() { SetSeed(5489); }
  Random(i32 seed) { SetSeed(seed); }

  void SetSeed(i32 seed) {
    m_seed = seed;
    m_mti = kN + 1;
    m_haveNextNextGaussian = false;
    m_nextNextGaussian = 0;
    m_InitGenRand(seed);
  }

  i32 GetSeed() const { return m_seed; }
  bool NextBoolean() { return (m_GenRandInt32() & 0x8000000) > 0; }
  f32 NextFloat() { return (f32)m_GenRandReal2(); }
  f64 NextDouble() { return m_GenRandReal2(); }
  i32 NextInt() { return (i32)(m_GenRandInt32() >> 1); }
  i32 NextInt(i32 n) { return m_GenRandInt32() % n; }

  /*
  i64 NextLong() {
    return (i32)(m_GenRandInt32() >> 1);
  }
  i64 NextLong(i64 n) {
    return m_GenRandInt32() % n;
  }*/

  f32 NextGaussian() {
    if (m_haveNextNextGaussian) {
      m_haveNextNextGaussian = false;
      return m_nextNextGaussian;
    } else {
      f32 v1, v2, s;
      do {
        // between -1.0 and 1.0
        v1 = 2 * NextFloat() - 1;
        v2 = 2 * NextFloat() - 1;
        s = v1 * v1 + v2 * v2;
      } while (s >= 1 || s == 0);
      f32 multiplier = std::sqrt(-2 * std::log(s) / s);
      m_nextNextGaussian = v2 * multiplier;
      m_haveNextNextGaussian = true;
      return v1 * multiplier;
    }
  }

private:
  // Initializes m_mt[kN] with a seed.
  void m_InitGenRand(u32 s) {
    m_mt[0] = s & 0xFFFFFFFFul;
    for (m_mti = 1; m_mti < kN; m_mti++) {
      m_mt[m_mti] =
        (1812433253ul * (m_mt[m_mti - 1] ^ (m_mt[m_mti - 1] >> 30)) + m_mti);
      // See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier.
      // In the previous versions, MSBs of the seed affect
      // only MSBs of the array m_mt[].
      // 2002/01/09 modified by Makoto Matsumoto
      m_mt[m_mti] &= 0xFFFFFFFFul;
      // for >32 bit machines
    }
  }

  // Initialize by an array with array-length.
  // initKey is the array for initializing keys
  // keyLength is its length
  // slight change for C++, 2004/2/26
  void m_InitByArray(u32 initKey[], i32 keyLength) {
    i32 i, j, k;
    m_InitGenRand(19650218ul);
    i = 1; j = 0;
    k = (kN > keyLength ? kN : keyLength);
    for (; k; k--) {
      m_mt[i] = (m_mt[i] ^ ((m_mt[i - 1] ^ (m_mt[i - 1] >> 30)) * 1664525ul))
        + initKey[j] + j; // non linear
      m_mt[i] &= 0xFFFFFFFFul; // for WORDSIZE > 32 machines
      i++; j++;
      if (i >= kN) { m_mt[0] = m_mt[kN - 1]; i = 1; }
      if (j >= keyLength) j = 0;
    }
    for (k = kN - 1; k; k--) {
      m_mt[i] = (m_mt[i] ^ ((m_mt[i - 1] ^ (m_mt[i - 1] >> 30)) * 1566083941ul))
        - i; // non linear
      m_mt[i] &= 0xFFFFFFFFul; // for WORDSIZE > 32 machines
      i++;
      if (i >= kN) { m_mt[0] = m_mt[kN - 1]; i = 1; }
    }

    m_mt[0] = 0x80000000ul; // MSB is 1; assuring non-zero initial array
  }

  // Generates a random number on [0,0xFFFFFFFF]-interval
  u32 m_GenRandInt32() {
    u32 y;
    static u32 mag01[2] = {0x0ul, kMatrixA};
    // mag01[x] = x * kMatrixA for x=0,1

    if (m_mti >= kN) { // generate kN words at one time
      //static Stopwatch sw;
      //sw.start();
      i32 kk;

      if (m_mti == kN + 1)   // if m_InitGenRand() has not been called,
        m_InitGenRand(5489ul); // a default initial seed is used

      for (kk = 0; kk < kN - kM; kk++) {
        y = (m_mt[kk] & kUpperMask) | (m_mt[kk + 1] & kLowerMask);
        m_mt[kk] = m_mt[kk + kM] ^ (y >> 1) ^ mag01[y & 0x1ul];
      }
      for (; kk < kN - 1; kk++) {
        y = (m_mt[kk] & kUpperMask) | (m_mt[kk + 1] & kLowerMask);
        m_mt[kk] = m_mt[kk + (kM - kN)] ^ (y >> 1) ^ mag01[y & 0x1ul];
      }
      y = (m_mt[kN - 1] & kUpperMask) | (m_mt[0] & kLowerMask);
      m_mt[kN - 1] = m_mt[kM - 1] ^ (y >> 1) ^ mag01[y & 0x1ul];

      m_mti = 0;
    }

    y = m_mt[m_mti++];

    // Tempering
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9D2C5680ul;
    y ^= (y << 15) & 0xEFC60000ul;
    y ^= (y >> 18);

    return y;
  }

  // Generates a random number on [0,0x7FFFFFFF]-interval
  i32 m_GenRandInt31() {
    return (i32)(m_GenRandInt32() >> 1);
  }

  // Generates a random number on [0,1]-real-interval
  // Divided by 2^32-1
  f64 m_GenRandReal1() {
    return m_GenRandInt32() * (1.0 / 4294967295.0);
  }

  // Generates a random number on [0,1)-real-interval
  // Divided by 2^32
  f64 m_GenRandReal2() {
    return m_GenRandInt32() * (1.0 / 4294967296.0);
  }

  // Generates a random number on (0,1)-real-interval
  // Divided by 2^32
  f64 m_GenRandReal3() {
    return (((f64)m_GenRandInt32()) + 0.5) * (1.0 / 4294967296.0);
  }

  // Generates a random number on [0,1) with 53-bit resolution
  f64 m_GenRandRes53() {
    u32 a = m_GenRandInt32() >> 5, b = m_GenRandInt32() >> 6;
    return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0);
  }
  // These real versions are due to Isaku Wada, 2002/01/09 added

  //
  // Added helper (and quicker) functions
  //
  void m_RrDiff(f32 &x) {
    u32 u = m_GenRandInt32();
    const f32 xx0 = (u & 0xFFFF) / 65536.0f; // 2 x 16 bits
    const f32 xx1 = ((u >> 16) & 0xFFFF) / 65536.0f;
    x = xx0 - xx1;
  }

  void m_RrDiff(f32 &x, f32 &y) {
    u32 u = m_GenRandInt32();
    const f32 xx0 = (u & 0xFF) / 256.0f; // 4 x 8 bits
    const f32 xx1 = ((u >> 8) & 0xFF) / 256.0f;
    const f32 yy0 = ((u >> 16) & 0xFF) / 256.0f;
    const f32 yy1 = ((u >> 24) & 0xFF) / 256.0f;
    x = xx0 - xx1;
    y = yy0 - yy1;
  }

  void m_RrDiff(f32 &x, f32 &y, f32 &z) {
    u32 u = m_GenRandInt32();
    const f32 xx0 = (u & 0x1F) / 32.0f; // 6 x 5 bits
    const f32 xx1 = ((u >> 5) & 0x1F) / 32.0f;
    const f32 yy0 = ((u >> 10) & 0x1F) / 32.0f;
    const f32 yy1 = ((u >> 15) & 0x1F) / 32.0f;
    const f32 zz0 = ((u >> 20) & 0x1F) / 32.0f;
    const f32 zz1 = ((u >> 25) & 0x1F) / 32.0f;
    x = xx0 - xx1;
    y = yy0 - yy1;
    z = zz0 - zz1;
  }

  i32 m_seed = 5489;
  // The array for the state vector.
  u32 m_mt[kN];
  // m_mti == kN + 1 means m_mt[kN] is not initialized.
  i32 m_mti = kN + 1;
  bool m_haveNextNextGaussian = false;
  f32 m_nextNextGaussian = 0;
};

#endif
