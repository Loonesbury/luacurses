#ifndef LC_WINDOW_H
#define LC_WINDOW_H

#include "luacurses.h"

#define LC_WINDOWMT "lc-window"

typedef struct winhandle {
  struct winhandle *parent, *sub, *next, *hnext;
  WINDOW *win;
  PANEL *pan;
  int refs;
} winhandle;

extern winhandle *lc_winlist;

void lc_reg_window(lua_State *L);

/* pushes the existing handle for `w', or a new one if it doesn't exist */
winhandle* lc_pushwindow(lua_State *L, WINDOW *w);

/* pushes a reference to an existing winhandle */
winhandle* lc_pushhandle(lua_State *L, winhandle *wh);

/* returns the handle for the given window, or NULL if it doesn't exist */
winhandle* lc_findwindow(lua_State *L, WINDOW *w);

winhandle* lc_checkhandle(lua_State *L, int narg);
WINDOW*    lc_checkwindow(lua_State *L, int narg);

/* if first 2 args are numbers, pops them and does a wmove() */
/* if wmove() fails, pushes 'false' (or 'nil' if pushnil=1) and returns 0 */
int lc_checkmv(lua_State *L, WINDOW *w, int pushnil);

int lc_close_handle(winhandle *wh, int top);

/* calls delwin() on the handle's window, and all of its subwindows */
/* (without freeing the handle itself) */
#define lc_closehandle(wh) (lc_close_handle(wh, 1))

#endif
