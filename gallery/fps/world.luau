local World = {}
World.__index = World
World.new = function()
	local self = setmetatable(
		{} :: {
			position: Vec3,
			rotation: Vec3,
			scale: Vec3,
			aabb: AABB,
		},
		World :: typeof(World)
	)
	self.position = Vec3.new(0)
	self.rotation = Vec3.new(0)
	self.scale = Vec3.new(1)
	self.aabb = AABB.new()
	return self
end
export type World = typeof(World.new())
return World
