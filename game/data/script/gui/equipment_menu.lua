local button = require("gui/button")()
local function equipment_menu()
  local object = {
    hide = true,
    ui_panel = {},
    ui_panel_texture = {},
    setup = function(self)
      -- setup menu
      self.ui_panel_texture = texture()
      self.ui_panel_texture:fill_color(color(1, 1, 1, 0.5))
      self.ui_panel = drawui(self.ui_panel_texture)
      self.ui_panel.scale = vector2(1120, 630)

    end,
    draw = function(self)
      if self.hide then
        return
      end
    end,
    update = function(self)
      if keyboard.is_pressed(keyboard.E) then
        self.hide = not self.hide
      end
      if self.hide then
        return false
      end
      mouse.hide_cursor(false)
      GUI_MANAGER:add(self.ui_panel)
      if button:show("×", vector2(500, 270), vector2(100, 50)) then
        self.hide = true
      end
      -- update menu
      return true

    end
  }
  return object
end

return equipment_menu
