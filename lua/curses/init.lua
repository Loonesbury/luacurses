local curses = require("curses.core")
local window = curses._WINDOW

local failsafe
if _VERSION > "Lua 5.1" then
  failsafe = setmetatable({}, { __gc = curses.endwin })
elseif _VERSION == "Lua 5.1" then
  failsafe = debug.setmetatable(newproxy(), { __gc = curses.endwin })
end

function window:scanw(y, x, fmt)
  error("IMPLEMENT ME")
end

function window:printw(...)
  local fmt, i
  local args = {...}

  if type(args[1]) == "number" and type(args[2]) == "number" then
    self:move(args[1], args[2])
    fmt = args[3]
    i = 4
  else
    fmt = args[1]
    i = 2
  end

  self:addstr(string.format(fmt, select(i, ...)))
end

function curses.addch(...) return curses.stdscr:addch(...) end
-- function curses.addchstr(...) return curses.stdscr:addchstr(...) end
function curses.addstr(...) return curses.stdscr:addstr(...) end
function curses.attr_get(...) return curses.stdscr:attr_get(...) end
function curses.attr_off(...) return curses.stdscr:attr_off(...) end
function curses.attr_on(...) return curses.stdscr:attr_on(...) end
function curses.attr_set(...) return curses.stdscr:attr_set(...) end
function curses.attroff(...) return curses.stdscr:attroff(...) end
function curses.attron(...) return curses.stdscr:attron(...) end
function curses.attrset(...) return curses.stdscr:attrset(...) end
function curses.bkgd(...) return curses.stdscr:bkgd(...) end
function curses.bkgdset(...) return curses.stdscr:bkgdset(...) end
function curses.border(...) return curses.stdscr:border(...) end
function curses.box(...) return curses.stdscr:box(...) end
function curses.chgat(...) return curses.stdscr:chgat(...) end
function curses.clear(...) return curses.stdscr:clear(...) end
function curses.clrtobot(...) return curses.stdscr:clrtobot(...) end
function curses.clrtoeol(...) return curses.stdscr:clrtoeol(...) end
function curses.delch(...) return curses.stdscr:delch(...) end
function curses.deleteln(...) return curses.stdscr:deleteln(...) end
function curses.echochar(...) return curses.stdscr:echochar(...) end
function curses.erase(...) return curses.stdscr:erase(...) end
function curses.getch(...) return curses.stdscr:getch(...) end
function curses.getstr(...) return curses.stdscr:getstr(...) end
function curses.hline(...) return curses.stdscr:hline(...) end
function curses.hline_set(...) return curses.stdscr:hline_set(...) end
function curses.inch(...) return curses.stdscr:inch(...) end
-- function curses.inchstr(...) return curses.stdscr:inchstr(...) end
function curses.insch(...) return curses.stdscr:insch(...) end
function curses.insdelln(...) return curses.stdscr:insdelln(...) end
function curses.insertln(...) return curses.stdscr:insertln(...) end
function curses.insstr(...) return curses.stdscr:insstr(...) end
function curses.instr(...) return curses.stdscr:instr(...) end
function curses.keypad(...) return curses.stdscr:keypad(...) end
function curses.move(...) return curses.stdscr:move(...) end
function curses.printw(...) return curses.stdscr:printw(...) end
function curses.refresh() return curses.stdscr:refresh() end
function curses.scanw(...) return curses.stdscr:scanw(...) end
function curses.scrl(...) return curses.stdscr:scrl(...) end
function curses.setscrreg(...) return curses.stdscr:setscrreg(...) end
function curses.standend(...) return curses.stdscr:standend(...) end
function curses.standout(...) return curses.stdscr:standout(...) end
function curses.timeout(...) return curses.stdscr:timeout(...) end
function curses.vline(...) return curses.stdscr:vline(...) end

-- lcurses compatibility
curses.version = curses._VERSION

return curses
