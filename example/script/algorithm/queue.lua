local function queue()
  local object = {
    que = {},
    push = function(self, value)
      table.insert(self.que, value)
    end,
    pop = function(self)
      return table.remove(self.que, 1)
    end,
    size = function(self)
      return #self.que
    end,
    empty = function(self)
      return self:size() == 0
    end,
    front = function(self)
      return self.que[1]
    end,
    clear = function(self)
      self.que = {}
    end
  }
  return object
end

return queue
