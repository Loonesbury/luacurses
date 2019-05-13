#include "luacurses.h"
#include "lc_window.h"
#include "lc_chstr.h"
#include <stdlib.h>
#include <string.h>

winhandle *lc_winlist = NULL;

winhandle* lc_findwindow(lua_State *L, WINDOW *w)
{
	winhandle *cur = lc_winlist;
	if (!w)
		return NULL;
	while (cur && cur->win != w)
		cur = cur->hnext;
	return cur;
}

winhandle* lc_pushhandle(lua_State *L, winhandle *wh)
{
	winhandle **whptr = lua_newuserdata(L, sizeof(winhandle*));
	*whptr = wh;
	luaL_setmetatable(L, LC_WINDOWMT);

	wh->refs += 1;

	return wh;
}

winhandle* lc_pushwindow(lua_State *L, WINDOW *w)
{
	winhandle *wh;

	if (!w) {
		lua_pushnil(L);
		return NULL;
	}

	wh = lc_findwindow(L, w);
	if (!wh) {
		wh = memset(malloc(sizeof(winhandle)), 0, sizeof(winhandle));
		wh->win = w;

		wh->hnext = lc_winlist;
		lc_winlist = wh;
	}

	return lc_pushhandle(L, wh);
}

winhandle* lc_checkhandle(lua_State *L, int narg)
{
	winhandle *wh = *((winhandle**)luaL_checkudata(L, narg, LC_WINDOWMT));
	luaL_argcheck(L, narg, wh->win != NULL, "invalid window");
	return wh;
}

WINDOW* lc_checkwindow(lua_State *L, int narg)
{
	return lc_checkhandle(L, narg)->win;
}

/*
* deletes the underlying WINDOW* and sets the 'win' ptr to NULL for the
* given handle and all of its subhandles (will fail if any subwindows
* weren't created by luacurses).
* returns true if delwin() returned OK or if the window was already deleted.
*/
int lc_close_handle(winhandle *wh, int top)
{
	winhandle *cur;

	/* if we're already dead, stop here */
	if (!wh->win)
		return 1;

	/*
	* NOTE: we do NOT yank it from the window list here, because that would
	* cause a duplicate winhandle struct if the WINDOW* is ever pushed again.
	* we yank it in __gc when all other refs are dead.
	*/

	/* kill subwindows first */
	cur = wh->sub;
	while (cur) {
		lc_close_handle(cur, 0);
		cur = cur->next;
	}
	wh->sub = NULL;

	/*
	* if we're being called recursively we're looping through a list of
	* subwindows so we don't really give a shit about closing the gap in
	* the list.
	* if this is the first time we were called, we do need to close it.
	*/
	if (top && wh->parent) {
		cur = wh->parent->sub;
		if (cur == wh) {
			wh->parent->sub = cur->next;
		} else {
			/* find ourselves in the list */
			while (cur) {
				if (cur->next == wh) {
					cur->next = wh->next;
					break;
				}
				cur = cur->next;
			}
		}
	}

	if (wh->win) {
		int rv = delwin(wh->win);
		wh->win = NULL;
		return rv != ERR;
	}

	return 1;
}

int lc_checkmv(lua_State *L, WINDOW *w, int pushnil)
{
	if (lua_type(L, 2) != LUA_TNUMBER || lua_type(L, 3) != LUA_TNUMBER)
		return 1;
	if (wmove(w, luaL_checkint(L, 2), luaL_checkint(L, 3)) == ERR) {
		if (pushnil)
			lua_pushnil(L);
		else
			lua_pushboolean(L, 0);
		return 0;
	}
	lua_remove(L, 2);
	lua_remove(L, 2);
	return 1;
}

/*
* bool window:addch([int y, int x,] int/char ch)
* Adds the given character to the window at the cursor position.
* Accepts a number or string.
*/
static LUA_PROTO(w_addch)
{
	WINDOW *w = lc_checkwindow(L, 1);
	chtype ch;

	if (!lc_checkmv(L, w, 0))
		return 1;

	/* avoid automatic string <-> number conversion */
	if (lua_type(L, 2) == LUA_TNUMBER)
		ch = lua_tointeger(L, 2);
	else if (lua_type(L, 2) == LUA_TSTRING)
		ch = *luaL_checkstring(L, 2);
	else
		luaL_typerror(L, 2, "number or string");

	lua_pushboolean(L, waddch(w, ch) != ERR);
	return 1;
}

/*
* bool window:addchstr([int y, int x,] chstr, n = -1)
* Adds a chtype array to the window, created with curses.chstr(),
* truncating at EOL.
* If n is specified, only writes n chars.
*/
LUA_PROTO(w_addchstr)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int rv;
	chstr *cs;
	if (!lc_checkmv(L, w, 0))
		return 1;
	cs = lc_checkchstr(L, 2);
	if (!lua_isnoneornil(L, 3))
		rv = waddchnstr(w, cs->str, luaL_checkint(L, 3));
	else
		rv = waddchstr(w, cs->str);
	lua_pushboolean(L, rv != ERR);
	return 1;
}

