#ifndef LC_CHSTR_H
#define LC_CHSTR_H

#include "luacurses.h"

#define LC_CHSTRMT "lc-chstr"

typedef struct chstr {
	size_t len;
	chtype str[1];
} chstr;

void lc_reg_chstr(lua_State *L);
chstr* lc_pushchstr(lua_State *L, int len);
chstr* lc_checkchstr(lua_State *L, int narg);

#endif
