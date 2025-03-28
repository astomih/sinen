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
      self.ui_panel_texture = Texture()
      self.ui_panel_texture:fill_color(Color(1, 1, 1, 0.5))
      self.ui_panel = Draw2D(self.ui_panel_texture)
      self.ui_panel.scale = Vector2(1120, 630)
      button.fg_color = Color(0, 0, 0, 0.9)
      button.bg_color = Color(1, 1, 1, 1.0)
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
      button.fg_color = Color(1, 1, 1, 0.9)
      button.bg_color = Color(1, 0, 0, 1.0)
      if button:show("BACK", Vector2(500, 270), Vector2(100, 50)) then
        self.hide = true
      end
      button.fg_color = Color(0, 0, 0, 0.9)
      button.bg_color = Color(1, 1, 1, 1.0)
      text:show("MHP: ", Vector2(-400, 200), 50)
      text:show("STM: ", Vector2(-400, 175), 50)
      text:show("OIL: ", Vector2(-400, 150), 50)

      if button:show("SP1", Vector2(-400, 100), Vector2(150, 50)) then
        self.is_list = true
      end
      if button:show("SP2", Vector2(-400, 0), Vector2(150, 50)) then
        self.is_list = true
      end
      if button:show("ORBIT", Vector2(-400, -100), Vector2(150, 50)) then
        self.is_list = true
      end
      if button:show("BOOSTER", Vector2(-400, -200), Vector2(150, 50)) then
        self.is_list = true
      end
      if self.is_list then
        scroll:show(Vector2(200, 0), Vector2(20, 200))
        local offset = scroll.pos.y
        local tex = Texture()
        tex:fill_color(Color(0, 0, 0, 0.5))
        image:show(tex, Vector2(0, 0), Vector2(250, 500))
        if button:show("a", Vector2(0, 100 + offset), Vector2(200, 50)) then
          -- do
        end
        if button:show("b", Vector2(0, 50 + offset), Vector2(200, 50)) then
          -- do
        end
        if button:show("c", Vector2(0, 0 + offset), Vector2(200, 50)) then
          -- do
        end
        image:show(tex, Vector2(0, 200), Vector2(250, 50))
        text:show("EQUIPMENT LIST", Vector2(0, 200), 50)
        button.fg_color = Color(1, 1, 1, 0.9)
        button.bg_color = Color(1, 0, 0, 1.0)
        if button:show("Close", Vector2(0, -150), Vector2(200, 50))
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