/*
* bool window:addstr([int y, int x,] str, n = -1)
* Adds a string to the window, wrapping at EOL.
* If n is specified, only writes n chars.
*/
static LUA_PROTO(w_addstr)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int rv;
	if (!lc_checkmv(L, w, 0))
		return 1;
	if (!lua_isnoneornil(L, 3))
		rv = waddnstr(w, luaL_checkstring(L, 2), luaL_checkint(L, 3));
	else
		rv = waddstr(w, luaL_checkstring(L, 2));
	lua_pushboolean(L, rv != ERR);
	return 1;
}

/*
* int attrs, pair = window:attr_get()
*/
static LUA_PROTO(w_attr_get)
{
	attr_t attrs = -1;
	short pair = -1;
	WINDOW *w = lc_checkwindow(L, 1);
	if (wattr_get(w, &attrs, &pair, NULL) == ERR) {
		lua_pushnil(L);
		return 1;
	}
	lua_pushinteger(L, attrs);
	lua_pushinteger(L, pair);
	return 2;
}

/*
* bool window:attr_off(int attrs)
* Disables the given attributes.
*/
static LUA_PROTO(w_attr_off)
{
	WINDOW *w = lc_checkwindow(L, 1);
	attr_t attrs = luaL_checkint(L, 2);
	lua_pushboolean(L, wattr_off(w, attrs, NULL) != ERR);
	return 1;
}

/*
* bool window:attr_on(int attrs)
* Enables the given attributes.
*/
static LUA_PROTO(w_attr_on)
{
	WINDOW *w = lc_checkwindow(L, 1);
	attr_t attrs = luaL_checkint(L, 2);
	lua_pushboolean(L, wattr_on(w, attrs, NULL) != ERR);
	return 1;
}

/*
* bool window:attr_set(int attrs, int pair)
* Set the window's attributes and color, overriding the previous.
*/
static LUA_PROTO(w_attr_set)
{
	WINDOW *w = lc_checkwindow(L, 1);
	attr_t attrs = luaL_checkint(L, 2);
	short pair = luaL_checkint(L, 3);
	lua_pushboolean(L, wattr_set(w, attrs, pair, NULL) != ERR);
	return 1;
}

/*
* bool window:attroff(int attr)
* Disables the given attributes, which may be OR'd with a color pair under 256.
*/
static LUA_PROTO(w_attroff)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int attrs = luaL_checkint(L, 2);
	lua_pushboolean(L, wattroff(w, attrs) != ERR);
	return 1;
}

/*
* bool window:attron(int attr)
* Enables the given attributes, which may be OR'd with a color pair under 256.
*/
static LUA_PROTO(w_attron)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int attrs = luaL_checkint(L, 2);
	lua_pushboolean(L, wattron(w, attrs) != ERR);
	return 1;
}

/*
* bool window:attrset(int attr)
* Sets the given attributes, which may be OR'd with a color pair under 256,
* and overrides anything previous.
*/
static LUA_PROTO(w_attrset)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int attrs = luaL_checkint(L, 2);
	lua_pushboolean(L, wattrset(w, attrs) != ERR);
	return 1;
}

/*
* bool window:bkgd(int ch)
* Sets the background char and attributes of the window.
* The new char replaces the old one where it appears.
* The attributes apply to any chars subsequently written with addch().
*/
static LUA_PROTO(w_bkgd)
{
	WINDOW *w = lc_checkwindow(L, 1);
	chtype ch = luaL_checkchar(L, 2);
	lua_pushboolean(L, wbkgd(w, ch) != ERR);
	return 1;
}

/*
* void window:bkgdset(int ch)
* Sets the background char and attributes of the window.
* The new char and attributes apply to any chars subsequently written
* with addch().
*/
static LUA_PROTO(w_bkgdset)
{
	WINDOW *w = lc_checkwindow(L, 1);
	chtype ch = luaL_checkchar(L, 2);
	wbkgd(w, ch);
	return 1;
}

/*
* Draws a box around the edges of the window.
*/
static LUA_PROTO(w_border)
{
	WINDOW *w = lc_checkwindow(L, 1);
	lua_pushboolean(L, wborder(
		w,
		luaL_optint(L, 2, 0),
		luaL_optint(L, 3, 0),
		luaL_optint(L, 4, 0),
		luaL_optint(L, 5, 0),
		luaL_optint(L, 6, 0),
		luaL_optint(L, 7, 0),
		luaL_optint(L, 8, 0),
		luaL_optint(L, 9, 0)
	) != ERR);
	return 1;
}

/*
* Draws a box around the edges of the window.
*/
static LUA_PROTO(w_box)
{
	WINDOW *w = lc_checkwindow(L, 1);
	lua_pushboolean(L, box(w, luaL_optint(L, 2, 0), luaL_optint(L, 3, 0)) != ERR);
	return 1;
}

/*
* bool window:chgat([int x, int y,] int n, int attr, int pair)
*/
static LUA_PROTO(w_chgat)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int n;
	attr_t attr;
	short pair;
	if (!lc_checkmv(L, w, 0))
		return 1;
	n = luaL_checkint(L, 2);
	attr = luaL_checkint(L, 3);
	pair = luaL_checkint(L, 4);
	lua_pushboolean(L, wchgat(w, n, attr, pair, NULL) != ERR);
	return 1;
}

