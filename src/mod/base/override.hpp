#ifndef __MOD_BASE_OVERRIDE_HPP__
#define __MOD_BASE_OVERRIDE_HPP__

#include <thread>
#include <Base/Meta.hpp>
#include "mod/base/proxyMetaSystem.hpp"
#include "sky/skyGame.hpp"

class Override {
public:
  Override() = default;
  ~Override() = default;

  void Initialize(
    FakeMetaSystem *metaSystem,
    Game *game);

  lua_State *GetLua();

private:
  void m_OverrideMetaSystem(
    FakeMetaSystem *metaSystem);
  void m_OverrideModObject();

  std::thread::id m_mainThread = {};
  ProxyMetaSystem *m_proxyMetaSystem = nullptr;
  Game *m_game = nullptr;
  lua_State *m_lua = nullptr;
};

Override *GetOverride();

#endif
