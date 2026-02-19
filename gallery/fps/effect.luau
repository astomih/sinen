local World = require("./world")

---@class Effect
---@field duration number
---@field looping boolean
---@field prewarm boolean
---@field start_lifetime number
---@field start_speed number
---@field start_size number
---@field start_rotation number
---@field start_color  Color
---@field gravity_multiplier number
---@field inherit_velocity number
---@field play_on_awake boolean
---@field max_particles number
---@field transform Transform[]
---@field model Model
---@field texture Texture
---@field worlds World[]
---@field is_playing boolean
---@field timer number
local Effect = {}
Effect.__index = Effect
Effect.new = function(): Effect
	local self = setmetatable(
		{} :: {
			duration: number,
			looping: boolean,
			prewarm: boolean,
			start_lifetime: number,
			start_speed: number,
			start_size: number,
			start_rotation: number,
			start_color: Color,
			gravity_multiplier: number,
			inherit_velocity: number,
			play_on_awake: boolean,
			max_particles: number,
			transform: { Transform },
			model: Model,
			texture: Texture,
			worlds: { World.World },
			is_playing: boolean,
			timer: number,

			impl: (self: Effect) -> (),
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
	self.start_color = Color.new(1, 1, 1, 1)
	self.gravity_multiplier = 0.0
	self.inherit_velocity = 0.0
	self.play_on_awake = false
	self.max_particles = 10

	self.looping = false
	self.texture = Texture.new()
	self.texture:fill(self.start_color)
	self.transform = {}
	self.model = Model.new()
	self.model:loadBox()
	self.model:setTexture(TextureKey.BaseColor, self.texture)

	self.worlds = {}
	for i = 1, self.max_particles do
		self.worlds[i] = World.new()
		self.worlds[i].position = Vec3.new(0)
		self.worlds[i].rotation = Vec3.new(0)
		self.worlds[i].scale = Vec3.new(0.1)
	end
	self.is_playing = false
	self.timer = 0.0
	return self
end

---@param self Effect
Effect.impl = function(self: Effect)
	local dT = Time.delta()
	for i = 1, self.max_particles do
		self.worlds[i].position.x = self.worlds[i].position.x + math.cos(i) * dT
		self.worlds[i].position.y = self.worlds[i].position.y + math.sin(i) * dT
		self.worlds[i].position.z = self.worlds[i].position.z + dT
	end
end
---@param self Effect
Effect.update = function(self: Effect)
	--  if not self.is_playing then return end
	self.timer = self.timer + Time.delta()
	if self.timer > self.start_lifetime then
		self.timer = 0.0
		for i = 1, self.max_particles do
			self.worlds[i].position = Vec3.new(0)
			self.worlds[i].rotation = Vec3.new(0)
			self.worlds[i].scale = Vec3.new(0.1)
		end
		if not self.looping then
			self.is_playing = false
		end
	end
	self:impl()
	if self.is_playing then
		self.transform = {}
		for i = 1, self.max_particles do
			local transform = Transform.new()
			transform.position = self.worlds[i].position
			transform.rotation = self.worlds[i].rotation
			transform.scale = self.worlds[i].scale
			table.insert(self.transform, transform)
		end
	end
end
---@param self Effect
Effect.draw = function(self)
	if self.is_playing then
		Graphics.drawModelInstanced(self.model, self.transform)
	end
end
---@param self Effect
Effect.play = function(self)
	self.is_playing = true
end
export type Effect = typeof(Effect.new())
return Effect
