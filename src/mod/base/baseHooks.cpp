#include <errno.h>
#include <includes/htmodloader.h>
#include "utils/htmodloader.hpp"
#include "sky/skyGame.hpp"
#include "mod/base/override.hpp"
#include "mod/base/proxyMetaSystem.hpp"
#include "mod/base/baseHooks.hpp"

typedef void (__fastcall *PFN_MetaSystem_Initialize)(
  FakeMetaSystem *);
typedef void (__fastcall *PFN_Game_Alloc)(
  Game *);
typedef const MetaClass *(__fastcall *PFN_GetMetaClassById)(
  u32);
typedef const MetaClass *(__fastcall *PFN_GetMetaClassByName)(
  cstring, bool);

static FakeMetaSystem *g_metaSystemCache = nullptr;

static const HTAsmSig sigE8_MetaSystem_Initialize{
  "31 D2 E8 ?  ?  ?  ?  48 89 D9 E8 ?  ?  ?  ?  B9 "
  "?  ?  ?  ?  E8",
  HT_SCAN_E8,
  0x0A
};

static const HTAsmSig sigE8_Game_Alloc{
  "E8 ?  ?  ?  ?  C7 87 ?  ?  ?  ?  ?  ?  ?  ?  48 "
  "89 F9 E8 ?  ?  ?  ?  48 8B 06 ",
  HT_SCAN_E8,
  0x12
};

static const HTAsmSig sigE8_GetMetaClassById{
  "4C 89 8C 24 ?  ?  ?  ?  41 8B 08 E8 ?  ?  ?  ?  "
  "48 8B 10 48 89 C1 ",
  HT_SCAN_E8,
  0x0B
};

static const HTAsmSig sigE8_GetMetaClassByName{
  "48 89 D9 31 D2 E8 ?  ?  ?  ?  48 89 C3 E8 ",
  HT_SCAN_E8,
  0x05
};

static HTAsmFunction sfn_MetaSystem_Initialize{
  "MetaSystem::Initialize()",
  nullptr,
  nullptr,
  nullptr
};

static HTAsmFunction sfn_Game_Alloc{
  "Game::Alloc()",
  nullptr,
  nullptr,
  nullptr
};

static HTAsmFunction sfn_GetMetaClassById{
  "GetMetaClassById()",
  nullptr,
  nullptr,
  nullptr
};

static HTAsmFunction sfn_GetMetaClassByName{
  "GetMetaClassByName()",
  nullptr,
  nullptr,
  nullptr
};

static void hook_MetaSystem_Initialize(
  FakeMetaSystem *self
) {
  ((PFN_MetaSystem_Initialize)sfn_MetaSystem_Initialize.origin)(self);

  g_metaSystemCache = self;
}

static void hook_Game_Alloc(
  Game *self
) {
  Override *ov = new Override();
  ov->Initialize(g_metaSystemCache, self);
  // Reset the cache because it is freed.
  g_metaSystemCache = nullptr;

  ((PFN_Game_Alloc)sfn_Game_Alloc.origin)(self);
}

static LPCMetaClass hook_GetMetaClassById(
  u32 id
) {
  if (!GetOverride())
    return ((PFN_GetMetaClassById)sfn_GetMetaClassById.origin)(id);

  return GetMetaClassById(id);
}

static LPCMetaClass hook_GetMetaClassByName(
  cstring name,
  bool isConstString
) {
  if (!GetOverride())
    return ((PFN_GetMetaClassByName)sfn_GetMetaClassByName.origin)(name, isConstString);

  return GetMetaClassByName(name);
}

void BaseHooks::Initialize() {
  sfn_MetaSystem_Initialize.detour = (void *)hook_MetaSystem_Initialize;
  HTModLoader::createHookAndEnable(
    &sigE8_MetaSystem_Initialize,
    &sfn_MetaSystem_Initialize);

  sfn_Game_Alloc.detour = (void *)hook_Game_Alloc;
  HTModLoader::createHookAndEnable(
    &sigE8_Game_Alloc,
    &sfn_Game_Alloc);

  sfn_GetMetaClassById.detour = (void *)hook_GetMetaClassById;
  HTModLoader::createHookAndEnable(
    &sigE8_GetMetaClassById,
    &sfn_GetMetaClassById);

  sfn_GetMetaClassByName.detour = (void *)hook_GetMetaClassByName;
  HTModLoader::createHookAndEnable(
    &sigE8_GetMetaClassByName,
    &sfn_GetMetaClassByName);
}
