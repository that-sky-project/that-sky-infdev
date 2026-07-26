#include "mod/meta.hpp"

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
  pData->m_count = 0;

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
  *m_data = *p->m_data;

  // Actually we don't know how many classes we need to copy at compile time.
  memcpy(m_classes, (const void *)p->m_classes, count * sizeof(const MetaClass *));

  m_data->m_count = count;
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
    // Thus, an external module only needs to declare an "empty" MetaType with the
    // same name to use the MetaType of its dependency.
    cstring name = p->GetName();
    const auto &itType = m_data->m_metaTypes.find(name);
    if (itType != m_data->m_metaTypes.end()) {
      p->SetActive(itType->second);
      continue;
    }

    // Copy the name string for easy searching.
    size_t l = strlen(name) + 1;
    char *s = new char[l];
    strncpy(s, p->GetName(), l);

    // Copy the MetaType (or MetaClass).
    auto mt = p->Copy();
    mt->SetName(s);
    mt->SetActive(mt);
    p->SetActive(mt);

    m_data->m_metaTypes[mt->GetName()] = mt;
  
    if (!p->AsClass())
      continue;

    auto mc = (MetaClass *)mt;
    mc->m_globalId = m_data->m_count++;
    if (mc->m_metaDataContainer)
      delete mc->m_metaDataContainer;
    mc->m_metaDataContainer = new MetaDataContainer();

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

    // Copy the name string for easy searching.
    size_t l = strlen(name) + 1;
    char *s = new char[l];
    strncpy(s, p->GetName(), l);

    auto &store = p->GetClass()->m_metaDataContainer->m_functions;
    if (store.find(name) != store.end())
      continue;

    store.emplace(name, new MetaMemberFunction(*p));
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

    // Copy the name string for easy searching.
    size_t l = strlen(name) + 1;
    char *s = new char[l];
    strncpy(s, p->GetName(), l);

    auto &store = p->GetClass()->m_metaDataContainer->m_variables;
    if (store.find(name) != store.end())
      continue;

    store.emplace(name, new MetaMemberVariable(*p));
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
