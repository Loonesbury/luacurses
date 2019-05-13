#include "lc_chstr.h"
#include <string.h>
#include <stdlib.h>

chstr* lc_pushchstr(lua_State *L, int len)
{
	int sz = sizeof(chstr) + len * sizeof(chtype);
	chstr *cs = (chstr*)lua_newuserdata(L, sz);
	luaL_setmetatable(L, LC_CHSTRMT);
	memset(cs, 0, sz);
	cs->len = len;
	return cs;
}

chstr* lc_checkchstr(lua_State *L, int narg)
{
	return (chstr*)luaL_checkudata(L, narg, LC_CHSTRMT);
}

/*
* chstr curses.chstr(int len)
* Returns a new chstr of the given length, initialized to all spaces
*/
static LUA_PROTO(lc_chstr)
{
	chstr *cs;
	chtype *ch, *chend;
	int len = luaL_checkint(L, 1);
	luaL_argcheck(L, len > 0, 1, "invalid length");
	cs = lc_pushchstr(L, len);
	for (ch = cs->str, chend = ch + len; ch < chend; ch++)
		*ch = ' ';
	return 1;
}

/*
* void chstr:set_str(int offset, str value, [int attrs=A_NORMAL], [int reps=1])
* Overwrites the contents of the chstr starting at the given offset
*/
static LUA_PROTO(cs_set_str)
{
	chstr *cs = lc_checkchstr(L, 1);
	int offset = luaL_checkint(L, 2);
	const char *str = luaL_checkstring(L, 3);
	int attrs = luaL_optint(L, 4, 0);
	int reps = luaL_optint(L, 5, 1);
	int len = strlen(str);
	int iters = len*reps;
	int i;

	luaL_argcheck(L, offset >= 0, 2, "invalid offset");
	if (offset >= cs->len) {
		/* do nothing */
		return 0;
	}
	if (offset + iters > cs->len)
		iters = cs->len - offset;

	for (i = 0; i < iters; i++)
		cs->str[offset + i] = str[i % len] | attrs;

	return 0;
}

/*
* void chstr:set_ch(int offset, int/str ch, [int attrs=A_NORMAL], [int reps=1])
* Overwrites the contents of the chstr at the given offset
*/
static LUA_PROTO(cs_set_ch)
{
	chstr *cs  = lc_checkchstr(L, 1);
	int offset = luaL_checkint(L, 2);
	chtype ch  = lua_isstring(L, 3) ? (chtype)(*lua_tostring(L, 3))
	           : lua_isnumber(L, 3) ? lua_tonumber(L, 3)
			   : luaL_typerror(L, 3, "number or string");
	int attrs  = luaL_optint(L, 4, 0);
	int reps   = luaL_optint(L, 5, 1);
	int i;

	luaL_argcheck(L, offset >= 0, 2, "invalid offset");
	if (offset >= cs->len) {
		/* do nothing */
		return 0;
	}
	if (offset + reps > cs->len)
		reps = cs->len - offset;

	for (i = 0; i < reps; i++)
		cs->str[offset + i] = ch | attrs;

	return 0;
}

/*
* (int, int, int) OR void chstr:get(int offset)
* Returns the char, attrs, and colorpair at the given offset
* Returns (no value) if offset is invalid
*/
static LUA_PROTO(cs_get)
{
	chstr *cs = lc_checkchstr(L, 1);
	int offset = luaL_checkint(L, 2);
	chtype ch;

	if (offset >= cs->len)
		return 0;

	ch = cs->str[offset];
	lua_pushnumber(L, ch & A_CHARTEXT);
	lua_pushnumber(L, ch & A_ATTRIBUTES);
	lua_pushnumber(L, ch & A_COLOR);
	return 3;
}

/*
* str chstr:get_str()
* Returns a plain string representation of the chstr's contents, attributes
* or colors.  This assumes the chstr does not contain embedded NULLs.
*/
static LUA_PROTO(cs_get_str)
{
	chstr *cs = lc_checkchstr(L, 1);
	/* don't look at this */
	char *b = malloc(cs->len + 1);
	int i;
	for (i = 0; i < cs->len && cs->str[i] != 0; i++)
		b[i] = cs->str[i] & A_CHARTEXT;
	b[i] = '\0';
	lua_pushstring(L, b);
	free(b);
	return 1;
}

/*
* int chstr:len()
* Returns the length of the given chstr
*/
static LUA_PROTO(cs_len)
{
	chstr *cs = lc_checkchstr(L, 1);
	/* chtype *p = cs->str;
	while (*p) p++;
	lua_pushnumber(L, p - cs->str); */
	lua_pushnumber(L, cs->len);
	return 1;
}

/*
* chstr chstr:dup()
* Returns a copy of the given chstr
*/
static LUA_PROTO(cs_dup)
{
	chstr *cs = lc_checkchstr(L, 1);
	chstr *copy = lc_pushchstr(L, cs->len);
	memcpy(copy->str, cs->str, cs->len * sizeof(chtype));
	return 1;
}

/*
* str chstr:__tostring()
* Returns "chstr($len)"
*/
static LUA_PROTO(cs___tostring)
{
	chstr *cs = lc_checkchstr(L, 1);
	lua_pushfstring(L, "chstr(%d)", cs->len);
	return 1;
}

#define LCF(fn) { #fn, cs_ ## fn }

static const luaL_Reg chstrfuncs[] = {
	LCF(__tostring),
	{ "__len", cs_len },
	LCF(set_str),
	LCF(set_ch),
	LCF(get),
	LCF(get_str),
	LCF(len),
	LCF(dup),
	{ NULL, NULL }
};

void lc_reg_chstr(lua_State *L)
{
	luaL_newmetatable(L, LC_CHSTRMT);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	LC_REGISTER(L, chstrfuncs);

	lua_setfield(L, -2, "_CHSTR");

	lua_pushcfunction(L, lc_chstr);
	lua_setfield(L, -2, "chstr");
}
