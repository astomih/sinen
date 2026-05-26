local sn = require("@sinen")
local World = require("./world")

local Effect = {}
Effect.__index = Effect
Effect.new = function()
	local self = setmetatable(
		{} :: {
			duration: number,
			looping: boolean,
			prewarm: boolean,
			start_lifetime: number,
			start_speed: number,
			start_size: number,
			start_rotation: number,
			start_color: sn.Color,
			gravity_multiplier: number,
			inherit_velocity: number,
			play_on_awake: boolean,
			max_particles: number,
			transform: { sn.Transform },
			model: sn.Model,
			texture: sn.Texture,
			worlds: { World.World },
			is_playing: boolean,
			timer: number,

			impl: (self: Effect) -> (),
			update: (self: Effect) -> (),
			draw: (self: Effect) -> (),
			play: (self: Effect) -> (),
		},
		Effect :: typeof(Effect)
	)
	self.duration = 0.0
	self.looping = false
	self.prewarm = false
	self.start_lifetime = 0.5
	self.start_speed = 5.0
	self.start_size = 1.0
	self.start_rotation = 0.0
	self.start_color = sn.Color.new(1, 1, 1, 1)
	self.gravity_multiplier = 0.0
	self.inherit_velocity = 0.0
	self.play_on_awake = false
	self.max_particles = 10

	self.looping = false
	self.texture = sn.Texture.new()
	self.texture:fill(self.start_color)
	self.transform = {}
	self.model = sn.Model.new()
	self.model:loadBox()
	self.model:setTexture(sn.TextureKey.BaseColor, self.texture)

	self.worlds = {}
	for i = 1, self.max_particles do
		self.worlds[i] = World.new()
		self.worlds[i].position = sn.Vec3.new(0)
		self.worlds[i].rotation = sn.Vec3.new(0)
		self.worlds[i].scale = sn.Vec3.new(0.1)
	end
	self.is_playing = false
	self.timer = 0.0

	return self
end
export type Effect = typeof(Effect.new())

Effect.impl = function(self: Effect)
	local dT = sn.Time.delta()
	for i = 1, self.max_particles do
		self.worlds[i].position.x = self.worlds[i].position.x + math.cos(i) * dT
		self.worlds[i].position.y = self.worlds[i].position.y + math.sin(i) * dT
		self.worlds[i].position.z = self.worlds[i].position.z + dT
	end
end
Effect.update = function(self: Effect)
	--  if not self.is_playing then return end
	local dT: number = sn.Time.delta()
	self.timer = self.timer + dT
	if self.timer > self.start_lifetime then
		self.timer = 0.0
		for i = 1, self.max_particles do
			self.worlds[i].position = sn.Vec3.new(0)
			self.worlds[i].rotation = sn.Vec3.new(0)
			self.worlds[i].scale = sn.Vec3.new(0.1)
		end
		if not self.looping then
			self.is_playing = false
		end
	end
	self:impl()
	if self.is_playing then
		self.transform = {}
		for i = 1, self.max_particles do
			local transform = sn.Transform.new()
			transform.position = self.worlds[i].position
			transform.rotation = self.worlds[i].rotation
			transform.scale = self.worlds[i].scale
			table.insert(self.transform, transform)
		end
	end
end
---@param self Effect
Effect.draw = function(self: Effect)
	if self.is_playing then
		sn.Graphics.drawModelInstanced(self.model, self.transform)
	end
end
---@param self Effect
Effect.play = function(self: Effect)
	self.is_playing = true
end
return Effect
