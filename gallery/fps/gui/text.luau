local sn = require("@sinen")
local G = require("../global")
local text = {}
text.__index = text
text.new = function()
	return setmetatable({}, text)
end

text.show = function(self, text, pos, scale)
	G.GUI_MANAGER:add({
		draw = function()
			sn.Graphics.drawText(text, G.FONT_DEFAULT32, pos, sn.Color.new(1), 24)
		end,
	})

	return false
end

return text
