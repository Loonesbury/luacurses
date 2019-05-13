A slapdash wrapper around the curses and panels libraries.
Currently uses no ncurses or pdcurses extensions.

QUICK NOTES:

* Standard funcs and macros like initscr() are stored in 'curses'.

* Global variables like LINES are accessed through matching functions.

* Window funcs like waddstr() are now window methods minus the 'w' prefix.
  Stdscr functions like refresh() are stored in 'curses'.

* All 'mv' and 'n' prefixed funcs are removed and replaced by optional args to
  the normal equivalents, e.g. for wmvaddnstr:

  window:addstr(str)
  window:addstr(str, n)
  window:addstr(y, x, str)
  window:addstr(y, x, str, n)

* Panels are not separate objects. Instead they're implemented as window
  methods, and win:panel_above() etc returns another window object. This
  means you can't share a window between several panels, but that doesn't
  really work anyway. Email me if you think this is unreasonable.
