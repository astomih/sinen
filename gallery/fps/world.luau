---@class World
---@field position Vec3
---@field rotation Vec3
---@field scale Vec3
---@field aabb AABB
local World = {}
World.__index = World
World.new = function()
	local self = setmetatable({}, World)
	self.position = Vec3.new(0)
	self.rotation = Vec3.new(0)
	self.scale = Vec3.new(1)
	self.aabb = AABB.new()
	return self
end
return World