/*
* bool window:clear()
*/
static LUA_PROTO(w_clear)
{
	lua_pushboolean(L, wclear(lc_checkwindow(L, 1)) != ERR);
	return 1;
}

/*
* bool window:clearok(bool ok)
* If true, the next call to wrefresh with this window will completely clear
* and redraw the screen. If used on curscr, the next refresh of any window
* will repaint the screen.
*/
static LUA_PROTO(w_clearok)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int ok = luaL_checkbool(L, 2);
	lua_pushboolean(L, clearok(w, ok) != ERR);
	return 1;
}

/*
* bool window:clrtobot()
*/
static LUA_PROTO(w_clrtobot)
{
	lua_pushboolean(L, wclrtobot(lc_checkwindow(L, 1)) != ERR);
	return 1;
}

/*
* bool window:clrtoeol()
*/
static LUA_PROTO(w_clrtoeol)
{
	lua_pushboolean(L, wclrtoeol(lc_checkwindow(L, 1)) != ERR);
	return 1;
}

/*
* bool window:color_set(int pair)
* Sets the current foreground/background combination to 'pair'.
*/
static LUA_PROTO(w_color_set)
{
	WINDOW *w = lc_checkwindow(L, 1);
	short pair = luaL_checkint(L, 2);
	lua_pushboolean(L, wcolor_set(w, pair, NULL) != ERR);
	return 1;
}

/*
* bool window:copywin(window dest, int sminrow, int smincol,
*                     int dminrow, int dmincol, int dmaxrow,
*                     int dmaxcol, [bool overlay])
* Copies to a rectangle on the destination window from (dminrow,dmincol) to
* (dmaxrow,dmaxcol), starting at (sminrow,smincol) on the current window.
* If overlay is true, blanks are not copied.
*/
static LUA_PROTO(w_copywin)
{
	lua_pushboolean(L, copywin(
		lc_checkwindow(L, 1),
		lc_checkwindow(L, 2),
		luaL_checkint(L, 3),
		luaL_checkint(L, 4),
		luaL_checkint(L, 5),
		luaL_checkint(L, 6),
		luaL_checkint(L, 7),
		luaL_checkint(L, 8),
		luaL_optbool(L, 9, 0)
	) != ERR);
	return 1;
}

/*
* void window:cursyncup()
*/
static LUA_PROTO(w_cursyncup)
{
	wcursyncup(lc_checkwindow(L, 1));
	return 1;
}

/*
* bool window:delch([int x, int y])
*/
static LUA_PROTO(w_delch)
{
	WINDOW *w = lc_checkwindow(L, 1);
	if (lc_checkmv(L, w, 0))
		lua_pushboolean(L, wdelch(w) != ERR);
	return 1;
}

/*
* bool window:deleteln()
*/
static LUA_PROTO(w_deleteln)
{
	lua_pushboolean(L, wdeleteln(lc_checkwindow(L, 1)) != ERR);
	return 1;
}

/*
* bool window:delwin()
* Deletes the window (but does not erase its screen image).
* Subwindows will be safely deleted first.
*/
static LUA_PROTO(w_delwin)
{
	winhandle *wh = lc_checkhandle(L, 1);
	lua_pushboolean(L, lc_closehandle(wh));
	return 1;
}

