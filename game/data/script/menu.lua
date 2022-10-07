local gui_list = require("gui_list")

local function menu()
    local object = {
        list = {},
        hide = true,
        setup = function(self)
            self.list = gui_list()
            self.list.num = 1
            self.list.texts[1] = "Quit"
            self.list:setup()
            self.list.hide = self.hide
        end,
        update = function(self)
            if keyboard:key_state(keyESCAPE) == buttonPRESSED then
                self.hide = not self.hide
                mouse:hide_cursor(self.hide)
                self.list.hide = self.hide
            end
            if not self.hide then self.list:update() end
        end,
        draw = function(self) self.list:draw() end
    }
    return object
end

return menu
