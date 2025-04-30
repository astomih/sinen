--------------------------------------------------------------------
-- room.lua
--------------------------------------------------------------------

local Room = {}
Room.__index = Room

function Room:new(grid_size, minimum, maximum)
  local self = setmetatable({}, Room)
  self:_set_size(minimum, maximum)
  self:_set_random_position(grid_size)
  return self
end

function Room:fill(grid, floor_id)
  for dy = 0, self.size.y - 1 do
    for dx = 0, self.size.x - 1 do
      grid:set(self.position.x + dx,
        self.position.y + dy,
        floor_id)
    end
  end
end

-- ------- getters -------
function Room:get_position() return self.position end

function Room:get_size() return self.size end

-- ------- private helpers -------
function Room:_set_size(minimum, maximum)
  local w = math.random(minimum.x, maximum.x)
  local h = math.random(minimum.y, maximum.y)
  self.size = Point2i(w, h)
end

function Room:_set_random_position(grid_size)
  local px = math.random(2, grid_size.x - self.size.x)
  local py = math.random(2, grid_size.y - self.size.y)
  self.position = Point2i(px, py)
end

return Room
