---@diagnostic disable: duplicate-type
---@class Button
---@field font sn.Font
---@field bg_color sn.Color
local Button = {}

Button.__index = Button

Button.new = function()
    local self = setmetatable({}, Button)
    self.font = sn.Font.new()
    self.font:load(24)
    self.bg_color = sn.Color.new(0.7, 0.2, 0.2, 1.0)
    return self
end

Button.show = function(self, text, pos, scale)
    -- Mouse in _?
    local mpos = sn.Mouse.getPositionOnScene()
    if mpos.x >= pos.x - scale.x / 2 and mpos.x <= pos.x + scale.x / 2 and mpos.y >= pos.y - scale.y / 2 and mpos.y <=
        pos.y + scale.y / 2 then
        GUI_MANAGER:add({
            draw = function()
                sn.Graphics.drawRect(sn.Rect.new(pos, scale), sn.Color.new(0.2, 0.7, 0.2, 1))
            end
        })
        if sn.Mouse.isPressed(sn.Mouse.LEFT) then
            GUI_MANAGER:add({
                draw = function()
                    sn.Graphics.drawRect(sn.Rect.new(pos, scale), sn.Color.new(0.2, 0.2, 0.7, 1))
                end
            })
            return true
        end
    else
        GUI_MANAGER:add({
            draw = function()
                sn.Graphics.drawRect(sn.Rect.new(pos, scale), self.bg_color)
            end
        })
    end
    GUI_MANAGER:add({
        draw = function()
            sn.Graphics.drawText(text, self.font, pos, sn.Color.new(1), 24)
        end
    })

    return false
end

return Button
