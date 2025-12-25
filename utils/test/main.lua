if os.getenv("LOCAL_LUA_DEBUGGER_VSCODE") == "1" then
  require("lldebugger").start()
end

local v = sn.Transform.new()

sn.Logger.info(v)


function Update()
end

function Draw()
  sn.ImGui.Begin("Hello ImGui")
  sn.ImGui.End()
end
