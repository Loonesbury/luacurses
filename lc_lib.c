#include <string.h>
#include "lc_lib.h"
#include "lc_window.h"

int lc_initonce = 0;

typedef struct constpair {
	const char *key;
	const chtype val;
} constpair;

#define LCC(x)  { #x, x },

/*
* int curses.COLOR_PAIR(int pair)
* Converts a color pair number to an attribute, where 0 <= pair <= COLOR_PAIRS.
*/
static LUA_PROTO(c_COLOR_PAIR)
{
	int n = luaL_checknumber(L, 1);
	luaL_argcheck(L, n >= 0 && n < COLOR_PAIRS, 1, "invalid pair number");
	lua_pushinteger(L, COLOR_PAIR(n));
	return 1;
}

/*
* int curses.PAIR_NUMBER(int attr)
* Extracts a color pair number from an attribute int or chtype.
*/
static LUA_PROTO(c_PAIR_NUMBER)
{
	int n = luaL_checknumber(L, 1);
	lua_pushinteger(L, PAIR_NUMBER(n));
	return 1;
}

/*
* int curses.LINES()
*/
static LUA_PROTO(c_LINES)
{
	lua_pushinteger(L, lc_initonce ? LINES : -1);
	return 1;
}

/*
* int curses.COLS()
*/
static LUA_PROTO(c_COLS)
{
	lua_pushinteger(L, lc_initonce ? COLS : -1);
	return 1;
}

/*
* int curses.COLORS()
*/
static LUA_PROTO(c_COLORS)
{
	lua_pushinteger(L, lc_initonce ? COLORS : -1);
	return 1;
}

/*
* int curses.COLOR_PAIRS()
*/
static LUA_PROTO(c_COLOR_PAIRS)
{
	lua_pushinteger(L, lc_initonce ? COLOR_PAIRS : -1);
	return 1;
}

/*
* int curses.baudrate()
* Returns terminal baud rate in bits per second, eg. 9600
*/
static LUA_PROTO(c_baudrate)
{
	lua_pushinteger(L, baudrate());
	return 1;
}

/*
* bool curses.beep()
* Sounds an audible alarm on the terminal, if possible. Otherwise, flashes the
* screen (visual bell).
*/
static LUA_PROTO(c_beep)
{
	lua_pushboolean(L, beep() != ERR);
	return 1;
}

/*
* bool curses.can_change_color()
* Returns true if the terminal supports color and can change their
* definitions.
*/
static LUA_PROTO(c_can_change_color)
{
	lua_pushboolean(L, can_change_color());
	return 1;
}

/*
* bool curses.cbreak(bool on)
* If given `nil' or `true', disables line buffering, making input immediately
* available to the program. If given 'false', re-enables line buffering.
* Returns false on failure.
*/
static LUA_PROTO(c_cbreak)
{
	int rv;
	if (luaL_checkbool(L, 1))
		rv = cbreak();
	else
		rv = nocbreak();
	lua_pushboolean(L, rv != ERR);
	return 1;
}

/*
* int r, g, b = curses.color_content(int color)
* Returns the intensity of the red, green and blue components of the given
* color, from 0 to 1000, OR nil on failure.
*/
static LUA_PROTO(c_color_content)
{
	short color = luaL_checkint(L, 1);
	short r, g, b;
#ifdef LC_ERRORS
	luaL_argcheck(L, color >= 0 && color < COLORS, 1, "invalid color number");
#endif

	if (color_content(color, &r, &g, &b) == ERR) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushinteger(L, r);
	lua_pushinteger(L, g);
	lua_pushinteger(L, b);
	return 3;
}

/*
* int curses.curs_set(int visibility)
* Sets the visibility of the cursor:
*   0 = invisible  1 = normal  2 = high-visibility
* Returns the previous visibility level, or nil if the given
* visibility is unsupported.
* NOTE: curs_set's return value is inconsistent and should not
* be relied on.
*/
static LUA_PROTO(c_curs_set)
{
	int prev = curs_set(luaL_checkint(L, 1));
	if (prev >= 0)
		lua_pushinteger(L, prev);
	else
		lua_pushnil(L);
	return 1;
}

