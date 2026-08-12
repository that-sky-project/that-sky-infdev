#include <Base/Meta.hpp>
#include "mod/scriptEngine/luacall.hpp"

void lua_debugdostring(
  lua_State *L,
  cstring s
) {
  if (!L || !s)
    return;

  int top = lua_gettop(L);
  const char *error = nullptr;

  if (luaL_loadstring(L, s) != 0) {
    error = lua_tostring(L, -1);
    //LOGW("Compilation err: %s\n", error);
  } else if (lua_pcall(L, 0, LUA_MULTRET, 0) != 0) {
    error = lua_tostring(L, -1);
    //LOGW("Runtime err: %s\n", error);
  }

  lua_settop(L, top);
}
