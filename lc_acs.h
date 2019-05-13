/* VT100-compatible symbols -- box chars */

  LCC(ACS_ULCORNER)
  LCC(ACS_LLCORNER)
  LCC(ACS_URCORNER)
  LCC(ACS_LRCORNER)
  LCC(ACS_RTEE)
  LCC(ACS_LTEE)
  LCC(ACS_BTEE)
  LCC(ACS_TTEE)
  LCC(ACS_HLINE)
  LCC(ACS_VLINE)
  LCC(ACS_PLUS)

/* VT100-compatible symbols -- other */

  LCC(ACS_S1)
  LCC(ACS_S9)
  LCC(ACS_DIAMOND)
  LCC(ACS_CKBOARD)
  LCC(ACS_DEGREE)
  LCC(ACS_PLMINUS)
  LCC(ACS_BULLET)

/* Teletype 5410v1 symbols -- these are defined in SysV curses, but
   are not well-supported by most terminals. Stick to VT100 characters
   for optimum portability. */

  LCC(ACS_LARROW)
  LCC(ACS_RARROW)
  LCC(ACS_DARROW)
  LCC(ACS_UARROW)
  LCC(ACS_BOARD)
  LCC(ACS_LANTERN)
  LCC(ACS_BLOCK)

/* That goes double for these -- undocumented SysV symbols. Don't use
   them. */

  LCC(ACS_S3)
  LCC(ACS_S7)
  LCC(ACS_LEQUAL)
  LCC(ACS_GEQUAL)
  LCC(ACS_PI)
  LCC(ACS_NEQUAL)
  LCC(ACS_STERLING)

/* Box char aliases */

  LCC(ACS_BSSB)
  LCC(ACS_SSBB)
  LCC(ACS_BBSS)
  LCC(ACS_SBBS)
  LCC(ACS_SBSS)
  LCC(ACS_SSSB)
  LCC(ACS_SSBS)
  LCC(ACS_BSSS)
  LCC(ACS_BSBS)
  LCC(ACS_SBSB)
  LCC(ACS_SSSS)

/* cchar_t aliases */

#ifdef PDC_WIDE
  LCC(WACS_ULCORNER)
  LCC(WACS_LLCORNER)
  LCC(WACS_URCORNER)
  LCC(WACS_LRCORNER)
  LCC(WACS_RTEE)
  LCC(WACS_LTEE)
  LCC(WACS_BTEE)
  LCC(WACS_TTEE)
  LCC(WACS_HLINE)
  LCC(WACS_VLINE)
  LCC(WACS_PLUS)

  LCC(WACS_S1)
  LCC(WACS_S9)
  LCC(WACS_DIAMOND)
  LCC(WACS_CKBOARD)
  LCC(WACS_DEGREE)
  LCC(WACS_PLMINUS)
  LCC(WACS_BULLET)

  LCC(WACS_LARROW)
  LCC(WACS_RARROW)
  LCC(WACS_DARROW)
  LCC(WACS_UARROW)
  LCC(WACS_BOARD)
  LCC(WACS_LANTERN)
  LCC(WACS_BLOCK)

  LCC(WACS_S3)
  LCC(WACS_S7)
  LCC(WACS_LEQUAL)
  LCC(WACS_GEQUAL)
  LCC(WACS_PI)
  LCC(WACS_NEQUAL)
  LCC(WACS_STERLING)

  LCC(WACS_BSSB)
  LCC(WACS_SSBB)
  LCC(WACS_BBSS)
  LCC(WACS_SBBS)
  LCC(WACS_SBSS)
  LCC(WACS_SSSB)
  LCC(WACS_SSBS)
  LCC(WACS_BSSS)
  LCC(WACS_BSBS)
  LCC(WACS_SBSB)
  LCC(WACS_SSSS)
#endif