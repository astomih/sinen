DEFAULT_FONT64 = sn.Font.new()
DEFAULT_FONT64:load(64)

---@class Text
---@field text string
---@field font sn.Font
---@field color sn.Color
---@field size number
local Text = {}
Text.__index = Text

Text.new = function()
    local self = setmetatable({}, Text)
    self.text = ""
    self.color = sn.Color.new(1)
    self.size = 64
    return Text
end

local text = Text.new()
text.text = "Editor"
text.font = DEFAULT_FONT64

function Update()
end

function Draw()
    sn.Graphics.drawText(text.text, text.font, sn.Vec2.new(0), text.color, text.size)
end
