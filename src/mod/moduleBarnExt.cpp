#include <Base/Meta.hpp>
#include "sky/skyPrivate.hpp"
#include "mod/moduleBarnExt.hpp"

META_REGISTER_CLASS(Module)

void ModuleBarnExt::Initialize(
  Object *container
) {
  m_LinkContainer(container);
}

void ModuleBarnExt::Terminate() {
  m_modules.clear();
  m_fields.clear();
}

void ModuleBarnExt::m_LinkContainer(
  Object *container
) {
  // Save the original type and type-erased container.
  LPCMetaClass mc = GetMetaClassById(container->GetMetaClassId());
  if (!mc || !mc->AsClass() || !mc->Downcast(container))
    AssertMsg(false, "container is not a valid class.");
  m_metaClass = mc->AsClass();
  m_container = container;

  for (const auto &it: mc->m_metaDataContainer->m_variables) {
    // Skip any array.
    if (it.second->IsArray())
      continue;

    m_fields[it.first] = Variable{
      mc->ResolveMember(mc->Downcast(container), mc, it.second->Address()),
      it.second->GetType()
    };
  }
}
