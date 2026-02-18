--------------------------------------------------------------------
-- room.lua
--------------------------------------------------------------------
local Room = {}
Room.__index = Room

local function RoomType(): Room
	local self = setmetatable(
		{} :: {
			size: Vec2,
			position: Vec2,
			fill: (self: Room, grid: Grid, floor_id: number) -> nil,
		},
		Room :: typeof(Room)
	)
	return self
end
export type Room = typeof(RoomType())

function Room.new(grid_size, minimum, maximum): Room
	local self = RoomType()
	self:_set_size(minimum, maximum)
	self:_set_random_position(grid_size)
	return self
end

---@param grid Grid
function Room.fill(self: Room, grid, floor_id)
	for dy = 0, self.size.y - 1 do
		for dx = 0, self.size.x - 1 do
			grid:set(self.position.x + dx, self.position.y + dy, floor_id)
		end
	end
end

-- ------- getters -------
function Room.get_position(self: Room): Vec2
	return self.position
end

function Room.get_size(self: Room): Vec2
	return self.size
end

-- ------- private helpers -------
function Room._set_size(self: Room, minimum: Vec2, maximum: Vec2)
	local w = Random.getIntRange(minimum.x, maximum.x)
	local h = Random.getIntRange(minimum.y, maximum.y)
	self.size = Vec2.new(w, h)
end

function Room._set_random_position(self: Room, grid_size: Vec2)
	local px = Random.getIntRange(2, grid_size.x - self.size.x)
	local py = Random.getIntRange(2, grid_size.y - self.size.y)
	self.position = Vec2.new(px, py)
end

return Room
