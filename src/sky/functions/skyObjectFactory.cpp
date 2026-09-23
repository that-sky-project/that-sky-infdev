#include "sky/skyPrivate.hpp"
#include "sky/skyObjectFactory.hpp"

META_REGISTER_CLASS(ObjectFactory, MetaClassImpl<Module>::Must_call_META_REGISTER_CLASS)

// ----------------------------------------------------------------------------
// [SECTION] ObjectFactory
// ----------------------------------------------------------------------------

Object *ObjectFactory::CreateObject(
  cstring clazz
) {
  /*LPCMetaClass metaClass = GetMetaClassByName(clazz);
  AssertMsg(metaClass, "Couldn't find MetaClass for type %s", clazz);

  const auto *factory = ObjectFactory::m_GetFactoryForTypeImpl(metaClass, metaClass);
  AssertMsg(factory->createFun, "No Create function registered for type %s", metaClass->GetName());

  const auto *createFun = factory->createFun;
  i32 argc = createFun->GetSignature().argCount;
  Object *value = nullptr;
  if (argc == 1) {
    Variable creator = factory->creator
      , retval
      , argv[1];
    argv[0] = { &metaClass, GetMetaClassByType<MetaClass *>() };
    retval = { &value, GetMetaClassByType<Object *>() };
    //ModuleBarn::Call::operator()(createFun, &creator, &retval, argv, 1);
  } else {
    if (argc) {
      AssertMsg(false, "Create Function must take 0 or 1 argument of type const MetaClass*");
    }
    Variable creator = factory->creator
      , retval = { &value, GetMetaClassByType<Object *>() };
    //ModuleBarn::Call::operator()(createFun, &creator, &retval, nullptr, 0);
  }

  Object *newObject = nullptr;
  GetMetaClassByType<Object *>()->DynamicCast(newObject, &value, GetMetaClassByType<Object *>());
  Assert(newObject);

  return newObject;*/

  using Call = decltype(&ObjectFactory::CreateObject);
  static Call p = nullptr;
  if (p) return (this->*p)(clazz);

  auto &functions = GetMetaClassByType<ObjectFactory *>()->m_metaDataContainer->m_functions;
  auto mmf = functions.find("CreateObject");
  Assert(mmf != functions.end());

  p = reinterpret_cast<Call>(mmf->second->Function());

  return (this->*p)(clazz);
}

void ObjectFactory::ReleaseObject(
  Object *object
) {
  using Call = decltype(&ObjectFactory::ReleaseObject);
  static Call p = nullptr;
  if (p) return (this->*p)(object);

  auto &functions = GetMetaClassByType<ObjectFactory *>()->m_metaDataContainer->m_functions;
  auto mmf = functions.find("ReleaseObject");
  Assert(mmf != functions.end());

  p = reinterpret_cast<Call>(mmf->second->Function());

  return (this->*p)(object);
}

const ObjectFactory::Factory *ObjectFactory::m_GetFactoryForTypeImpl(
  LPCMetaClass query,
  LPCMetaClass initial
) const {
  // NOTE: The attempt to fully match the disassembly's execution order failed,
  // so an implementation with equivalent logic is used instead.
  while (1) {
    const auto &it = m_factoryMap.find(query);

    if (it != m_factoryMap.end())
      return &it->second;

    // Fall back to the super class.
    LPCMetaClass super = query->GetParent();
    if (!super) {
      // The whole chain has been walked without a match.
      AssertMsg(it != m_factoryMap.end(), "Unable to find factory for type %s.", initial->GetName());
      return nullptr;
    }

    query = super;
  }
}
