#include <includes/htmodloader.h>
#include <Base/Meta.hpp>
#include "sky/skyPrivate.hpp"
#include "mod/objectSource.hpp"

bool ObjectSource::TryAddFactory(
  const Variable &creator
) {
  bool result = false;
  LPCMetaClass creatorMetaClass = creator.type->AsClass();
  Assert(creatorMetaClass);

  const auto &functions = creatorMetaClass->m_metaDataContainer->m_functions;
  for (const auto &it: functions) {
    // Iterate through all member functions and find members with create or
    // release metadata, then record the type it creates (or releases).
    bool isCreate = true;
    cstring type = it.second->GetMetaData("ObjectFactory_CreatesType");
    if (!type) {
      type = it.second->GetMetaData("ObjectFactory_ReleasesType");
      if (!type) continue;
      isCreate = false;
    }

    LPCMetaClass mc = GetMetaClassByName(type, false);
    Assert(mc);

    // Record the create (release) function.
    Factory &fact = m_factoryMap[mc];
    fact.creator = creator;
    if (isCreate) {
      AssertMsg(!fact.createFun, "CreateFun already registered for type %s", mc->GetName());
      fact.createFun = it.second;
    } else {
      AssertMsg(!fact.releaseFun, "ReleaseFun already registered for type %s", mc->GetName());
      fact.releaseFun = it.second;
    }

    AssertMsg(fact.createFun != fact.releaseFun, "CreateFun is the same as ReleaseFun for type %s", mc->GetName());
    result = true;

    HTTellText("§aAdd ObjectFactory for type %s", mc->GetName());
  }

  return result;
}
