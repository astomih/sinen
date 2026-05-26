local sn = require("@sinen")
local Grid = require("../grid")
--------------------------------------------------------------------
-- room.lua
--------------------------------------------------------------------
local Room = {}
Room.__index = Room

local function RoomType()
	local self = setmetatable(
		{} :: {
			size: sn.Vec2,
			position: sn.Vec2,
			fill: (self: Room, grid: Grid.Grid, floor_id: number) -> nil,

			new: (grid_size: sn.Vec2, minimum: sn.Vec2, maximum: sn.Vec2) -> Room,
			get_position: (self: Room) -> sn.Vec2,
			get_size: (self: Room) -> sn.Vec2,
		},
		Room :: typeof(Room)
	)
	return self
end
export type Room = typeof(RoomType())

function Room.new(grid_size: sn.Vec2, minimum: sn.Vec2, maximum: sn.Vec2): Room
	local self = RoomType()
	self:_set_size(minimum, maximum)
	self:_set_random_position(grid_size)
	return self
end

---@param grid Grid
function Room.fill(self: Room, grid: Grid.Grid, floor_id)
	for dy = 1, self.size.y do
		for dx = 1, self.size.x do
			grid:set(self.position.x + dx, self.position.y + dy, floor_id)
		end
	end
end

-- ------- getters -------
function Room.get_position(self: Room): sn.Vec2
	return self.position
end

function Room.get_size(self: Room): sn.Vec2
	return self.size
end

-- ------- private helpers -------
function Room._set_size(self: Room, minimum: sn.Vec2, maximum: sn.Vec2)
	local w = sn.Random.getIntRange(minimum.x, maximum.x)
	local h = sn.Random.getIntRange(minimum.y, maximum.y)
	self.size = sn.Vec2.new(w, h)
end

function Room._set_random_position(self: Room, grid_size: sn.Vec2)
	local px = sn.Random.getIntRange(2, grid_size.x - self.size.x)
	local py = sn.Random.getIntRange(2, grid_size.y - self.size.y)
	self.position = sn.Vec2.new(px, py)
end

return Room