/*
* window window:derwin(int nlines, int ncols, int beginy, int beginx)
* Like subwin, but y and x are relative to the origin of the window rather
* than the screen. Use mvderwin to move a derived window/subwindow inside
* its parent window.
*/
static LUA_PROTO(w_derwin)
{
	WINDOW *dw = derwin(
		lc_checkwindow(L, 1),
		luaL_checkint(L, 2), luaL_checkint(L, 3),
		luaL_checkint(L, 4), luaL_checkint(L, 5)
	);
	if (dw) {
		lc_pushwindow(L, dw);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/*
* window window:dupwin()
* Creates an exact duplicate.
*/
static LUA_PROTO(w_dupwin)
{
	WINDOW *dw = dupwin(lc_checkwindow(L, 1));
	if (dw) {
		lc_pushwindow(L, dw);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/*
* bool window:echochar()
*/
static LUA_PROTO(w_echochar)
{
	WINDOW *w = lc_checkwindow(L, 1);
	chtype ch = luaL_checkint(L, 2);
	lua_pushboolean(L, wechochar(w, ch) != ERR);
	return 1;
}

/*
* bool window:erase()
*/
static LUA_PROTO(w_erase)
{
	lua_pushboolean(L, werase(lc_checkwindow(L, 1)) != ERR);
	return 1;
}

/*
* int window:getattrs()
* Returns the window's current attributes in a single int.
*/
static LUA_PROTO(w_getattrs)
{
	lua_pushinteger(L, getattrs(lc_checkwindow(L, 1)));
	return 1;
}

/*
* y, x = window:getbegyx()
* Returns the window's beginning coordinates.
*/
static LUA_PROTO(w_getbegyx)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int y = -1, x = -1;
	getyx(w, y, x);
	lua_pushinteger(L, y);
	lua_pushinteger(L, x);
	return 2;
}

/*
* int window:getbkgd()
* Returns the window's background char and attributes.
*/
static LUA_PROTO(w_getbkgd)
{
	lua_pushinteger(L, getbkgd(lc_checkwindow(L, 1)));
	return 1;
}

/*
* int window:getch([int x, int y])
*/
static LUA_PROTO(w_getch)
{
	WINDOW *w = lc_checkwindow(L, 1);
	if (lc_checkmv(L, w, 1))
		lua_pushinteger(L, wgetch(w));
	return 1;
}

/*
* y, x = window:getmaxyx()
* Returns the window's max coordinates, i.e. its size.
*/
static LUA_PROTO(w_getmaxyx)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int y, x;
	getmaxyx(w, y, x);
	lua_pushinteger(L, y);
	lua_pushinteger(L, x);
	return 2;
}

/*
* y, x = window:getparyx()
* Returns a subwindow's position relative to its parent.
* If not a subwindow, returns nil.
*/
static LUA_PROTO(w_getparyx)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int y, x;
	getparyx(w, y, x);
	if (y > 0) {
		lua_pushinteger(L, y);
		lua_pushinteger(L, x);
		return 2;
	} else {
		lua_pushnil(L);
		return 1;
	}
}

/*
* str window:getstr([int x, int y,] [int n])
*/
static LUA_PROTO(w_getstr)
{
	WINDOW *w = lc_checkwindow(L, 1);
	char *buf;
	int n;
	if (!lc_checkmv(L, w, 1))
		return 1;
	n = luaL_optint(L, 2, -1);
	if (n < 0) {
		n = LUAL_BUFFERSIZE;
	} else if (n == 0) {
		lua_pushstring(L, "");
		return 1;
	}
	buf = malloc(n + 1);
	if (wgetnstr(w, buf, n) != ERR) {
		lua_pushlstring(L, buf, n);
	} else {
		lua_pushnil(L);
	}
	free(buf);
	return 1;
}

/*
* y, x = window:getyx()
* Gets the window's cursor position.
*/
static LUA_PROTO(w_getyx)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int y, x;
	getyx(w, y, x);
	lua_pushinteger(L, y);
	lua_pushinteger(L, x);
	return 2;
}

/*
* bool window:hline([int y, int x], [int/str ch], [int n])
*/
static LUA_PROTO(w_hline)
{
	WINDOW *w = lc_checkwindow(L, 1);
	chtype ch;
	int n;
	if (lc_checkmv(L, w, 0))
		return 1;
	ch = luaL_optint(L, 2, -1);
	n = luaL_optint(L, 3, COLS);
	lua_pushboolean(L, whline(w, ch, n) != ERR);
	return 1;
}

/*
* void window:idcok(bool ok)
* Tells curses to use the terminal's insert-/delete-char functionality
* if possible.
*/
static LUA_PROTO(w_idcok)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int ok = luaL_checkbool(L, 2);
	idcok(w, ok);
	return 0;
}

/*
* bool window:idlok(bool ok)
* Tells curses to use the terminal's insert-/delete-line functionality
* if possible. Returns false on error.
*/
static LUA_PROTO(w_idlok)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int ok = luaL_checkbool(L, 2);
	lua_pushboolean(L, idlok(w, ok) != ERR);
	return 1;
}

/*
* void window:immedok(bool ok)
* If enabled, any change in the window image (addch, clrtobot, scrl, etc)
* will cause an immediate refresh. Degrades performance, off by default.
*/
static LUA_PROTO(w_immedok)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int ok = luaL_checkbool(L, 2);
	immedok(w, ok);
	return 0;
}

/*
* int window:inch([int y, int x])
*/
static LUA_PROTO(w_inch)
{
	WINDOW *w = lc_checkwindow(L, 1);
	if (lc_checkmv(L, w, 1))
		lua_pushinteger(L, winch(w));
	return 1;
}

/*
* chstr window:chstr([int y, int x,] [int n])
*/
LUA_UNIMP(w_inchstr)

/*
* bool window:insch([int y, int x,] int/str ch)
*/
static LUA_PROTO(w_insch)
{
	WINDOW *w = lc_checkwindow(L, 1);
	chtype ch;

	if (!lc_checkmv(L, w, 0))
		return 1;

	if (lua_isnumber(L, 2))
		ch = lua_tointeger(L, 2);
	else
		ch = *luaL_checkstring(L, 2);
	lua_pushboolean(L, winsch(w, ch) != ERR);
	return 1;
}

/*
* bool window:insdelln(int n)
*/
static LUA_PROTO(w_insdelln)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int n = luaL_checkint(L, 2);
	lua_pushboolean(L, winsdelln(w, n) != ERR);
	return 1;
}

/*
* bool widnow:insertln()
*/
static LUA_PROTO(w_insertln)
{
	lua_pushboolean(L, winsertln(lc_checkwindow(L, 1)) != ERR);
	return 1;
}

