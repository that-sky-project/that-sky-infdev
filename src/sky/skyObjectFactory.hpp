#ifndef __SKY_SKYOBJECTFACTORY_HPP__
#define __SKY_SKYOBJECTFACTORY_HPP__

#include <unordered_map>
#include <Base/Meta.hpp>
#include "mod/moduleBarnExt.hpp"

class ObjectFactory;
META_DECLARE_CLASS(ObjectFactory)

class ObjectFactory: public Module {
private:
  struct Fmix64Hash {
    size_t operator()(const MetaClass *mc) const noexcept {
      uint64_t k = reinterpret_cast<uint64_t>(mc);
      k ^= k >> 33;
      k *= 0xFF51AFD7ED558CCDull;
      k ^= k >> 33;
      k *= 0xC4CEB9FE1A85EC53ull;
      k ^= k >> 33;
      return static_cast<size_t>(k);
    }
  };

protected:
  struct Factory {
    Variable creator = {};
    const MetaMemberFunction *createFun = nullptr;
    const MetaMemberFunction *releaseFun = nullptr;
  };

public:
  ObjectFactory() = default;
  ~ObjectFactory() = default;

  Object *CreateObject(cstring clazz);
  void ReleaseObject(Object *object);

protected:
  Object *m_barnContainer = nullptr;
  std::unordered_map<LPCMetaClass, ObjectFactory::Factory, ObjectFactory::Fmix64Hash> m_factoryMap = {};
};

#endif
