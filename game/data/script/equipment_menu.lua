local function equipment_menu()
  local object = {
    hide = true,
    draw = function(self)
      if self.hide then
        return
      end
      -- draw menu
    end,
    update = function(self)
      if self.hide then
        return
      end
      -- update menu
    end,
    start = function(self)
      self.hide = false
    end,
    stop = function(self)
      self.hide = true
    end,
  }
  return object
end

return equipment_menu
