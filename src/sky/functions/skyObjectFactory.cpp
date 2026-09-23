#include "sky/skyPrivate.hpp"
#include "sky/skyObjectFactory.hpp"

META_REGISTER_CLASS(ObjectFactory, MetaClassImpl<Module>::Must_call_META_REGISTER_CLASS)

// ----------------------------------------------------------------------------
// [SECTION] ObjectFactory
// ----------------------------------------------------------------------------

Object *ObjectFactory::CreateObject(
  cstring clazz
) {
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
