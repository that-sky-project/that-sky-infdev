#ifndef __SKY_SKYOBJECTFACTORY_HPP__
#define __SKY_SKYOBJECTFACTORY_HPP__

#include <unordered_map>
#include <Base/Meta.hpp>
#include "mod/moduleBarnExt.hpp"

class ObjectFactory;
META_DECLARE_CLASS(ObjectFactory)

class ObjectFactory: public Module {
private:
  struct Factory {
    Variable creator = {};
    const MetaMemberFunction *createFun = nullptr;
    const MetaMemberFunction *releaseFun = nullptr;
  };

public:
  ObjectFactory() = default;
  ~ObjectFactory() = default;

  //inline void SetBarnContainer(Object *container) { m_barnContainer = container; }

  Object *CreateObject(cstring clazz);
  void ReleaseObject(Object *object);

private:
  const ObjectFactory::Factory *m_GetFactoryForTypeImpl(
    LPCMetaClass query,
    LPCMetaClass initial
  ) const;

protected:
  Object *m_barnContainer = nullptr;
  std::unordered_map<LPCMetaClass, ObjectFactory::Factory> m_factoryMap = {};
};

#endif