/*
* bool window:insstr([int y, int x,] str s, [int n])
*/
static LUA_PROTO(w_insstr)
{
	WINDOW *w = lc_checkwindow(L, 1);
	const char *str;
	int n;
	if (!lc_checkmv(L, w, 0))
		return 1;
	str = luaL_checkstring(L, 2);
	n = luaL_optint(L, 3, -1);
	lua_pushboolean(L, winsnstr(w, str, n) != ERR);
	return 1;
}

/*
* str window:instr([int y, int x,] [int n])
*/
static LUA_PROTO(w_instr)
{
	WINDOW *w = lc_checkwindow(L, 1);
	char *buf;
	int n;
	if (!lc_checkmv(L, w, 1))
		return 1;
	n = luaL_optint(L, 2, -1);
	if (n < 0) {
		n = LUAL_BUFFERSIZE;
	} else if (n == 0) {
		lua_pushstring(L, "");
		return 1;
	}
	buf = malloc(n + 1);
	if (winnstr(w, buf, n) != ERR) {
		lua_pushlstring(L, buf, n);
	} else {
		lua_pushnil(L);
	}
	free(buf);
	return 1;
}

/*
* bool window:intrflush(bool ok)
* If enabled, and an interrupt key is pressed on the keyboard, all output
* in the tty driver queue will be flushed, giving the effect of faster
* response to the interrupt, but causing curses to have the wrong idea of
* what is on the screen. The default is inherited from the tty driver
* settings.
* This will affect ALL windows.
*/
static LUA_PROTO(w_intrflush)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int ok = luaL_checkbool(L, 2);
	lua_pushboolean(L, intrflush(w, ok) != ERR);
	return 1;
}

/*
* bool window:is_linetouched(int line)
* Returns true if the given line on this window was modified since the last
* call to refresh().  Returns nil if an invalid line was given.
*/
static LUA_PROTO(w_is_linetouched)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int line = luaL_checkint(L, 2);
	int rv = is_linetouched(w, line);
#ifdef LC_ERRORS
	luaL_argcheck(L, rv != ERR, 2, "invalid line");
#else
	if (rv == ERR)
		lua_pushnil(L);
	else
#endif
		lua_pushboolean(L, rv);
	return 1;
}

/*
* bool window:is_linetouched(int line)
* Returns true if the given window was modified since the last call to
* refresh().
*/
static LUA_PROTO(w_is_wintouched)
{
	WINDOW *w = lc_checkwindow(L, 1);
	lua_pushboolean(L, is_wintouched(w));
	return 1;
}

/*
* bool window:keypad(bool enable)
* Enables the keypad of the user's terminal, allowing getch() to return
* a single value representing function keys when pressed, e.g. KEY_LEFT.
* Off by default.
*/
static LUA_PROTO(w_keypad)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int enable = luaL_checkbool(L, 2);
	lua_pushboolean(L, keypad(w, enable) != ERR);
	return 1;
}

/*
* bool window:leaveok(bool ok)
* Allows the hardware cursor to be left wherever a refresh happens to leave
* it, instead of being moved back to match the window cursor. Useful when
* the cursor is not being used, since less cursor movement is needed.
*/
static LUA_PROTO(w_leaveok)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int ok = luaL_checkbool(L, 2);
	lua_pushboolean(L, leaveok(w, ok) != ERR);
	return 1;
}

/*
* bool window:meta(bool enable)
* Initially, whether the terminal returns 7 or 8 significant bits on input
* depends on the control mode of the tty driver. Use true to force 8 bits,
* or false to force 7 bits.
* This will affect ALL windows.
*/
static LUA_PROTO(w_meta)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int enable = luaL_checkbool(L, 2);
	lua_pushboolean(L, meta(w, enable) != ERR);
	return 1;
}

/*
* bool window:move(y, x)
*/
static LUA_PROTO(w_move)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int y = luaL_checkint(L, 2);
	int x = luaL_checkint(L, 3);
	lua_pushboolean(L, wmove(w, y, x) != ERR);
	return 1;
}

/*
* bool window:mvwin(y, x)
* Moves the window to the given coordinates. If the window would be moved
* offscreen, returns false and the window is not moved.
* Moving subwindows is allowed, but should be avoided.
*/
static LUA_PROTO(w_mvwin)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int y = luaL_checkint(L, 2);
	int x = luaL_checkint(L, 3);
	lua_pushboolean(L, mvwin(w, y, x) != ERR);
	return 1;
}

/*
* bool window:nodelay(bool enable)
* If enabled, getch() will not block, and will return nil if no input is
* ready. While interpreting an input escape sequence, wgetch() sets a timer
* while waiting for the next character (to differentiate between sequences
* received from a function key, and those typed by the user), unless
* notimeout() is enabled.
*/
static LUA_PROTO(w_nodelay)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int enable = luaL_checkbool(L, 2);
	lua_pushboolean(L, nodelay(w, enable) != ERR);
	return 1;
}

