local sound = sound()
sound:load("text_sound.wav")
local function gui_list()
    local object = {
        num = 5,
        list_space = 100,
        scale = vector2(window.size().x / 2, 50),
        start_pos = vector2(0, 0),

        tex = {},
        drawers = {},
        font = {},
        texts = {},
        text_textures = {},
        text_drawers = {},

        select_frame_tex = {},
        select_frame_drawer = {},
        selecting_index = 1,

        hide = false,
        selected_index = 0,
        setup = function(self)
            self.tex = texture()
            self.font = font()
            self.font:load("x16y32pxGridGazer.ttf", 32)
            self.start_pos.y = (self.scale.y + self.list_space) * self.num / 4
            for i = 1, self.num do
                for i = #self.texts + 1, self.num do
                    self.texts[i] = ""
                    if string.len(self.texts[i]) == 0 then
                        self.texts[i] = "Text is null."
                    end
                end

                local drawer = draw2d(self.tex)
                table.insert(self.drawers, drawer)
                local text_texture = texture()
                self.text_drawers[i] = draw2d(text_texture)
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
            self.select_frame_tex:fill_color(color(1, 0.5, 0.5, 1.0))
            self.select_frame_drawer = draw2d(self.select_frame_tex)
            self.select_frame_drawer.scale = self.drawers[1].scale
            self.select_frame_drawer.position = self.drawers[1].position
        end,
        before_len = 1,
        update = function(self)
            if keyboard:key_state(keyDOWN) == buttonPRESSED then
                self.selecting_index = self.selecting_index + 1
                if self.selecting_index > self.num then
                    self.selecting_index = 1
                end
            end
            if keyboard:key_state(keyUP) == buttonPRESSED then
                self.selecting_index = self.selecting_index - 1
                if self.selecting_index < 1 then
                    self.selecting_index = self.num
                end
            end
            self.select_frame_drawer.position =
            self.drawers[self.selecting_index].position
            if keyboard:key_state(keyENTER) == buttonPRESSED then
                self.selected_index = self.selecting_index
            else
                self.selected_index = 0
            end

            local mpos = mouse:position()
            mpos.x = mpos.x - 1280 / 2
            mpos.y = mpos.y - 720 / 2
            for i = 1, self.num do
                if mpos.x > self.drawers[i].position.x - self.drawers[i].scale.x /
                    2 and mpos.x < self.drawers[i].position.x +
                    self.drawers[i].scale.x / 2 then
                    if mpos.y > self.drawers[i].position.y -
                        self.drawers[i].scale.y / 2 and mpos.y <
                        self.drawers[i].position.y + self.drawers[i].scale.y / 2 then
                        self.selecting_index = self.num - i + 1
                    end

                end
            end
        end,
        draw = function(self)
            if not self.hide then
                self.select_frame_drawer:draw()
                for i = 1, self.num do
                    self.drawers[i]:draw()
                    self.text_drawers[i]:draw()
                end
            end
        end

    }
    return object
end

return gui_list
