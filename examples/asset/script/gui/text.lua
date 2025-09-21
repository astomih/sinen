local function text()
    local object = {
        font = {},
        drawer = {},
        color = sn.Color(1, 1, 1, 0.9),
        show = function(self, text, pos, scale)
            local texture = GUI_MANAGER:get_texture()
            self.drawer = sn.Draw2D(texture)
            self.font:RenderText(texture, text, self.color)
            self.drawer.scale = texture:Size()
            self.drawer.position = pos
            GUI_MANAGER:add(self.drawer)
        end,
    }
    object.font = sn.Font()
    object.font:Load(32)
    object.drawer = sn.Draw2D(sn.Texture())

    return object
end

return text
