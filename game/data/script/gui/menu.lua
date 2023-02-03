local list = require("gui/list")

local function menu()
    local object = {
        list = {},
        hide = true,
        hide_next = false,
        setup = function(self)
            self.list = list()
            self.list.texts = { "Resume", "Option", "Quit" }
            self.list.num = #self.list.texts
            self.list:setup()
            self.list.hide = self.hide
        end,
        update = function(self)
            if self.hide_next then
                self.hide = not self.hide
                self.hide_next = false
                mouse:hide_cursor(false)
                self.list.hide = self.hide
            end
            if keyboard:is_key_pressed(keyESCAPE) then
                self.hide = not self.hide
                mouse:hide_cursor(false)
                self.list.hide = self.hide
            end
            if not self.hide then
                self.list:update()
                if self.list.selecting then
                    if self.list.texts[self.list.selecting_index] == "Resume" and
                        (mouse:is_button_pressed(mouseLEFT)) then
                        self.hide_next = true
                    end
                    if self.list.texts[self.list.selecting_index] == "Option"
                        and (mouse:is_button_pressed(mouseLEFT)) then
                        print("Option")
                    end
                    if self.list.texts[self.list.selecting_index] == "Quit"
                        and (mouse:is_button_pressed(mouseLEFT)) then
                        change_scene("")
                    end
                end
            end
        end,
        draw = function(self) self.list:draw() end
    }
    return object
end

return menu