/*
* bool curses.def_prog_mode()
* Saves the current terminal modes as the 'program' (in curses) state, for use
* by reset_prog_mode().
*/
static LUA_PROTO(c_def_prog_mode)
{
	lua_pushboolean(L, def_prog_mode() != ERR);
	return 1;
}

/*
* bool curses.def_shell_mode()
* Saves the current terminal modes as the 'shell' (not in curses) state, for use
* by reset_shell_mode().
*/
static LUA_PROTO(c_def_shell_mode)
{
	lua_pushboolean(L, def_shell_mode() != ERR);
	return 1;
}

/*
* bool curses.delay_output(int ms)
* Inserts a pause in the output for 'ms' milliseconds. This should not be used
* extensively because padding characters are used rather than a CPU pause.
*/
static LUA_PROTO(c_delay_output)
{
	int ms = luaL_checkint(L, 1);
	lua_pushboolean(L, delay_output(ms) != ERR);
	return 1;
}

/*
* bool curses.doupdate()
* Updates the terminal to reflect changes made to the virtual screen.
* Called automatically by refresh/wrefresh.
*/
static LUA_PROTO(c_doupdate)
{
	lua_pushboolean(L, doupdate() != ERR);
	return 1;
}

/*
* bool curses.echo(bool on)
* Enables or disables echo of typed characters.
*/
static LUA_PROTO(c_echo)
{
	int rv;
	if (luaL_checkbool(L, 1))
		rv = echo();
	else
		rv = noecho();
	lua_pushboolean(L, rv != ERR);
	return 1;
}

/*
* bool curses.endwin()
* Exits curses and restores the terminal to its previous state.
*/
static LUA_PROTO(c_endwin)
{
	if (lc_initonce)
		lua_pushboolean(L, endwin() != ERR);
	else
		lua_pushboolean(L, 0);
	return 1;
}

/*
* str curses.erasechar()
* Returns the user's current erase character.
*/
static LUA_PROTO(c_erasechar)
{
	char e = erasechar();
	if (e != '\0')
		lua_pushlstring(L, &e, 1);
	else
		lua_pushnil(L);
	return 1;
}

/*
* void curses.filter()
* Restricts curses to a single line. Must be called before initscr().
* Poorly-defined, may not work.
*/
static LUA_PROTO(c_filter)
{
	filter();
	return 0;
}

/*
* bool curses.flash()
* Flashes the terminal if possible. Otherwise, sounds an audible alarm on
* the terminal.
*/
static LUA_PROTO(c_flash)
{
	lua_pushboolean(L, flash() != ERR);
	return 1;
}

/*
* bool curses.flushinp()
* Throws away any typeahead that has been typed by the user but not yet read
* by the program.
*/
static LUA_PROTO(c_flushinp)
{
	lua_pushboolean(L, flushinp() != ERR);
	return 1;
}

/*
* window curses.getwin(file f)
* Reads window-related data previously stored in a file with putwin()
* and creates a new window.  It will be a top-level window/pad,
* and any cells using color pairs not created with init_pair will not
* be colored on refresh.
*/
static LUA_PROTO(c_getwin)
{
	luaL_error(L, "%s", "IMPLEMENT ME");
	return 0;
}

/*
* bool curses.halfdelay(int delay)
* Like cbreak, chars typed by the user are immediately available to the
* program, but if nothing was typed after the given delay (in tenths of
* seconds) ERR is returned. Use cbreak(false) to leave half-delay mode.
*/
static LUA_PROTO(c_halfdelay)
{
	lua_pushboolean(L, halfdelay(luaL_checkint(L, 1)) != ERR);
	return 1;
}

/*
* bool curses.has_colors()
* Returns true if the terminal supports color.
*/
static LUA_PROTO(c_has_colors)
{
	lua_pushboolean(L, has_colors());
	return 1;
}

/*
* bool curses.has_ic()
* Returns true if the terminal has insert- and delete-character capabilities.
*/
static LUA_PROTO(c_has_ic)
{
	lua_pushboolean(L, has_ic());
	return 1;
}

/*
* bool curses.has_il()
* Returns true if the terminal has insert- and delete-line capabilities, or
* can simulate them using scrolling regions.
*/
static LUA_PROTO(c_has_il)
{
	lua_pushboolean(L, has_il());
	return 1;
}

