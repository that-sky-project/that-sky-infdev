#ifndef __MOD_LUACALL__
#define __MOD_LUACALL__

#include <Base/Meta.hpp>
#include <Utils/Types.h>

void lua_debugdostring(
  lua_State *L,
  cstring s);

void lua_debugdofile(
  lua_State *L);

#endif
