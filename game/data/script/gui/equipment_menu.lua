local button = require("gui/button")()
local text = require("gui/text")()
local scroll = require("gui/scroll")()
local image = require("gui/image")()
local function equipment_menu()
  local object = {
    hide = true,
    ui_panel = {},
    ui_panel_texture = {},
    is_list = false,
    setup = function(self)
      -- setup menu
      self.ui_panel_texture = texture()
      self.ui_panel_texture:fill_color(color(1, 1, 1, 0.5))
      self.ui_panel = drawui(self.ui_panel_texture)
      self.ui_panel.scale = vector2(1120, 630)
      button.fg_color = color(0, 0, 0, 0.9)
      button.bg_color = color(1, 1, 1, 1.0)

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
      button.fg_color = color(1, 1, 1, 0.9)
      button.bg_color = color(1, 0, 0, 1.0)
      if button:show("BACK", vector2(500, 270), vector2(100, 50)) then
        self.hide = true
      end
      button.fg_color = color(0, 0, 0, 0.9)
      button.bg_color = color(1, 1, 1, 1.0)
      text:show("MHP: ", vector2(-400, 200), 50)
      text:show("STM: ", vector2(-400, 175), 50)
      text:show("OIL: ", vector2(-400, 150), 50)

      if button:show("SP1", vector2(-400, 100), vector2(150, 50)) then
        self.is_list = true
      end
      if button:show("SP2", vector2(-400, 0), vector2(150, 50)) then
        self.is_list = true
      end
      if button:show("ORBIT", vector2(-400, -100), vector2(150, 50)) then
        self.is_list = true
      end
      if button:show("BOOSTER", vector2(-400, -200), vector2(150, 50)) then
        self.is_list = true
      end
      if self.is_list then
        scroll:show(vector2(200, 0), vector2(20, 200))
        local offset = scroll.pos.y
        local tex = texture()
        tex:fill_color(color(0, 0, 0, 0.5))
        image:show(tex, vector2(0, 0), vector2(250, 500))
        if button:show("a", vector2(0, 100 + offset), vector2(200, 50)) then
          -- do
        end
        if button:show("b", vector2(0, 50 + offset), vector2(200, 50)) then
          -- do
        end
        if button:show("c", vector2(0, 0 + offset), vector2(200, 50)) then
          -- do
        end
        image:show(tex, vector2(0, 200), vector2(250, 50))
        text:show("EQUIPMENT LIST", vector2(0, 200), 50)
        button.fg_color = color(1, 1, 1, 0.9)
        button.bg_color = color(1, 0, 0, 1.0)
        if button:show("Close", vector2(0, -150), vector2(200, 50))
        then
          self.is_list = false
        end

      end


      -- update menu
      return true

    end
  }
  return object
end

return equipment_menu