/*
* bool window:notimeout(bool no)
*/
static LUA_PROTO(w_notimeout)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int enable = luaL_checkbool(L, 2);
	lua_pushboolean(L, notimeout(w, enable) != ERR);
	return 1;
}

/*
* bool window:noutrefresh()
*/
static LUA_PROTO(w_noutrefresh)
{
	lua_pushboolean(L, wnoutrefresh(lc_checkwindow(L, 1)) != ERR);
	return 1;
}

/*
* bool window:overlay(window dest)
* Overlays this window on top of the destination window, except for blanks.
* Only text where the windows overlap is copied.
*/
static LUA_PROTO(w_overlay)
{
	WINDOW *w = lc_checkwindow(L, 1);
	WINDOW *dest = lc_checkwindow(L, 2);
	lua_pushboolean(L, overlay(w, dest) != ERR);
	return 1;
}

/*
* bool window:overlay(window dest)
* Overlays this window on top of the destination window, including blanks.
* Only text (or spaces) where the windows overlap is copied.
*/
static LUA_PROTO(w_overwrite)
{
	WINDOW *w = lc_checkwindow(L, 1);
	WINDOW *dest = lc_checkwindow(L, 2);
	lua_pushboolean(L, overwrite(w, dest) != ERR);
	return 1;
}

/*
* bool window:putwin(file f)
* Writes all data associated with the window to a file, which can be
* restored using curses.getwin().
*/
LUA_UNIMP(w_putwin) /* TODO */

/*
* bool window:redrawln(int beg_line, int num_lines)
*/
static LUA_PROTO(w_redrawln)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int start = luaL_checkint(L, 2);
	int lines = luaL_checkint(L, 3);
	lua_pushboolean(L, wredrawln(w, start, lines) != ERR);
	return 1;
}

/*
* bool window:redrawwin()
* Tells curses that the entire window needs to be redrawn completely.
*/
static LUA_PROTO(w_redrawwin)
{
	WINDOW *w = lc_checkwindow(L, 1);
	lua_pushboolean(L, redrawwin(w) != ERR);
	return 1;
}

/*
* bool window:refresh()
*/
static LUA_PROTO(w_refresh)
{
	lua_pushboolean(L, wrefresh(lc_checkwindow(L, 1)) != ERR);
	return 1;
}

/*
* bool window:scrl(int n)
*/
static LUA_PROTO(w_scrl)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int n = luaL_checkint(L, 2);
	lua_pushboolean(L, wscrl(w, n) != ERR);
	return 1;
}

/*
* bool window:scroll()
* Scrolls the window up one line.
*/
static LUA_PROTO(w_scroll)
{
	WINDOW *w = lc_checkwindow(L, 1);
	lua_pushboolean(L, scroll(w) != ERR);
	return 1;
}

/*
* bool window:scrollok(bool ok)
* If disabled, when the cursor is moved off the edge of the window or
* scrolling region, it is left on the bottom line. If enabled, the window
* is scrolled up one line.
*/
static LUA_PROTO(w_scrollok)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int ok = luaL_checkbool(L, 2);
	lua_pushboolean(L, scrollok(w, ok) != ERR);
	return 1;
}

/*
* bool window:setscrreg(int top, int bot)
*/
static LUA_PROTO(w_setscrreg)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int top = luaL_checkint(L, 2);
	int bot = luaL_checkint(L, 3);
	lua_pushboolean(L, wsetscrreg(w, top, bot) != ERR);
	return 1;
}

/*
* bool window:standend()
*/
static LUA_PROTO(w_standend)
{
	lua_pushboolean(L, wstandend(lc_checkwindow(L, 1)) != ERR);
	return 1;
}

/*
* bool window:standout()
*/
static LUA_PROTO(w_standout)
{
	lua_pushboolean(L, wstandout(lc_checkwindow(L, 1)) != ERR);
	return 1;
}

/*
* window window:subpad(int nlines, int ncols, int beginy, int beginx)
* Creates a subwindow within a pad. Unlike subwin(), which uses screen
* coords, the position is relative to the pad.
*/
static LUA_PROTO(w_subpad)
{
	winhandle *wh = lc_checkhandle(L, 1);
	int nlines = luaL_checkint(L, 2);
	int ncols = luaL_checkint(L, 3);
	int beginy = luaL_checkint(L, 4);
	int beginx = luaL_checkint(L, 5);

	WINDOW *sub = subpad(wh->win, nlines, ncols, beginy, beginx);
	winhandle *subh = lc_pushwindow(L, sub);
	subh->next = wh->sub;
	wh->sub = subh;

	return 1;
}

/*
* window window:subwin(int nlines, int ncols, int beginy, int beginx)
* Creates a new subwindow. The position is relative to the screen.
*/
static LUA_PROTO(w_subwin)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int nlines = luaL_checkint(L, 2);
	int ncols = luaL_checkint(L, 3);
	int beginy = luaL_checkint(L, 4);
	int beginx = luaL_checkint(L, 5);
	lc_pushwindow(L, subwin(w, nlines, ncols, beginy, beginx));
	return 1;
}

