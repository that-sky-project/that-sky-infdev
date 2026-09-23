#include "sky/skyPrivate.hpp"
#include "mod/base/proxyMetaSystem.hpp"
#include "utils/stringUtils.hpp"

ProxyMetaSystemDataContainer &ProxyMetaSystemDataContainer::operator=(
  const MetaSystemDataContainer &rhs
) {
  m_metaTypes = rhs.m_metaTypes;
  m_metaConstants = rhs.m_metaConstants;
  m_metaVariables = rhs.m_metaVariables;
  m_metaFunctions = rhs.m_metaFunctions;
  m_metaClasses = rhs.m_metaClasses;
  unk_6 = rhs.unk_6;
  unk_7 = rhs.unk_7;
  unk_8 = rhs.unk_8;
  return *this;
}

ProxyMetaSystem *ProxyMetaSystem::create(
  u32 maxClasses
) {
  ProxyMetaSystemDataContainer *pData = new ProxyMetaSystemDataContainer();
  pData->m_maxClasses = maxClasses;

  ProxyMetaSystem *pMetaSystem = (ProxyMetaSystem *)operator new(
    sizeof(ProxyMetaSystem) + sizeof(const MetaClass *) * maxClasses);
  new (pMetaSystem) ProxyMetaSystem();
  pMetaSystem->m_data = pData;

  for (u32 i = 0; i < maxClasses; i++)
    pMetaSystem->m_classes[i] = GetMetaClass();

  return pMetaSystem;
}

void ProxyMetaSystem::set(
  const MetaSystemExample *p,
  u32 count
) {
  Assert(count);

  *m_data = *p->m_data;

  // Actually we don't know how many classes we need to copy at compile time.
  memcpy(m_classes, (const void *)p->m_classes, count * sizeof(const MetaClass *));

  m_data->m_count = m_data->m_index = count;
}

bool ProxyMetaSystem::submitChain(
  LPMetaType chain
) {
  if (!m_data)
    return false;

  for (auto p = chain; p; p = p->GetPrev()) {
    // To make it easier for a mod to register MetaType for its dependencies,
    // we stipulate: if a MetaType with the same name already exists in the registry,
    // only set the input MetaType's m_self. Since almost all MetaType operations
    // must be performed via Must_call_META_REGISTER_CLASS or GetMetaTypeByType
    // (i.e., through m_self), all MetaType operations can be forwarded to the
    // existing MetaType.
    // 
    // Furthermore, because a dependent module is always loaded after its dependency,
    // and a child class is always loaded after its parent class, this initialization
    // process is guaranteed to be stable.
    // 
    // Thus, the ProxyMetaSystem promises an external module only needs to declare
    // an "empty" MetaType with the same name to use the MetaType of its dependency.
    cstring name = p->GetName();
    const auto &itType = m_data->m_metaTypes.find(name);
    if (itType != m_data->m_metaTypes.end()) {
      p->SetActive(itType->second);
      continue;
    }

    // Copy the name string for easy searching.
    cstring s = StringUtils::StrDup(p->GetName());

    // Copy the MetaType (or MetaClass).
    auto mt = p->Copy();
    mt->SetName(s);
    mt->SetActive(mt);
    p->SetActive(mt);

    m_data->m_metaTypes[mt->GetName()] = mt;

    if (!p->AsClass())
      continue;

    auto mc = (MetaClass *)mt;

    if (mc->m_metaDataContainer)
      delete mc->m_metaDataContainer;
    mc->m_metaDataContainer = new MetaDataContainer();

    // The metaclass objects that can be loaded into a level in the game only
    // exist within the space provided by the game itself, and do not include
    // extended space. If an extended metaclass declares that it needs to be
    // loaded by a level, we iterate through all metaclass positions in the game
    // itself, find an unused empty slot, and replace it.
    if (mc->GetMetaData("Infdev_LevelExport")) {
      u32 i = 0;
      for (; i < m_data->m_count; i++) {
        if (m_classes[i] != GetMetaClass())
          continue;

        mc->m_globalId = i;
        break;
      }

      AssertMsg(i != m_data->m_count, "MetaClass %s cannot be initialized as level export", mc->GetName());
    } else {
      mc->m_globalId = m_data->m_index++;
    }

    m_classes[mc->m_globalId] = mc;
    m_data->m_metaClasses[mt->GetName()] = mc;
  }

  i32 topoId = -2147483648;
  for (auto p = chain; p; p = p->GetPrev()) {
    if (!p->GetActive()->AsClass())
      continue;
    m_recursiveSort((LPMetaClass)p->GetActive(), &topoId);
  }

  return true;
}

bool ProxyMetaSystem::submitChain(
  MetaMemberFunction *chain
) {
  if (!m_data)
    return false;

  for (auto p = chain; p; p = p->GetPrev()) {
    cstring name = p->GetName();
    p->Initialize();

    // Copy the name string.
    cstring s = StringUtils::StrDup(p->GetName());

    auto &store = p->GetClass()->m_metaDataContainer->m_functions;
    if (store.find(name) != store.end())
      continue;

    auto *mmf = new MetaMemberFunction(*p);
    mmf->SetName(s);
    store.emplace(name, mmf);
  }

  return true;
}

bool ProxyMetaSystem::submitChain(
  MetaMemberVariable *chain
) {
  if (!m_data)
    return false;

  for (auto p = chain; p; p = p->GetPrev()) {
    cstring name = p->GetName();

    // Copy the name string.
    cstring s = StringUtils::StrDup(p->GetName());

    auto &store = p->GetClass()->m_metaDataContainer->m_variables;
    if (store.find(name) != store.end())
      continue;

    auto *mmv = new MetaMemberVariable(*p);
    mmv->SetName(s);
    store.emplace(name, mmv);
  }

  return true;
}

void ProxyMetaSystem::m_recursiveSort(
  LPMetaClass mc,
  i32 *topoId
)  {
  if (mc->m_topoOrder != -1)
    return;

  if (mc->m_parent)
    m_recursiveSort(mc->m_parent(), topoId);

  mc->m_baseTopoIdList.clear();

  if (mc->m_parent) {
    LPMetaClass superClass = mc->m_parent();

    if (superClass->m_topoOrder == -1) {
      i32 id = *topoId;
      superClass->m_topoOrder = id;
      superClass->m_baseTopoIdList.push_back(id);
      *topoId++;
    }

    if (superClass != mc) {
      mc->m_baseTopoIdList.insert(
        mc->m_baseTopoIdList.end(),
        superClass->m_baseTopoIdList.begin(),
        superClass->m_baseTopoIdList.end());
    }
  }
}
