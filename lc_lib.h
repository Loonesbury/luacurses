#ifndef LC_LIB_H
#define LC_LIB_H

#include "luacurses.h"

void lc_reg_lib(lua_State *L);

extern int lc_initonce;  /* 1 if initscr() was EVER called */
extern int lc_initcount; /* incr'd on initscr(), decr'd on endwin() */

#endif
