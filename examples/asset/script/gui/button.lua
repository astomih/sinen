require("gui/global")

local button = function()
    local _       = {}

    _.font        = ({
        Init = function()
            local f = sn.Font()
            f:Load(32)
            return f
        end
    }).Init()

    _.back_drawer = {}
    _.drawer      = {}
    _.bg_color    = sn.Color(0.7, 0.2, 0.2, 1)
    _.fg_color    = sn.Color(1, 1, 1, 0.9)

    _.show        = function(self, text, pos, scale)
        self.font:Resize(32 * UI_SCALE)
        local back_texture = GUI_MANAGER:get_texture()
        local texture = GUI_MANAGER:get_texture()
        back_texture:FillColor(self.bg_color)
        self.back_drawer = sn.Draw2D(back_texture)
        self.back_drawer.position = pos
        self.back_drawer.scale = sn.Vec2(scale.x * UI_SCALE, scale.y * UI_SCALE)
        GUI_MANAGER:add(self.back_drawer)

        self.drawer = sn.Draw2D(texture)
        self.font:RenderText(texture, text, self.fg_color)
        self.drawer.scale = texture:Size()
        self.drawer.position = pos
        GUI_MANAGER:add(self.drawer)

        -- Mouse in _?
        local mpos = sn.Mouse.GetPositionOnScene()
        if mpos.x >= pos.x - scale.x / 2
            and
            mpos.x <= pos.x + scale.x / 2
            and
            mpos.y >= pos.y - scale.y / 2
            and
            mpos.y <= pos.y + scale.y / 2
        then
            back_texture:FillColor(sn.Color(0.2, 0.7, 0.2, 1))
            if sn.Mouse.IsPressed(sn.Mouse.LEFT) then
                back_texture:FillColor(sn.Color(0.2, 0.2, 0.7, 1))
                return true
            end
        else
            back_texture:FillColor(self.bg_color)
        end

        return false
    end
    return _
end

return button
