#ifndef __MOD_BASE_MODULEBARNEXT_HPP__
#define __MOD_BASE_MODULEBARNEXT_HPP__

#include <vector>
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

class ModuleTagList {
public:
  static ModuleTag *&m_List() {
    ModuleTag *p = nullptr;
    return p;
  }

  template<typename T>
  ModuleTagList(
    ModuleTag &tag,
    cstring name,
    ModuleClearMemory clear,
    bool isModule
  ) {

  }
};

class ModuleBarnExt {
private:
  struct ModuleDesc {
  };

public:
  ModuleBarnExt() = default;
  ~ModuleBarnExt() = default;

  void Initialize();
  void Terminate();

  void AddObjectByMetaClass(
    cstring tag,
    LPCMetaClass mc,
    ModuleClearMemory clearMemory,
    bool isModule = true);

  void AddValueByMetaClass();

  void CallFunction(
    cstring name);

  Object *GetObjectByTag(cstring tag);

private:

};

#endif
