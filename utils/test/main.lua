if os.getenv("LOCAL_LUA_DEBUGGER_VSCODE") == "1" then
  require("lldebugger").start()
end

local v = sn.Vec2.new(0, 0)

sn.Logger.info(v)


function Update()
end

function Draw()
end
