#include "Lua.h"

#include "WorldView.h"

#include "liblua/lauxlib.h"
#include "liblua/lualib.h"

void luaLoadAllLibraries(lua_State *L, WorldView *view) {
   luaL_openlibs(L);
   luaPushUserDataGlobal(L, LLIB_VIEW, view);
   luaRequire(L, "core");
  
   luaLoadActorLibrary(L);
   luaLoadStandardLibrary(L);
   luaLoadUILibrary(L);

}