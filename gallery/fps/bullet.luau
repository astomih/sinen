local sn = require("@sinen")
local m = sn.Model.new()
local sound = sn.Sound.new()
sound:load("shot.wav")
sound:setVolume(0.3)

local Bullet = {}
Bullet.__index = Bullet
Bullet.new = function()
	return setmetatable(
		{} :: {
			speed: number,
			texture: sn.Texture,
			transform: sn.Transform,
			model: sn.Model,
			forward: sn.Vec3,
			life_time: number,
			current_time: number,
			aabb: sn.AABB,

			setup: (self: Bullet, owner: sn.Transform, forward: sn.Vec3) -> nil,
			update: (self: Bullet) -> nil,
			draw: (self: Bullet) -> nil,
		},
		Bullet :: typeof(Bullet)
	)
end
export type Bullet = typeof(Bullet.new())

Bullet.setup = function(self: Bullet, owner: sn.Transform, forward: sn.Vec3)
	self.speed = 40
	self.texture = sn.Texture.new()
	self.texture:fill(sn.Color.new(1.0, 1.0, 1.0, 1.0))
	self.transform = sn.Transform.new()
	self.transform.position = sn.Vec3.new(owner.position.x, owner.position.y, owner.position.z)
	self.transform.rotation = owner.rotation
	self.transform.scale = sn.Vec3.new(0.2, 0.2, 0.2)
	self.model = sn.Model.new()
	self.model:loadBox()
	self.model:setTexture(sn.TextureKey.BaseColor, self.texture)
	self.forward = forward
	self.life_time = 0.25
	self.current_time = 0.0
	self.aabb = sn.AABB.new()
	sound:play()
end
---@param self Bullet
Bullet.update = function(self: Bullet)
	local dT = sn.Time.delta()
	self.aabb.max = self.transform.position + (self.transform.scale * m:getAABB().max)
	self.aabb.min = self.transform.position + (self.transform.scale * m:getAABB().min)
	self.current_time = self.current_time + dT
	self.transform.position = self.transform.position
		+ (self.forward * sn.Vec3.new(self.speed, self.speed, self.speed) * sn.Vec3.new(dT))
end
---@param self Bullet
Bullet.draw = function(self: Bullet)
	sn.Graphics.drawModel(self.model, self.transform)
end

return Bullet
