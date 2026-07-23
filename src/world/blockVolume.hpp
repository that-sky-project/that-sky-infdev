#ifndef __WORLD_BLOCKVOLUME_HPP__
#define __WORLD_BLOCKVOLUME_HPP__

#include <vector>
#include <Utils/Types.h>
#include "world/block.hpp"

class BlockVolume {
public:
  BlockVolume(
    u32 xL,
    u32 yL,
    u32 zL,
    Block placeholder = 
  ) {

  }
  ~BlockVolume() = default;

private:
  std::vector<Block> m_blocks = {};
};

#endif
