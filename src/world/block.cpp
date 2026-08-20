#include "world/block.hpp"

void BlockDefBarn::Initialize() {
  m_blockDefs["minecraft:air"] = BlockDef("minecraft:air");
  m_blockDefs["minecraft:grass"] = BlockDef("minecraft:grass");
  m_blockDefs["minecraft:stone"] = BlockDef("minecraft:stone");
}

const BlockDef *BlockDefBarn::GetBlockByName(
  cstring name
) {
  const auto &it = m_blockDefs.find(name);
  if (it == m_blockDefs.end())
    return nullptr;
  return &it->second;
}
