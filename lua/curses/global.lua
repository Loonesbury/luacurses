local curses = require("luacurses")

-- dump lib funcs into global
for k, v in pairs(curses) do
  if k:sub(1, 1) ~= "_" then
    _G[k] = v
  end
end

-- tell C-side to use _G instead of curses
curses._NOLIB = true

return curses
