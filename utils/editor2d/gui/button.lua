---@diagnostic disable: duplicate-set-field, duplicate-doc-field
---@class Button
---@field font sn.Font
---@field bg_color sn.Color
local Button = {}

Button.__index = Button

Button.new = function()
    local self = setmetatable({}, Button)
    self.font = sn.Font.new()
    self.font:load(18)
    self.bg_color = sn.Color.new(0.2, 0.2, 0.2, 0.9)
    return self
end

Button.show = function(self, text, pos, scale)
    local mpos = sn.Mouse.getPositionOnScene()
    local hovered = mpos.x >= pos.x - scale.x / 2 and mpos.x <= pos.x + scale.x / 2 and mpos.y >= pos.y - scale.y / 2 and
        mpos.y <= pos.y + scale.y / 2

    local bg = self.bg_color
    if hovered then
        bg = sn.Color.new(0.3, 0.3, 0.3, 0.9)
    end

    GUI_MANAGER:add({
        draw = function()
            sn.Graphics.drawRect(sn.Rect.new(pos, scale), bg)
        end
    })
    GUI_MANAGER:add({
        draw = function()
            sn.Graphics.drawText(text, self.font, pos, sn.Color.new(1), 18)
        end
    })

    if hovered and sn.Mouse.isPressed(sn.Mouse.LEFT) then
        return true
    end
    return false
end

return Button
