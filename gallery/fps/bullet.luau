local m = Model.new()
local sound = Sound.new()
sound:load("shot.wav")
sound:setVolume(0.3)

local Bullet = {}
Bullet.__index = Bullet
Bullet.new = function()
	return setmetatable(
		{} :: {
			speed: number,
			texture: Texture,
			transform: Transform,
			model: Model,
			forward: Vec3,
			life_time: number,
			current_time: number,
			aabb: AABB,

			setup: (self: Bullet, owner: Transform, forward: Vec3) -> nil,
			update: (self: Bullet) -> nil,
			draw: (self: Bullet) -> nil,
		},
		{} :: typeof(Bullet)
	)
end
export type Bullet = typeof(Bullet.new())

Bullet.setup = function(self: Bullet, owner: Transform, forward: Vec3)
	self.speed = 40
	self.texture = Texture.new()
	self.texture:fill(Color.new(1.0, 1.0, 1.0, 1.0))
	self.transform = Transform.new()
	self.transform.position = Vec3.new(owner.position.x, owner.position.y, owner.position.z)
	self.transform.rotation = owner.rotation
	self.transform.scale = Vec3.new(0.2, 0.2, 0.2)
	self.model = Model.new()
	self.model:loadBox()
	self.model:setTexture(TextureKey.BaseColor, self.texture)
	self.forward = forward
	self.life_time = 0.25
	self.current_time = 0.0
	self.aabb = AABB.new()
	sound:play()
end
---@param self Bullet
Bullet.update = function(self: Bullet)
	local dT = Time.delta()
	self.aabb.max = self.transform.position + (self.transform.scale * m:getAABB().max)
	self.aabb.min = self.transform.position + (self.transform.scale * m:getAABB().min)
	self.current_time = self.current_time + dT
	self.transform.position = self.transform.position
		+ (self.forward * Vec3.new(self.speed, self.speed, self.speed) * Vec3.new(dT))
end
---@param self Bullet
Bullet.draw = function(self: Bullet)
	Graphics.drawModel(self.model, self.transform)
end

return Bullet
