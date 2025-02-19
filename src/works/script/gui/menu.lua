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
            local is_esc = keyboard.is_pressed(keyboard.ESCAPE)
            if self.hide_next then
                self.hide = not self.hide
                self.hide_next = false
            else
                if self.hide and is_esc then
                    self.hide_next = true
                end
            end
            if self.option_window_size then
                text:show("Window Size", vector2(0, 200), 50)
                scroll:show(vector2(200, 0), vector2(20, 400))
                local offset = scroll.pos.y * 1.5
                local start = 100
                if button:show("1120x630", vector2(0, start + offset), vector2(200, 50)) then
                    window.resize(vector2(1120, 630))
                end
                start = start - 100
                if button:show("1280x720", vector2(0, start + offset), vector2(200, 50)) then
                    window.resize(vector2(1280, 720))
                end
                start = start - 100
                if button:show("1440x810", vector2(0, start + offset), vector2(200, 50)) then
                    window.resize(vector2(1440, 810))
                end
                start = start - 100
                if button:show("1600x900", vector2(0, start + offset), vector2(200, 50)) then
                    window.resize(vector2(1600, 900))
                end
                start = start - 100
                if button:show("1760x990", vector2(0, start + offset), vector2(200, 50)) then
                    window.resize(vector2(1760, 990))
                end
                start = start - 100
                if button:show("1920x1080", vector2(0, start + offset), vector2(200, 50)) then
                    window.resize(vector2(1920, 1080))
                end
                if button:show("Back", vector2(0, -300), vector2(200, 50)) or is_esc then
                    self.option_window_size = false
                end

                return
            end
            if self.option then
                text:show("Option", vector2(0, 200), 50)
                if button:show("Window Size", vector2(0, 100), vector2(200, 50)) then
                    self.option_window_size = true
                end
                if button:show("Back", vector2(0, -100), vector2(200, 50)) or is_esc then
                    self.option = false
                end
                return
            end
            if not self.hide then
                mouse.hide_cursor(false)
                text:show("Menu", vector2(0, 200), 50)
                if button:show("Resume", vector2(0, 70), vector2(150, 50)) or is_esc then
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
