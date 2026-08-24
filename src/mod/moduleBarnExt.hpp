#ifndef __MOD_BASE_MODULEBARNEXT_HPP__
#define __MOD_BASE_MODULEBARNEXT_HPP__

#include <vector>
#include <list>
#include <Utils/Types.h>
#include <Base/Meta.hpp>

class Module;
META_DECLARE_CLASS(Module)

class Module: public Object {
public:
  Module() { m_metaClassId = MetaClassId(Module); };
  ~Module() = default;

private:
  u32 m_entryIndex = 0;
  u32 m_groupMask = 0;
};

class ModuleTag { };

template<typename T>
class ModuleTagImpl { };

enum ModuleClearMemory: u08 {
  // Do not clear memory.
  kModuleClearMemory_None = 0,
  // Clear memory with 0x00.
  kModuleClearMemory_Zero = 1,
  // Clear memory with 0xCD.
  kModuleClearMemory_Default = 2,
};

class ModuleBarnExt {
private:
  struct FunctionLink {
    const MetaMemberFunction *fn;
    std::vector<Variable> args;
  };

  struct ModuleDesc {
    TgcString name;
    MetaStrHashMap<ModuleBarnExt::FunctionLink> functions;
  };

public:
  ModuleBarnExt() = default;
  ~ModuleBarnExt() = default;

  void Initialize(Object *container);
  void Terminate();

  void CallFunction(
    cstring name);

  Object *GetObjectByTag(cstring tag);

private:
  void m_LinkContainer(Object *container);
  void m_ModuleMakeTag();
  void m_FindField();
  void m_Link();

  Object *m_container = nullptr;
  LPCMetaClass m_metaClass = nullptr;

  // NOTE: All values below is not allocated by ModuleBarnExt, the ModuleBarn
  // only holds a pointer to them. Values are from g_Game or MetaVariable
  // registered by mods.
  MetaStrHashMap<Variable> m_fields = {};
  std::list<ModuleDesc> m_modules = {};
};

#endif
