#include <errno.h>
#include <includes/htmodloader.h>
#include "utils/htmodloader.hpp"
#include "sky/skyGame.hpp"
#include "mod/meta.hpp"

typedef void (__fastcall *PFN_MetaSystem_Initialize)(
  FakeMetaSystem *);
typedef void (__fastcall *PFN_Game_Alloc)(
  Game *);
typedef const MetaClass *(__fastcall *PFN_GetMetaClassById)(
  u32);
typedef const MetaClass *(__fastcall *PFN_GetMetaClassByName)(
  cstring, bool);

FakeMetaSystem *gMetaSystem = nullptr;
ProxyMetaSystem *gProxyMetaSystem = nullptr;
Game *gGame = nullptr;

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

  gMetaSystem = self;
}

static void hook_Game_Alloc(
  Game *self
) {
  const auto &classes = gMetaSystem->data->m_metaClasses;

  // Resolve sizeof(MetaSystem).
  const auto &itMetaSystem = classes.find("MetaSystem");
  if (itMetaSystem == classes.end())
    return ((PFN_Game_Alloc)sfn_Game_Alloc.origin)(self);

  const MetaClass *pmcMetaSystem = itMetaSystem->second;
  size_t sizeMetaSystem = pmcMetaSystem->SizeOfObject();
  u32 classCount = (sizeMetaSystem - sizeof(FakeMetaSystem)) / sizeof(const MetaClass *);

  // Resolve Game.metaSystem.
  const auto &itGame = classes.find("Game");
  if (itGame == classes.end())
    return ((PFN_Game_Alloc)sfn_Game_Alloc.origin)(self);

  const MetaClass *pmcGame = itGame->second;
  const auto &variables = pmcGame->m_metaDataContainer->m_variables;
  const auto &itVars = variables.find("metaSystem");
  if (itVars == variables.end())
    return ((PFN_Game_Alloc)sfn_Game_Alloc.origin)(self);

  // Override Game.metaSystem.
  const MetaMemberVariable *pmmvMetaSystem = itVars->second;
  //ProxyMetaSystem **ppGameMetaSystem = (ProxyMetaSystem **)((char *)self + pmmvMetaSystem->GetContext().offset);
  ProxyMetaSystem **ppGameMetaSystem = 
    &(self->*reinterpret_cast<ProxyMetaSystem *Game::*>(pmmvMetaSystem->Address()));

  // Create ProxyMetaSystem from MetaSystem.
  gProxyMetaSystem = ProxyMetaSystem::create();

  // Subtract 15 from the calculated maximum number of classes so that the UIDs of
  // newly added metaclasses are less than 2560.
  //
  // WARN: This is an unsafe and highly incompatible implementation, intended for
  // use in this example only. Do not use it in actual development.
  gProxyMetaSystem->set(
    reinterpret_cast<const MetaSystem *>(gMetaSystem),
    classCount);

  HTTellText(
    "§a[ThatSkyInfdev] MetaSystem overriden: %p -> %p",
    *ppGameMetaSystem,
    gProxyMetaSystem);
  HTTellText(
    "§e[ThatSkyInfdev] Copied %u classes of %llu classes",
    gProxyMetaSystem->m_data->m_count,
    gProxyMetaSystem->m_data->m_metaClasses.size());

  const MetaSystem *old = reinterpret_cast<const MetaSystem *>(*ppGameMetaSystem);

  HTTellText("§a[ThatSkyInfdev] Destroying previous.....");
  // Call destructor of MetaStrMap.
  delete old->m_data;
  // Directly free the memory.
  operator delete((void *)old);

  *ppGameMetaSystem = gProxyMetaSystem;

  // - Register new metadata.
  gProxyMetaSystem->submitChain(MetaObject<MetaType>::m_List());
  gProxyMetaSystem->submitChain(MetaObject<MetaMemberFunction>::m_List());
  gProxyMetaSystem->submitChain(MetaObject<MetaMemberVariable>::m_List());

  MetaSystem::SetMetaSystem(reinterpret_cast<MetaSystem *>(gProxyMetaSystem));

  ((PFN_Game_Alloc)sfn_Game_Alloc.origin)(self);
}

static LPCMetaClass hook_GetMetaClassById(
  u32 id
) {
  if (!gProxyMetaSystem)
    return ((PFN_GetMetaClassById)sfn_GetMetaClassById.origin)(id);

  if (id > gProxyMetaSystem->m_data->m_maxClasses)
    return nullptr;

  return gProxyMetaSystem->m_classes[id];
}

static LPCMetaClass hook_GetMetaClassByName(
  cstring name,
  bool isConstString
) {
  if (!gProxyMetaSystem)
    return ((PFN_GetMetaClassByName)sfn_GetMetaClassByName.origin)(name, isConstString);

  if (!name)
    return nullptr;

  const auto &classes = gProxyMetaSystem->m_data->m_metaClasses;
  const auto &it = classes.find(name);

  if (it == classes.end())
    return nullptr;

  return it->second;
}

void MetaSystemOverride::initialize() {
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
