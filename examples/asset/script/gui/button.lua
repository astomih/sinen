local button = function()
    local _       = {}

    _.font        = ({
        Init = function()
            local f = sn.Font.new()
            f:load(32)
            return f
        end
    }).Init()

    _.back_drawer = {}
    _.drawer      = {}
    _.bg_color    = sn.Color.new(0.7, 0.2, 0.2, 1)
    _.fg_color    = sn.Color.new(1, 1, 1, 0.9)

    _.show        = function(self, text, pos, scale)
        self.font:resize(32)
        local back_texture = GUI_MANAGER:get_texture()
        local texture = GUI_MANAGER:get_texture()
        back_texture:fill(self.bg_color)
        self.back_drawer = sn.Draw2D.new(back_texture)
        self.back_drawer.position = pos
        self.back_drawer.scale = sn.Vec2.new(scale.x, scale.y)
        GUI_MANAGER:add(self.back_drawer)

        self.drawer = sn.Draw2D.new(texture)
        self.font:renderText(texture, text, self.fg_color)
        self.drawer.scale = texture:size()
        self.drawer.position = pos
        GUI_MANAGER:add(self.drawer)

        -- Mouse in _?
        local mpos = sn.Mouse.getPositionOnScene()
        if mpos.x >= pos.x - scale.x / 2
            and
            mpos.x <= pos.x + scale.x / 2
            and
            mpos.y >= pos.y - scale.y / 2
            and
            mpos.y <= pos.y + scale.y / 2
        then
            back_texture:fill(sn.Color.new(0.2, 0.7, 0.2, 1))
            if sn.Mouse.isPressed(sn.Mouse.LEFT) then
                back_texture:fill(sn.Color.new(0.2, 0.2, 0.7, 1))
                return true
            end
        else
            back_texture:fill(self.bg_color)
        end

        return false
    end
    return _
end

return button
