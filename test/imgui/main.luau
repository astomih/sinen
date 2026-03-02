local sn = require("@sinen")
function draw()
	sn.ImGui.setNextWindowSize(sn.Vec2.new(500, 500))
	sn.ImGui.Begin("Hello ImGui Test")
	sn.ImGui.text("Hello, world!")
	sn.ImGui.End()
end
