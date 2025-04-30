local sound = Sound()
sound:load("text_sound.wav")
local function text_window()
    local object = {
        tex = {},
        drawer = {},
        font = {},
        texts = {},
        timer = 0.0,
        text_texture = {},
        text_drawer = {},
        text_number = 1,
        text_count = 0,
        is_draw_all_texts = false,
        setup = function(self)
            self.tex = Texture()
            self.drawer = Draw2D(self.tex)
            self.font = Font()
            self.font:load(DEFAULT_FONT_NAME, 32)
            self.text_texture = Texture()
            self.text_drawer = Draw2D(self.text_texture)
            self.font:render_text(self.text_texture, "test", Color(1, 1, 1, 1))
            self.text_drawer.scale = self.text_texture:size()
            self.text_drawer.position = Vec2(0, -720 / 3)
            self.tex:fill_color(Color(0.2, 0.2, 1, 0.5))
            self.drawer.scale = Vec2(1280, 720 / 2)
            self.drawer.position = Vec2(0, -720 / 2)

            self.texts = { "Text is null." }
        end,
        before_len = 1,
        update = function(self)
            self.timer = self.timer + scene:delta_time() * 50

            local len = string.len(self.texts[self.text_number])

            if self.text_count ~= len then
                self.text_count = math.floor(self.timer) + 1
            end

            if keyboard.is_pressed(keyboard.SPACE) or
                mouse.is_pressed(mouse.LEFT) then
                if self.text_count == len then
                    self.text_number = self.text_number + 1
                    self.text_number = math.min(self.text_number, #self.texts)
                    self.before_len = 1
                    if self.text_number == #self.texts then
                        self.is_draw_all_texts = true
                    end
                    self.timer = 0.0
                else
                    self.text_count = len
                end
            end
            local str = string.sub(self.texts[self.text_number], 1,
                self.text_count)
            if self.text_count ~= self.before_len then
                sound:play()
                self.before_len = self.text_count
            end
            self.font:render_text(self.text_texture, str, Color(1, 1, 1, 1))
            self.text_drawer.scale = self.text_texture:size()
        end,
        draw = function(self)
            self.drawer:draw()
            self.text_drawer:draw()
        end
    }
    return object
end

return text_window
