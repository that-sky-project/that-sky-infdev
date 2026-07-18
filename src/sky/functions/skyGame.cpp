#include <Base/Meta.hpp>
#include "mod/base/override.hpp"
#include "sky/skyGame.hpp"

// Predefined metaclass, only provide the name.
META_REGISTER_CLASS(Game)

void *Game::resolveMember(
  cstring name
) {
  static MetaStrHashMap<ptrdiff_t> s_memberOffs = {};

  const auto &it = s_memberOffs.find(name);
  if (it != s_memberOffs.end())
    return reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(this) + it->second);

  LPCMetaClass mc = GetMetaClassByType<Game *>()->AsClass();
  if (!mc)
    return nullptr;

  const auto &itmv = mc->m_metaDataContainer->m_variables.find(name);
  if (itmv == mc->m_metaDataContainer->m_variables.end())
    return nullptr;
  const MetaMemberVariable *mmv = itmv->second;

  void **p = &(this->*reinterpret_cast<void *Game::*>(mmv->Address()));
  uintptr_t result = reinterpret_cast<uintptr_t>(p) - reinterpret_cast<uintptr_t>(this);
  s_memberOffs[name] = result;

  return reinterpret_cast<void *>(p);
}