/*
* void window:syncdown()
*/
static LUA_PROTO(w_syncdown)
{
	wsyncdown(lc_checkwindow(L, 1));
	return 0;
}

/*
* bool window:syncok()
* Causes syncup() to be called automatically whenever there is a change
* in the window.
*/
static LUA_PROTO(w_syncok)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int ok = luaL_checkbool(L, 2);
	lua_pushboolean(L, syncok(w, ok) != ERR);
	return 1;
}

/*
* void window:syncup()
*/
static LUA_PROTO(w_syncup)
{
	wsyncup(lc_checkwindow(L, 1));
	return 0;
}

/*
* void window:timeout(int delay)
*/
static LUA_PROTO(w_timeout)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int delay = luaL_checkint(L, 2);
	wtimeout(w, delay);
	return 0;
}

/*
* bool window:touchline(int start, int count)
* Marks the given lines as changed and needing to be completely redrawn.
*/
static LUA_PROTO(w_touchline)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int start = luaL_checkint(L, 2);
	int count = luaL_checkint(L, 3);
	lua_pushboolean(L, touchline(w, start, count) != ERR);
	return 1;
}

/*
* bool window:touchln(int start, int nlines, bool changed)
*/
static LUA_PROTO(w_touchln)
{
	WINDOW *w = lc_checkwindow(L, 1);
	int start = luaL_checkint(L, 2);
	int nlines = luaL_checkint(L, 3);
	int changed = luaL_checkbool(L, 4);
	lua_pushboolean(L, wtouchln(w, start, nlines, changed) != ERR);
	return 1;
}

/*
* bool window:touchwin(int start, int count)
* Marks the entire window as changed and needing to be completely redrawn.
*/
static LUA_PROTO(w_touchwin)
{
	WINDOW *w = lc_checkwindow(L, 1);
	lua_pushboolean(L, touchwin(w) != ERR);
	return 1;
}

/*
* bool window:untouchwin()
* Marks the entire window as unchanged since the last refresh.
*/
static LUA_PROTO(w_untouchwin)
{
	WINDOW *w = lc_checkwindow(L, 1);
	lua_pushboolean(L, untouchwin(w) != ERR);
	return 1;
}

/*
* bool window:vline([int y, int x], [int/str ch], [int n])
*/
static LUA_PROTO(w_vline)
{
	WINDOW *w = lc_checkwindow(L, 1);
	chtype ch;
	int n;
	if (lc_checkmv(L, w, 0))
		return 1;
	ch = luaL_optint(L, 2, -1);
	n = luaL_optint(L, 3, LINES);
	lua_pushboolean(L, wvline(w, ch, n) != ERR);
	return 1;
}

#ifdef LC_WIDE
static LUA_PROTO(w_add_wch)
static LUA_PROTO(w_add_wchnstr)
static LUA_PROTO(w_add_wchstr)
static LUA_PROTO(w_addnwstr)
static LUA_PROTO(w_addwstr)
static LUA_PROTO(w_bkgrnd)
static LUA_PROTO(w_bkgrndset)
static LUA_PROTO(w_border_set)
static LUA_PROTO(w_echo_wchar)
static LUA_PROTO(w_get_wch)
static LUA_PROTO(w_get_wstr)
static LUA_PROTO(w_getbkgrnd)
static LUA_PROTO(w_getn_wstr)
static LUA_PROTO(w_hline_set)
static LUA_PROTO(w_in_wch)
static LUA_PROTO(w_in_wchnstr)
static LUA_PROTO(w_in_wchstr)
static LUA_PROTO(w_innwstr)
static LUA_PROTO(w_ins_nwstr)
static LUA_PROTO(w_ins_wch)
static LUA_PROTO(w_ins_wstr)
static LUA_PROTO(w_inwstr)
static LUA_PROTO(w_vline_set)
#endif

#ifdef LC_NCURSES
static LUA_PROTO(w_is_cleared)
static LUA_PROTO(w_is_idcok)
static LUA_PROTO(w_is_idlok)
static LUA_PROTO(w_is_immedok)
static LUA_PROTO(w_is_keypad)
static LUA_PROTO(w_is_leaveok)
static LUA_PROTO(w_is_nodelay)
static LUA_PROTO(w_is_notimeout)
static LUA_PROTO(w_is_pad)
static LUA_PROTO(w_is_scrollok)
static LUA_PROTO(w_is_subwin)
static LUA_PROTO(w_is_syncok)
static LUA_PROTO(w_enclose)
static LUA_PROTO(w_getdelay)
static LUA_PROTO(w_getparent)
static LUA_PROTO(w_getscrreg)
static LUA_PROTO(w_mouse_trafo)
static LUA_PROTO(w_resize)
#endif

static LUA_PROTO(w_isvalid)
{
	winhandle *wh = *((winhandle**)luaL_checkudata(L, 1, LC_WINDOWMT));
	lua_pushboolean(L, wh->win != NULL);
	return 1;
}

