local button = require("gui/button")()
button.fg_color = Color(0, 0, 0, 0.9)
button.bg_color = Color(1, 1, 1, 1)
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
                text:show("Window Size", Vec2(0, 200), 50)
                scroll:show(Vec2(200, 0), Vec2(20, 400))
                local offset = scroll.pos.y * 1.5
                local start = 100
                if button:show("1120x630", Vec2(0, start + offset), Vec2(200, 50)) then
                    window.resize(Vec2(1120, 630))
                end
                start = start - 100
                if button:show("1280x720", Vec2(0, start + offset), Vec2(200, 50)) then
                    window.resize(Vec2(1280, 720))
                end
                start = start - 100
                if button:show("1440x810", Vec2(0, start + offset), Vec2(200, 50)) then
                    window.resize(Vec2(1440, 810))
                end
                start = start - 100
                if button:show("1600x900", Vec2(0, start + offset), Vec2(200, 50)) then
                    window.resize(Vec2(1600, 900))
                end
                start = start - 100
                if button:show("1760x990", Vec2(0, start + offset), Vec2(200, 50)) then
                    window.resize(Vec2(1760, 990))
                end
                start = start - 100
                if button:show("1920x1080", Vec2(0, start + offset), Vec2(200, 50)) then
                    window.resize(Vec2(1920, 1080))
                end
                if button:show("Back", Vec2(0, -300), Vec2(200, 50)) or is_esc then
                    self.option_window_size = false
                end

                return
            end
            if self.option then
                text:show("Option", Vec2(0, 200), 50)
                if button:show("Window Size", Vec2(0, 100), Vec2(200, 50)) then
                    self.option_window_size = true
                end
                if button:show("Back", Vec2(0, -100), Vec2(200, 50)) or is_esc then
                    self.option = false
                end
                return
            end
            if not self.hide then
                mouse.hide_cursor(false)
                text:show("Menu", Vec2(0, 200), 50)
                if button:show("Resume", Vec2(0, 70), Vec2(150, 50)) or is_esc then
                    self.hide_next = true
                end
                if button:show("Option", Vec2(0, 0), Vec2(150, 50)) then
                    self.option = true
                end
                if button:show("Quit", Vec2(0, -70), Vec2(150, 50)) then
                    scene.change("")
                end
            end
        end,
        draw = function(self) end
    }
    return object
end

return menu
