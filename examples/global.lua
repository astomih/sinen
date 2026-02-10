-- if os.getenv("LOCAL_LUA_DEBUGGER_VSCODE") == "1" then
--   require("lldebugger").start()
-- end

FONT = sn.Font.new()
FONT:load(24)
GUI_MANAGER = require("gui/gui_manager").new()