/*
* bool curses.init_color(int col, int r, int g, int b)
* Modifies the terminal's definition of a color. Returns false
* on error, or if the terminal doesn't support color.
*/
static LUA_PROTO(c_init_color) {
	short color, r, g, b;
#ifdef LC_ERRORS
	if (!can_change_color()) {
		lua_pushboolean(L, 0);
		return 1;
	}
#endif
	color = luaL_checkint(L, 1);
#ifdef LC_ERRORS
	luaL_argcheck(L, color >= 0 && color < COLORS, 1, "invalid color number");
#endif
	r = luaL_checkint(L, 2);
	g = luaL_checkint(L, 3);
	b = luaL_checkint(L, 4);
	lua_pushboolean(L, init_color(color, r, g, b) != ERR);
	return 1;
}

/*
* bool curses.init_pair(int pair, int fg, int bg)
*/
static LUA_PROTO(c_init_pair)
{
	short pair = luaL_checkint(L, 1);
	short fg = luaL_checkint(L, 2);
	short bg = luaL_checkint(L, 3);
#ifdef LC_ERRORS
	luaL_argcheck(L, pair >= 0 && pair < COLOR_PAIRS, 1, "invalid pair number");
	luaL_argcheck(L, fg >= 0 && pair < COLORS, 1, "invalid fg color");
	luaL_argcheck(L, fg >= 0 && pair < COLORS, 1, "invalid bg color");
#endif
	lua_pushboolean(L, init_pair(pair, fg, bg) != ERR);
	return 1;
}

/*
* window curses.initscr()
* Initializes curses and clears the screen. Additionally, sets ACS_* values in
* the curses table. (Some variants of curses do not initialize them until now.)
* Receives the 'curses' table as upvalue 1.
*/
static LUA_PROTO(c_initscr)
{
	const constpair acs_consts[] = {
		#include "lc_acs.h"
		{ NULL, -1 }
	};
	constpair *cp;

	if (initscr() == NULL) {
		lua_pushnil(L);
		return 1;
	}

	/* only slightly awful! */
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_getfield(L, -1, "_NOLIB");
	if (lua_toboolean(L, -1)) {
		lua_pop(L, 2);
#if LUA_VERSION_NUM >= 502
		lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
#else
		lua_pushvalue(L, LUA_GLOBALSINDEX);
#endif
		lua_pushvalue(L, -1);
	} else {
		lua_pop(L, 1);
	}

	/*
	* Once upon a time, SGI didn't initialize ACS_* constants until the first
	* initscr().  This may not still be the case, and we probably aren't on SGI,
	* but whatever, we have to do it now.
	*/
	for (cp = (constpair*)acs_consts; cp->key != NULL; cp++) {
		lua_pushinteger(L, cp->val);
		lua_setfield(L, -2, cp->key);
	}

	if (!lc_initonce) {
		lc_initonce = 1;
		lc_pushwindow(L, stdscr);

		/* XXX: this is really ugly */
		lua_pushvalue(L, lua_upvalueindex(1));
		lua_pushvalue(L, -2);
		lua_setfield(L, -2, "stdscr");
		lua_pop(L, 1);

		lua_setfield(L, -2, "stdscr");
	}

	lc_pushwindow(L, stdscr);
	return 1;
}

/*
* bool curses.isendwin()
* Returns true if refresh() has not been called since the last endwin().
*/
static LUA_PROTO(c_isendwin)
{
	lua_pushboolean(L, isendwin());
	return 1;
}

/*
* str curses.keyname(key)
* Returns the string name of the given key, or nil if invalid.
*/
static LUA_PROTO(c_keyname)
{
	int key = luaL_checkint(L, 1);
	const char *name = keyname(key);
	if (name && strcmp(name, "UNKNOWN KEY"))
		lua_pushstring(L, name);
	else
		lua_pushnil(L);
	return 1;
}

/*
* str curses.killchar()
* Returns the user's current line kill character.
*/
static LUA_PROTO(c_killchar)
{
	char kc = killchar();
	lua_pushlstring(L, &kc, 1);
	return 1;
}

/*
* str curses.longname()
* Returns a verbose descriptoon of the current terminal,
* limited to 128 chars.
*/
static LUA_PROTO(c_longname)
{
	if (!lc_initonce) {
		lua_pushstring(L, longname());
	} else {
#ifdef LC_ERRORS
		luaL_error(L, "must call initscr() or newterm()");
#else
		lua_pushnil(L);
#endif
	}
	return 1;
}

