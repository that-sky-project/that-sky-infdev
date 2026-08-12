#include <includes/htmodloader.h>
#include "mod/base/override.hpp"

static Override *g_Override = nullptr;

void Override::Initialize(
  FakeMetaSystem *metaSystem,
  Game *game
) {
  AssertMsg(!g_Override, "Override is a singleton and must be initialized only once.");
  g_Override = this;

  m_mainThread = std::this_thread::get_id();
  m_game = game;

  m_OverrideMetaSystem(metaSystem);
  m_OverrideModObject();
}

lua_State *Override::GetLua() {
  AssertMsg(m_proxyMetaSystem, "GetLua() must be called after Initialize().");

  if (m_lua)
    return m_lua;

  LPCMetaClass pmcGame = GetMetaClassByName("Game");
  Assert(pmcGame);

  // Resolve Game::lua.
  const auto &variables = pmcGame->m_metaDataContainer->m_variables;
  const auto &itVars = variables.find("lua");
  if (itVars == variables.end())
    AssertMsg(false, "Failed to get Game::lua from metamembervariable.");

  // Get Game.lua.
  const MetaMemberVariable *pmmv = itVars->second;
  lua_State *lua = (m_game->*reinterpret_cast<lua_State *Game::*>(pmmv->Address()));
  Assert(lua);

  m_lua = lua;
  return m_lua;
}

void Override::m_OverrideMetaSystem(
  FakeMetaSystem *metaSystem
) {
  const auto &classes = metaSystem->data->m_metaClasses;

  // Find MetaClassImpl<MetaSystem>.
  const auto &itMetaSystem = classes.find("MetaSystem");
  if (itMetaSystem == classes.end())
    AssertMsg(false, "Failed to get class MetaSystem from registered metaclasses.");

  // Resolve sizeof(MetaSystem).
  const MetaClass *pmcMetaSystem = itMetaSystem->second;
  size_t sizeMetaSystem = pmcMetaSystem->SizeOfObject();
  u32 classCount = (sizeMetaSystem - sizeof(FakeMetaSystem)) / sizeof(const MetaClass *);

  // Find MetaClassImpl<Game>.
  const auto &itGame = classes.find("Game");
  if (itGame == classes.end())
    AssertMsg(false, "Failed to get class Game from registered metaclasses.");

  // Resolve Game::metaSystem.
  const MetaClass *pmcGame = itGame->second;
  const auto &variables = pmcGame->m_metaDataContainer->m_variables;
  const auto &itVars = variables.find("metaSystem");
  if (itVars == variables.end())
    AssertMsg(false, "Failed to get Game::metaSystem from metamembervariable.");

  // Override Game.metaSystem.
  const MetaMemberVariable *pmmv = itVars->second;
  ProxyMetaSystem **ppGameMetaSystem = 
    &(m_game->*reinterpret_cast<ProxyMetaSystem *Game::*>(pmmv->Address()));

  // Create ProxyMetaSystem from MetaSystem.
  m_proxyMetaSystem = ProxyMetaSystem::create();

  // Subtract 15 from the calculated maximum number of classes so that the UIDs of
  // newly added metaclasses are less than 2560.
  //
  // WARN: This is an unsafe and highly incompatible implementation, intended for
  // use in this example only. Do not use it in actual development.
  m_proxyMetaSystem->set(
    reinterpret_cast<const MetaSystemExample *>(metaSystem),
    classCount);

  HTTellText(
    "§a[ThatSkyInfdev] MetaSystem overriden: %p -> %p",
    *ppGameMetaSystem,
    m_proxyMetaSystem);
  HTTellText(
    "§e[ThatSkyInfdev] Copied %u classes of %llu classes",
    m_proxyMetaSystem->m_data->m_count,
    m_proxyMetaSystem->m_data->m_metaClasses.size());

  const MetaSystemExample *old = reinterpret_cast<const MetaSystemExample *>(*ppGameMetaSystem);

  HTTellText("§a[ThatSkyInfdev] Destroying previous.....");
  // Call destructor of MetaStrMap.
  delete old->m_data;
  // Directly free the memory.
  operator delete((void *)old);
  // Reset cached MetaSystem.
  metaSystem = nullptr;

  *ppGameMetaSystem = m_proxyMetaSystem;

  // - Register new metadata.
  m_proxyMetaSystem->submitChain(MetaObject<MetaType>::m_List());
  m_proxyMetaSystem->submitChain(MetaObject<MetaMemberFunction>::m_List());
  m_proxyMetaSystem->submitChain(MetaObject<MetaMemberVariable>::m_List());

  SetMetaSystem(
    m_proxyMetaSystem,
    [](const void *user, i32 id) -> LPCMetaClass {
      return static_cast<const ProxyMetaSystem *>(user)->get(id);
    },
    [](const void *user, cstring name, bool) -> LPCMetaClass {
      return static_cast<const ProxyMetaSystem *>(user)->get(name);
    }
  );
}

void Override::m_OverrideModObject() {

}

Override *GetOverride() {
  return g_Override;
}