static LUA_PROTO(w___tostring)
{
	winhandle *wh = *((winhandle**)luaL_checkudata(L, 1, LC_WINDOWMT));
	if (wh->win == NULL) {
		lua_pushstring(L, "INVALID WINDOW");
	} else if (wh->win == stdscr) {
		lua_pushstring(L, "curses: stdscr");
	} else {
		lua_pushfstring(L, "curses: window %p", wh->win);
	}
	return 1;
}

static LUA_PROTO(w___eq)
{
	winhandle *wh1 = *((winhandle**)luaL_checkudata(L, 1, LC_WINDOWMT));
	winhandle *wh2 = *((winhandle**)luaL_checkudata(L, 2, LC_WINDOWMT));
	lua_pushboolean(L, wh1->win == wh2->win);
	return 1;
}

static LUA_PROTO(w___gc)
{
	winhandle *wh = *((winhandle**)luaL_checkudata(L, 1, LC_WINDOWMT));

	wh->refs -= 1;

	if (wh->refs <= 0) {
		/* delete window and subwindows */
		lc_closehandle(wh);

		/* yank from window list */
		if (wh == lc_winlist) {
			lc_winlist = wh->hnext;
		} else {
			winhandle *cur = lc_winlist;
			while (cur) {
				if (cur->hnext == wh) {
					cur->hnext = wh->hnext;
					break;
				}
				cur = cur->hnext;
			}
		}
	}
	return 0;
}

#define LCF(fn) { #fn, w_ ## fn }

static const luaL_Reg windowfuncs[] = {
	LCF(__tostring),
	LCF(__eq),
	LCF(__gc),
	LCF(isvalid),
	LCF(addch),
	LCF(addchstr),
	LCF(addstr),
	LCF(attr_get),
	LCF(attr_off),
	LCF(attr_on),
	LCF(attr_set),
	LCF(attroff),
	LCF(attron),
	LCF(attrset),
	LCF(bkgd),
	LCF(bkgdset),
	LCF(border),
	LCF(box),
	LCF(chgat),
	LCF(clear),
	LCF(clearok),
	LCF(clrtobot),
	LCF(clrtoeol),
	LCF(color_set),
	LCF(copywin),
	LCF(cursyncup),
	LCF(delch),
	LCF(deleteln),
	LCF(delwin),
	LCF(derwin),
	LCF(dupwin),
	LCF(echochar),
	LCF(erase),
	LCF(getattrs),
	LCF(getbegyx),
	LCF(getbkgd),
	LCF(getch),
	LCF(getmaxyx),
	LCF(getparyx),
	LCF(getstr),
	LCF(getyx),
	LCF(hline),
	LCF(idcok),
	LCF(idlok),
	LCF(immedok),
	LCF(inch),
	LCF(inchstr),
	LCF(insch),
	LCF(insdelln),
	LCF(insertln),
	LCF(insstr),
	LCF(instr),
	LCF(intrflush),
	LCF(is_linetouched),
	LCF(is_wintouched),
	LCF(keypad),
	LCF(leaveok),
	LCF(meta),
	LCF(move),
	LCF(mvwin),
	LCF(nodelay),
	LCF(notimeout),
	LCF(noutrefresh),
	LCF(overlay),
	LCF(overwrite),
	LCF(putwin),
	LCF(redrawln),
	LCF(redrawwin),
	LCF(refresh),
	LCF(scrl),
	LCF(scroll),
	LCF(scrollok),
	LCF(setscrreg),
	LCF(standend),
	LCF(standout),
	LCF(subpad),
	LCF(subwin),
	LCF(syncdown),
	LCF(syncok),
	LCF(syncup),
	LCF(timeout),
	LCF(touchline),
	LCF(touchln),
	LCF(touchwin),
	LCF(untouchwin),
	LCF(vline),
#ifdef LC_WIDE
	LCF(add_wch),
	LCF(add_wchstr),
	LCF(addwstr),
	LCF(bkgrnd),
	LCF(bkgrndset),
	LCF(border_set),
	LCF(wecho_wchar),
	LCF(get_wch),
	LCF(get_wstr),
	LCF(getbkgrnd),
	LCF(getn_wstr),
	LCF(hline_set),
	LCF(in_wch),
	LCF(in_wchstr),
	LCF(innwstr),
	LCF(ins_wch),
	LCF(ins_wstr),
	LCF(inwstr),
	LCF(unctrl),
	LCF(vline_set),
#endif
#ifdef LC_NCURSES
	LCF(is_cleared),
	LCF(is_idcok),
	LCF(is_idlok),
	LCF(is_immedok),
	LCF(is_keypad),
	LCF(is_leaveok),
	LCF(is_nodelay),
	LCF(is_notimeout),
	LCF(is_pad),
	LCF(is_scrollok),
	LCF(is_subwin),
	LCF(is_syncok),
	LCF(enclose),
	LCF(mouse_trafo),
	LCF(getdelay),
	LCF(getparent),
	LCF(getscrreg),
	LCF(resize),
#endif
	{ NULL, NULL }
};

void lc_reg_window(lua_State *L)
{
	luaL_newmetatable(L, LC_WINDOWMT);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	LC_REGISTER(L, windowfuncs);

	lua_setfield(L, -2, "_WINDOW");
}