/*
* bool curses.napms(int ms)
* Sleeps for ms milliseconds.
*/
static LUA_PROTO(c_napms)
{
	lua_pushboolean(L, napms(luaL_checkint(L, 1)) != ERR);
	return 1;
}

/*
* pad curses.newpad(int nlines, int ncols)
* Creates and returns a new pad.
*/
LUA_UNIMP(c_newpad)

/*
* screen curses.newterm(str type, file outfile, file infile)
* Used for programs that output to more than one terminal, or for
* programs that may want to run a line-oriented mode if the terminal
* cannot support a screen-oriented program.
* XXX: How are we implementing outfd and infd?
*/
LUA_UNIMP(c_newterm)

/*
* window curses.newwin(int nlines, int ncols, int beginy, int beginx)
* Creates a new window of the given size, starting at the given screen coords.
*/
static LUA_PROTO(c_newwin)
{
	lc_pushwindow(L, newwin(
		luaL_checkint(L, 1),
		luaL_checkint(L, 2),
		luaL_checkint(L, 3),
		luaL_checkint(L, 4)
	));
	return 1;
}

/*
* bool curses.nl(bool enable)
* Controls whether the underlying display device translates the return key
* into newline on input, and newline into CRLF. Disable to allow curses to
* detect the return key.
*/
static LUA_PROTO(c_nl)
{
	int rv;
	if (luaL_checkbool(L, 1))
		rv = nl();
	else
		rv = nonl();
	lua_pushboolean(L, rv != ERR);
	return 1;
}

/*
* int fg, bg = curses.pair_content(int pair)
* Returns fg and bg colors for the given pair, or nil on failure.
*/
static LUA_PROTO(c_pair_content)
{
	short pair = luaL_checkint(L, 1);
	short fg = -1, bg = -1;
#ifdef LC_ERRORS
	luaL_argcheck(L, pair >= 0 && pair < COLOR_PAIRS, 1, "invalid pair number");
#endif
	if (pair_content(pair, &fg, &bg) == ERR) {
		lua_pushnil(L);
		return 1;
	}
	lua_pushinteger(L, fg);
	lua_pushinteger(L, bg);
	return 2;
}

/*
* void curses.qiflush(bool enable)
* Enables or disables the normal flush of input and output queues on INTR,
* QUIT and SUSP characters. You may want to call qiflush(false) in a signal
* handler if you want output to continue as though the interrupt had not
* occurred, after the handler exits.
*/
static LUA_PROTO(c_qiflush)
{
	if (luaL_checkbool(L, 1))
		qiflush();
	else
		noqiflush();
	return 0;
}

/*
* bool curses.raw(bool enable)
* Places the terminal into or out of raw mode. Like cbreak mode, chars typed
* are immediately available to the program, but interrupt, quit, suspend and
* flow control chars are also passed through, instead of generating a signal.
* (The behavior of the BREAK key depends on bits that are not set by curses.)
*/
static LUA_PROTO(c_raw)
{
	int rv;
	if (luaL_checkbool(L, 1))
		rv = raw();
	else
		rv = noraw();
	lua_pushboolean(L, rv != ERR);
	return 1;
}

/*
* bool curses.reset_prog_mode()
* Restores the terminal to "program" (in-curses) state.
* This is done automatically by endwin() and, after an endwin(), by
* doupdate(), and normally isn't called.
*/
static LUA_PROTO(c_reset_prog_mode)
{
	lua_pushboolean(L, reset_prog_mode() != ERR);
	return 1;
}

/*
* bool curses.reset_shell_mode()
* Restores the terminal to "shell" (out-of-curses) state.
* See above.
*/
static LUA_PROTO(c_reset_shell_mode)
{
	lua_pushboolean(L, reset_shell_mode() != ERR);
	return 1;
}

/*
* bool curses.resetty()
* Restores terminal modes saved with savetty().
*/
static LUA_PROTO(c_resetty)
{
	lua_pushboolean(L, resetty() != ERR);
	return 1;
}

/*
* bool ripoffline(int line, func oninit)
* May be used before initscr() or newterm() to reduce the size of the screen.
*/
LUA_UNIMP(c_ripoffline)

