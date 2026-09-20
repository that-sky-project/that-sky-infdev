#ifndef __MC_POSTYPES_HPP__
#define __MC_POSTYPES_HPP__

#include <Utils/Types.h>

struct BlockPos {
  BlockPos() = default;
  ~BlockPos() = default;

  BlockPos(
    i32 x,
    i32 y,
    i32 z
  )
    : x(x)
    , y(y)
    , z(z)
  { }

  i32 x = 0;
  i32 y = 0;
  i32 z = 0;
};

struct ChunkPos {
  using Hash = u64;

  ChunkPos() = default;
  ~ChunkPos() = default;

  ChunkPos(
    i32 x,
    i32 z
  )
    : x(x)
    , z(z)
  { }

  bool operator==(const ChunkPos &that) const { return x == that.x && z == that.z; }
  bool operator!=(const ChunkPos &that) const { return !(that == *this); }

  i32 x = 0;
  i32 z = 0;
};

struct ChunkBlockPos {
  ChunkBlockPos() = default;
  ~ChunkBlockPos() = default;

  ChunkBlockPos(
    u32 x,
    u32 y,
    u32 z
  )
    : x(x & 0x0F)
    , y(y & 0xFF)
    , z(z & 0x0F)
  { }

  BlockPos ToBlockPos(
    const ChunkPos &chunkPos
  ) {
    return BlockPos(
      (chunkPos.x << 4) + x,
      y,
      (chunkPos.z << 4) + z
    );
  }

  u32 x = 0;
  u32 y = 0;
  u32 z = 0;
};

// ----------------------------------------------------------------------------
// [SECTION] Hash functions.
// ----------------------------------------------------------------------------

namespace std {

template<>
struct hash<ChunkPos> {
  size_t operator()(const ChunkPos &k) const noexcept {
    size_t h1 = std::hash<i32>{}(k.x);
    size_t h2 = std::hash<i32>{}(k.z);
    return h1 ^ (h2 << 1);
    }
  };
}

#endif
