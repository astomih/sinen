local sn = require("@sinen")
local World = {}
World.__index = World
World.new = function()
	local self = setmetatable(
		{} :: {
			position: sn.Vec3,
			rotation: sn.Vec3,
			scale: sn.Vec3,
			aabb: sn.AABB,
		},
		World :: typeof(World)
	)
	self.position = sn.Vec3.new(0)
	self.rotation = sn.Vec3.new(0)
	self.scale = sn.Vec3.new(1)
	self.aabb = sn.AABB.new()
	return self
end
export type World = typeof(World.new())
return World