/*
* bool curses.savetty()
* Saves terminal modes to be restored with resetty().
*/
static LUA_PROTO(c_savetty)
{
	lua_pushboolean(L, savetty() != ERR);
	return 1;
}

LUA_UNIMP(c_scr_dump)
LUA_UNIMP(c_scr_init)
LUA_UNIMP(c_scr_restore)
LUA_UNIMP(c_scr_set)

/*
* bool curses.start_color()
* Must be called before using any other color functions.
* Returns true or false, or nil on error.
*/
static LUA_PROTO(c_start_color)
{
	int rv = start_color();
	if (rv != ERR)
		lua_pushboolean(L, rv);
	else
		lua_pushnil(L);
	return 1;
}

/*
* int curses.termattrs()
* Returns a logical OR of all attributes supported by the terminal.
*/
static LUA_PROTO(c_termattrs)
{
	lua_pushinteger(L, termattrs());
	return 1;
}

/*
* str curses.termname()
* Returns the terminal name used by setupterm(), or nil on error.
* Usually truncated to 14 characters.
*/
static LUA_PROTO(c_termname)
{
	const char *name = termname();
	if (name)
		lua_pushstring(L, name);
	else
		lua_pushnil(L);
	return 1;
}

/*
* bool curses.typeahead(file fd)
* TODO. How do we handle file descriptors?
*/
LUA_UNIMP(c_typeahead)

/*
* str curses.unctrl(int/char c)
* Returns the printable representation of the given char, ignoring attributes.
*/
static LUA_PROTO(c_unctrl)
{
	const char *rep;
	chtype ch;
	if (lua_isnumber(L, 2))
		ch = lua_tointeger(L, 2);
	else
		ch = *luaL_checkstring(L, 2);
	rep = unctrl(ch);
	if (rep)
		lua_pushstring(L, rep);
	else
		lua_pushnil(L);
	return 1;
}

/*
* bool curses.ungetch(int/char c)
* Places a character back onto the input queue. This is shared by all windows.
*/
static LUA_PROTO(c_ungetch)
{
	int ch;
	if (lua_isnumber(L, 2))
		ch = lua_tointeger(L, 2);
	else
		ch = *luaL_checkstring(L, 2);
	lua_pushboolean(L, ungetch(ch) != ERR);
	return 1;
}

/*
* void curses.use_env(bool f)
* Modifies how curses treats environment variables when determining
* screen size.
*/
static LUA_PROTO(c_use_env)
{
	use_env(luaL_checkbool(L, 1));
	return 0;
}

#ifdef LC_WIDE
static LUA_PROTO(c_erasewchar)
static LUA_PROTO(c_getcchar)
static LUA_PROTO(c_key_name)
static LUA_PROTO(c_killwchar)
static LUA_PROTO(c_setcchar)
static LUA_PROTO(c_unget_wch)
static LUA_PROTO(c_wunctrl)
#endif

#ifdef LC_NCURSES
static LUA_PROTO(c_alloc_pair)
static LUA_PROTO(c_assume_default_colors)
static LUA_PROTO(c_curses_version)
static LUA_PROTO(c_define_key)
static LUA_PROTO(c_extended_color_content)
static LUA_PROTO(c_extended_pair_content)
static LUA_PROTO(c_extended_slk_color)
static LUA_PROTO(c_find_pair)
static LUA_PROTO(c_free_pair)
static LUA_PROTO(c_getmouse)
static LUA_PROTO(c_getsyx)
static LUA_PROTO(c_has_key)
static LUA_PROTO(c_init_extended_color)
static LUA_PROTO(c_init_extended_pair)
static LUA_PROTO(c_is_term_resized)
static LUA_PROTO(c_key_defined)
static LUA_PROTO(c_keybound)
static LUA_PROTO(c_keyok)
static LUA_PROTO(c_mcprint)
static LUA_PROTO(c_mouseinterval)
static LUA_PROTO(c_mousemask)
static LUA_PROTO(c_reset_color_pairs)
static LUA_PROTO(c_resize_term)
static LUA_PROTO(c_resizeterm)
static LUA_PROTO(c_setsyx)
static LUA_PROTO(c_ungetmouse)
static LUA_PROTO(c_use_default_colors)
static LUA_PROTO(c_use_extended_names)
static LUA_PROTO(c_use_legacy_coding)
static LUA_PROTO(c_use_tioctl)
#endif

