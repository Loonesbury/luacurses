#ifndef LUACURSES_H
#define LUACURSES_H

#include <curses.h>
#include <panel.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define LC_VERSION "0.90"

#define LUA_PROTO(fn) int fn(lua_State *L)
#define LUA_UNIMP(fn) int fn(lua_State *L) { luaL_error(L, "unimplemented function '%s'", #fn); return 0; }

#define LC_PUSHTRUE(L,x) do { lua_pushboolean((L), (x));        return 1; } while (0)
#define LC_PUSHOK(L,x)   do { lua_pushboolean((L), (x) != ERR); return 1; } while (0)

#if LUA_VERSION_NUM >= 502
#define LC_REGISTER(L,reg) luaL_setfuncs((L),(reg),0)
int luaL_typerror(lua_State *L, int narg, const char *tname);
#else
#define LC_REGISTER(L,reg) luaL_register((L),NULL,(reg))
void luaL_setmetatable(lua_State *L, const char *tname);
#endif

int luaL_checkbool(lua_State *L, int narg);
int luaL_optbool(lua_State *L, int narg, int d);
chtype luaL_checkchar(lua_State *L, int narg);
chtype luaL_optchar(lua_State *L, int narg, int d);
void lua_stackdump(lua_State *L);

extern int lc_initonce;

#endif
