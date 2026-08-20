#ifndef __WORLD_BLOCK_HPP__
#define __WORLD_BLOCK_HPP__

#include <string>
#include <Utils/Types.h>
#include <Base/Meta.hpp>

class BlockDef {
public:
  BlockDef(cstring name): m_name(name) { }
  ~BlockDef() = default;

  inline cstring GetName() { return m_name.c_str(); }

private:
  std::string m_name = "";
};

class BlockDefBarn: public Object {
public:
  BlockDefBarn() = default;
  ~BlockDefBarn() = default;

  void Initialize();

  const BlockDef *GetBlockByName(cstring name);

private:
  MetaStrHashMap<BlockDef> m_blockDefs = {};
};

class Block {
public:
  Block(const BlockDef *def): m_def(def) { }
  ~Block() = default;

  inline const BlockDef *GetDef() { return m_def; }

private:
  const BlockDef *m_def = nullptr;
};

#endif
