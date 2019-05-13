#include "luacurses.h"
#include "lc_lib.h"
#include "lc_window.h"
#include "lc_panel.h"
#include "lc_chstr.h"

#if LUA_VERSION_NUM >= 502
int luaL_typerror(lua_State *L, int narg, const char *tname)
{
	/* from lua 5.1 source */
	const char *msg = lua_pushfstring(L, "%s expected, got %s",
		tname, luaL_typename(L, narg));
	return luaL_argerror(L, narg, msg);
}
#else
void luaL_setmetatable(lua_State *L, const char *tname)
{
	luaL_getmetatable(L, tname);
	lua_setmetatable(L, -2);
}
#endif

int luaL_checkbool(lua_State *L, int narg)
{
	luaL_checktype(L, narg, LUA_TBOOLEAN);
	return lua_toboolean(L, narg);
}

int luaL_optbool(lua_State *L, int narg, int d)
{
	if (lua_isnoneornil(L, narg))
		return d;
	luaL_checktype(L, narg, LUA_TBOOLEAN);
	return lua_toboolean(L, narg);
}

chtype luaL_checkchar(lua_State *L, int narg)
{
	if (lua_isnumber(L, narg)) {
		return luaL_checkint(L, narg);
	} else if (lua_isstring(L, narg)) {
		return *luaL_checkstring(L, narg);
	} else {
		luaL_typerror(L, narg, "string or number");
		return 0;
	}
}

chtype luaL_optchar(lua_State *L, int narg, int d)
{
	if (lua_isnoneornil(L, narg))
		return d;
	return luaL_checkchar(L, narg);
}

void lua_stackdump(lua_State *L) {
	int i = lua_gettop(L);
	printf("--- Stack dump ---\n");
	while (i) {
		int t = lua_type(L, i);
		switch (t) {
			case LUA_TSTRING:
				printf("%d:  \"%s\"\n", i, lua_tostring(L, i));
				break;
			case LUA_TBOOLEAN:
				printf("%d:  %s\n", i, lua_toboolean(L, i) ? "true" : "false");
				break;
			case LUA_TNUMBER:
				printf("%d:  %g\n",  i, lua_tonumber(L, i));
				break;
			default:
				printf("%d:  %s\n", i, lua_typename(L, t));
				break;
		}
		i--;
	}
	printf("---\n");
}

LUA_PROTO(luaopen_curses_core)
{
	lua_newtable(L);

	lc_reg_lib(L);
	lc_reg_window(L);
	lc_reg_panel(L);
	lc_reg_chstr(L);

	lua_pushstring(L, LC_VERSION);
	lua_setfield(L, -2, "_VERSION");

	return 1;
}
