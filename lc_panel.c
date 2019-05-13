#include "lc_window.h"
#include "lc_panel.h"

static winhandle* lc_checkpanel(lua_State *L, int narg)
{
	winhandle *wh = lc_checkhandle(L, narg);
	luaL_argcheck(L, wh->pan != NULL, narg, "not a panel");
	return wh;
}

static winhandle* lc_checknotpanel(lua_State *L, int narg)
{
	winhandle *wh = lc_checkhandle(L, narg);
	luaL_argcheck(L, wh->pan == NULL, narg, "already a panel");
	return wh;
}

static winhandle* lc_findpanel(PANEL *p)
{
	winhandle *cur = lc_winlist;
	if (!p)
		return NULL;
	while (cur && cur->pan != p)
		cur = cur->hnext;
	return cur;
}

/*
* void curses.update_panels()
*/
static LUA_PROTO(c_update_panels)
{
	update_panels();
	return 0;
}

/*
* bool window:new_panel()
*/
static LUA_PROTO(p_new_panel)
{
	winhandle *wh = lc_checknotpanel(L, 1);
	wh->pan = new_panel(wh->win);
	lua_pushboolean(L, wh->pan != NULL);
	return 1;
}

/*
* bool window:bottom_panel()
*/
static LUA_PROTO(p_bottom_panel)
{
	winhandle *wh = lc_checkpanel(L, 1);
	lua_pushboolean(L, bottom_panel(wh->pan) != ERR);
	return 1;
}

/*
* bool window:top_panel()
*/
static LUA_PROTO(p_top_panel)
{
	winhandle *wh = lc_checkpanel(L, 1);
	lua_pushboolean(L, top_panel(wh->pan) != ERR);
	return 1;
}

/*
* bool window:show_panel()
*/
static LUA_PROTO(p_show_panel)
{
	winhandle *wh = lc_checkpanel(L, 1);
	lua_pushboolean(L, show_panel(wh->pan) != ERR);
	return 1;
}

/*
* bool window:hide_panel()
*/
static LUA_PROTO(p_hide_panel)
{
	winhandle *wh = lc_checkpanel(L, 1);
	lua_pushboolean(L, hide_panel(wh->pan) != ERR);
	return 1;
}

/*
* window window:panel_window()
*/
static LUA_PROTO(p_panel_window)
{
	winhandle *wh = lc_checkpanel(L, 1);
	lc_pushhandle(L, wh);
	return 1;
}

static LUA_PROTO(p_replace_panel)
{
	winhandle *wh, *wh2;
	wh = lc_checkpanel(L, 1);
	wh2 = lc_checknotpanel(L, 2);

	if (replace_panel(wh->pan, wh2->win) != ERR) {
		wh2->pan = wh->pan;
		wh->pan = NULL;
		lua_pushboolean(L, 1);
	} else {
		lua_pushboolean(L, 0);
	}

	return 1;
}

static LUA_PROTO(p_move_panel)
{
	winhandle *wh = lc_checkpanel(L, 1);
	int starty = luaL_checkint(L, 2);
	int startx = luaL_checkint(L, 3);
	lua_pushboolean(L, move_panel(wh->pan, starty, startx) != ERR);
	return 1;
}

static LUA_PROTO(p_panel_hidden)
{
	winhandle *wh = lc_checkpanel(L, 1);
	lua_pushboolean(L, panel_hidden(wh->pan));
	return 1;
}

static LUA_PROTO(p_panel_above)
{
	winhandle *wh = lc_checkpanel(L, 1);
	winhandle *above = lc_findpanel(panel_above(wh->pan));
	if (above)
		lc_pushhandle(L, above);
	else
		lua_pushnil(L);
	return 1;
}

static LUA_PROTO(p_panel_below)
{
	winhandle *wh = lc_checkpanel(L, 1);
	winhandle *below = lc_findpanel(panel_below(wh->pan));
	if (below)
		lc_pushhandle(L, below);
	else
		lua_pushnil(L);
	return 1;
}

/*
* bool window:del_panel()
*/
static LUA_PROTO(p_del_panel)
{
	winhandle *wh = lc_checkpanel(L, 1);
	if (del_panel(wh->pan) != ERR) {
		wh->pan = NULL;
		lua_pushboolean(L, 1);
	} else {
		lua_pushboolean(L, 0);
	}
	return 1;
}

#define LCF(fn) { #fn, p_ ## fn }

static const luaL_Reg panelfuncs[] = {
	LCF(new_panel),
	LCF(bottom_panel),
	LCF(top_panel),
	LCF(show_panel),
	LCF(hide_panel),
	LCF(panel_window),
	LCF(replace_panel),
	LCF(move_panel),
	LCF(panel_hidden),
	LCF(panel_above),
	LCF(panel_below),
	LCF(del_panel),
	{ NULL, NULL }
};

void lc_reg_panel(lua_State *L)
{
	luaL_getmetatable(L, LC_WINDOWMT);
	LC_REGISTER(L, panelfuncs);

	lua_pushcfunction(L, c_update_panels);
	lua_setfield(L, -2, "update_panels");
	lua_pop(L, 1);
}