#define LCF(fn) { #fn, c_ ## fn }

static const luaL_Reg libfuncs[] = {
	LCF(COLOR_PAIR),
	LCF(COLOR_PAIRS),
	LCF(COLORS),
	LCF(COLS),
	LCF(LINES),
	LCF(PAIR_NUMBER),
	LCF(baudrate),
	LCF(beep),
	LCF(can_change_color),
	LCF(cbreak),
	LCF(color_content),
	LCF(curs_set),
	LCF(def_prog_mode),
	LCF(def_shell_mode),
	LCF(delay_output),
	LCF(doupdate),
	LCF(echo),
	LCF(endwin),
	LCF(erasechar),
	LCF(filter),
	LCF(flash),
	LCF(flushinp),
	LCF(getwin),
	LCF(halfdelay),
	LCF(has_colors),
	LCF(has_ic),
	LCF(has_il),
	LCF(init_color),
	LCF(init_pair),
	LCF(isendwin),
	LCF(keyname),
	LCF(killchar),
	LCF(longname),
	LCF(napms),
	LCF(newpad),
	LCF(newterm),
	LCF(newwin),
	LCF(nl),
	LCF(pair_content),
	LCF(qiflush),
	LCF(raw),
	LCF(reset_prog_mode),
	LCF(reset_shell_mode),
	LCF(resetty),
	LCF(ripoffline),
	LCF(savetty),
	LCF(scr_dump),
	LCF(scr_init),
	LCF(scr_restore),
	LCF(scr_set),
	LCF(start_color),
	LCF(termattrs),
	LCF(termname),
	LCF(typeahead),
	LCF(unctrl),
	LCF(ungetch),
	LCF(use_env),
#ifdef LC_WIDE
	LCF(erasewchar),
	LCF(getcchar),
	LCF(killwchar),
	LCF(setcchar),
	LCF(term_attrs),
	LCF(unget_wch),
#endif
#ifdef LC_NCURSES
	LCF(alloc_pair),
	LCF(assume_default_colors),
	LCF(curses_version),
	LCF(extended_color_content),
	LCF(extended_pair_content),
	LCF(extended_slk_color),
	LCF(define_key),
	LCF(find_pair),
	LCF(free_pair),
	LCF(getmouse),
	LCF(getsyx),
	LCF(has_key),
	LCF(init_extended_color),
	LCF(init_extended_pair),
	LCF(is_term_resized),
	LCF(key_defined),
	LCF(key_name),
	LCF(keybound),
	LCF(keyok),
	LCF(mcprint),
	LCF(mouseinterval),
	LCF(mousemask),
	LCF(reset_color_pairs),
	LCF(resize_term),
	LCF(resizeterm),
	LCF(setsyx),
	LCF(ungetmouse),
	LCF(use_default_colors),
	LCF(use_extended_names),
	LCF(use_legacy_coding),
	LCF(use_tioctl),
#endif

	{ NULL, NULL }
};

void lc_reg_lib(lua_State *L)
{
	const constpair consts[] = {
		#include "lc_const.h"
		{ NULL, -1 }
	};
	constpair *cp = (constpair*)consts;
	int key;

	/* load constants */
	while (cp->key) {
		lua_pushstring(L, cp->key);
		lua_pushinteger(L, cp->val);
		lua_rawset(L, -3);
		cp++;
	}
	for (key = KEY_MIN; key <= KEY_MAX; key++) {
		const char *kname;

		if (key == KEY_F0) {
			while (key < KEY_F0 + 64) {
				lua_pushfstring(L, "KEY_F%d", key - KEY_F0);
				lua_pushinteger(L, key);
				lua_settable(L, -3);
				key++;
			}
			continue;
		}

		kname = keyname(key);

		if (!kname || !strcmp(kname, "UNKNOWN KEY"))
			continue;

		lua_pushinteger(L, key);
		lua_setfield(L, -2, kname);
	}

	/* load curses.* funcs */
	LC_REGISTER(L, libfuncs);

	/* initscr needs the lib as an upvalue to set ACS constants */
	lua_pushvalue(L, -1);
	lua_pushcclosure(L, c_initscr, 1);
	lua_setfield(L, -2, "initscr");
}
