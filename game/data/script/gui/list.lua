local sound = sound()
sound:load("text_sound.wav")
local function list()
    local object = {
        num = 5,
        list_space = 100,
        scale = vector2(scene.size().x / 2, 50),
        start_pos = vector2(0, 0),
        tex = {},
        drawers = {},
        font = {},
        texts = {},
        text_textures = {},
        text_drawers = {},
        select_frame_tex = {},
        select_frame_drawer = {},
        selecting = false,
        selecting_index = 0,
        hide = false,
        setup = function(self)
            self.tex = texture()
            self.font = font()
            self.font:load(DEFAULT_FONT_NAME, 32)
            self.start_pos.y = (self.scale.y + self.list_space) * self.num / 4
            for i = 1, self.num do
                for i = #self.texts + 1, self.num do
                    self.texts[i] = ""
                    if string.len(self.texts[i]) == 0 then
                        self.texts[i] = "Text is null."
                    end
                end

                local drawer = drawui(self.tex)
                table.insert(self.drawers, drawer)
                local text_texture = texture()
                self.text_drawers[i] = drawui(text_texture)
                self.font:render_text(text_texture, self.texts[i],
                    color(0, 0, 0, 1))
                self.text_drawers[i].scale = text_texture:size()
                self.drawers[i].scale = self.scale
                self.drawers[i].position =
                    vector2(self.start_pos.x,
                        self.start_pos.y + (i - 1) * -self.list_space)
                self.text_drawers[i].position = self.drawers[i].position
            end
            self.tex:fill_color(color(0.7, 0.7, 0.7, 0.8))
            self.select_frame_tex = texture()
            self.select_frame_tex:fill_color(color(1, 0.3, 0.3, 0.8))
            self.select_frame_drawer = drawui(self.select_frame_tex)
            self.select_frame_drawer.scale = self.drawers[1].scale
            self.select_frame_drawer.position = self.drawers[1].position
        end,
        before_len = 1,
        update = function(self)
            local mpos = mouse.position_on_scene()
            mpos.x = mpos.x - scene.center().x
            mpos.y = -(mpos.y - scene.center().y)
            self.selecting_index = 0
            self.selecting = false
            for i = 1, self.num do
                local scale = self.drawers[i].scale
                local pos = self.drawers[i].position
                if mpos.x >= pos.x - scale.x / 2
                    and
                    mpos.x <= pos.x + scale.x / 2
                    and
                    mpos.y >= pos.y - scale.y / 2
                    and
                    mpos.y <= pos.y + scale.y / 2
                then
                    self.selecting_index = i
                    self.selecting = true
                end
            end
        end,
        draw = function(self)
            if not self.hide then
                if self.selecting then
                    self.select_frame_drawer.position =
                        self.drawers[self.selecting_index].position
                    self.select_frame_drawer:draw()
                end
                for i = 1, self.num do
                    self.drawers[i]:draw()
                    self.text_drawers[i]:draw()
                end
            end
        end
    }
    return object
end

return list
