#ifndef __WORLD_BLOCK_HPP__
#define __WORLD_BLOCK_HPP__

#include <string>
#include <Utils/Types.h>

class Block {
public:
  Block(cstring name): m_name(name) { }
  ~Block() = default;

private:
  std::string m_name = "";
};

const Block kBlockDefault_Air = Block("");

#endif
