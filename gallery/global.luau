local sn = require("@sinen")
local GUIManager = require("./gui/gui_manager")
local G = {}
G.font = sn.Font.new()
G.font:load(24)
G.guiManager = GUIManager.new()
G.toScreen = function(pos: sn.Vec2): sn.Vec2
	local half = sn.Graphics.getCamera2D():half()
	return sn.Vec2.new(half.x + pos.x, half.y - pos.y)
end
G.rectFromCenter = function(pos: sn.Vec2, size: sn.Vec2): sn.Rect
	return sn.Rect.new(sn.Pivot.Center, G.toScreen(pos), size)
end
G.textTopLeftFromCenter = function(text: string, font: sn.Font, pos: sn.Vec2, fontSize: number): sn.Vec2
	return font:region(text, fontSize, sn.Pivot.Center, G.toScreen(pos)):topLeft()
end
return G
