local gui_list = require("gui/gui_list")

local function menu()
    local object = {
        list = {},
        hide = true,
        setup = function(self)
            self.list = gui_list()
            self.list.texts = { "Resume", "Quit" }
            self.list.num = #self.list.texts
            self.list:setup()
            self.list.hide = self.hide
        end,
        update = function(self)
            if keyboard:is_key_pressed(keyESCAPE) then
                self.hide = not self.hide
                mouse:hide_cursor(false)
                self.list.hide = self.hide
            end
            if not self.hide then
                self.list:update()
                if self.list.selecting_index == 1 and (mouse:is_button_down(mouseLEFT)) then
                    self.hide = not self.hide
                    mouse:hide_cursor(false)
                    self.list.hide = self.hide
                end
                if self.list.selecting_index == 2 and (mouse:is_button_down(mouseLEFT)) then
                    change_scene("")
                end
            end
        end,
        draw = function(self) self.list:draw() end
    }
    return object
end

return menu
