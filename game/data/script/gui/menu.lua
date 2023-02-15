local button = require("gui/button")()
button.fg_color = color(0, 0, 0, 0.9)
button.bg_color = color(1, 1, 1, 1)
local text = require("gui/text")()
local image = require("gui/image")()
local scroll = require("gui/scroll")()
local function menu()
    local object = {
        hide = true,
        hide_next = false,
        option = false,
        option_window_size = false,
        setup = function(self)
        end,
        update = function(self)
            if self.hide_next then
                self.hide = not self.hide
                self.hide_next = false
                mouse.hide_cursor(false)
                self.option = false
            end
            if keyboard.is_pressed(keyboard.ESCAPE) then
                self.hide_next = true
            end
            if self.option_window_size then
                local texture = GUI_MANAGER:get_texture()
                texture:fill_color(color(1, 1, 1, 0.5))
                image:show(texture, vector2(0, 0), vector2(1000, 600))
                text:show("Window Size", vector2(0, 200), 50)
                scroll:show(vector2(200, -200), vector2(20, 200))
                local offset = scroll.pos.y
                if button:show("800x600", vector2(0, 100 + offset), vector2(200, 50)) then
                    window.resize(vector2(800, 600))
                end
                if button:show("1024x768", vector2(0, 0 + offset), vector2(200, 50)) then
                    window.resize(vector2(1024, 768))
                end
                if button:show("1280x720", vector2(0, -100 + offset), vector2(200, 50)) then
                    window.resize(vector2(1280, 720))
                end
                if button:show("1920x1080", vector2(0, -200 + offset), vector2(200, 50)) then
                    window.resize(vector2(1920, 1080))
                end
                if button:show("Back", vector2(0, -300), vector2(200, 50)) then
                    self.option_window_size = false
                end

                return
            end
            if self.option then
                local texture = GUI_MANAGER:get_texture()
                texture:fill_color(color(1, 1, 1, 0.5))
                image:show(texture, vector2(0, 0), vector2(1000, 600))
                text:show("Option", vector2(0, 200), 50)
                if button:show("Window Size", vector2(0, 100), vector2(200, 50)) then
                    self.option_window_size = true
                end
                if button:show("Back", vector2(0, -100), vector2(200, 50)) then
                    self.option = false
                end
                return
            end
            if not self.hide then
                text:show("Menu", vector2(0, 200), 50)
                if button:show("Resume", vector2(0, 70), vector2(150, 50)) then
                    self.hide_next = true
                end
                if button:show("Option", vector2(0, 0), vector2(150, 50)) then
                    self.option = true
                end
                if button:show("Quit", vector2(0, -70), vector2(150, 50)) then
                    change_scene("")
                end
            end
        end,
        draw = function(self) end
    }
    return object
end

return menu
