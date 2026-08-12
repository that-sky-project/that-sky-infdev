#ifndef __MOD_META_HPP__
#define __MOD_META_HPP__

#include <map>
#include <unordered_map>
#include <Base/Meta.hpp>

#pragma warning(disable: 4200)

// ----------------------------------------------------------------------------
// [SECTION] FakeMetaSystem
// ----------------------------------------------------------------------------

class FakeMetaSystem {
public:
  FakeMetaSystem() = default;
  ~FakeMetaSystem() = default;

  u32 metaClassId;
  MetaSystemDataContainer *data;
};

// ----------------------------------------------------------------------------
// [SECTION] ProxyMetaSystem
// ----------------------------------------------------------------------------

class ProxyMetaSystemDataContainer: public MetaSystemDataContainer {
public:
  ProxyMetaSystemDataContainer() = default;
  ~ProxyMetaSystemDataContainer() = default;

  ProxyMetaSystemDataContainer &operator=(const MetaSystemDataContainer &);

  u32 m_count;
  u32 m_maxClasses;
};

class ProxyMetaSystem {
public:
  static ProxyMetaSystem *create(
    u32 maxClasses = 0x100000);

  ProxyMetaSystem() = default;
  ~ProxyMetaSystem() = default;
  
  bool submitChain(
    LPMetaType chain);

  bool submitChain(
    MetaMemberFunction *chain);

  bool submitChain(
    MetaMemberVariable *chain);
  
  void set(
    const MetaSystemExample *p,
    u32 count);

  inline LPCMetaClass get(u32 id) const { return id > m_data->m_maxClasses ? GetMetaClass() : m_classes[id]; }
  inline LPCMetaClass get(cstring name) const {
    if (!name) return GetMetaClass();
    const auto &classes = m_data->m_metaClasses;
    const auto &it = classes.find(name);
    return it == classes.end() ? GetMetaClass() : it->second;
  }

private:
  void m_recursiveSort(
    LPMetaClass mc,
    i32 *pTopologyCounter);

public:
  u32 m_metaClassId;
  ProxyMetaSystemDataContainer *m_data;
  const MetaClass *m_classes[];
};

namespace MetaSystemOverride {
void initialize();
}

#endif
