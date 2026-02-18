local G = require("./global")
local Bullet = require("bullet")
local calc_input_vector = require("calc_input_vector")
local is_collision = require("is_collision")
local input_vector = {}
local speed = 2.0
local Effect = require("effect")
local r1 = 0
local r2 = 0

local scopeTex = Texture.new()
scopeTex:load("image/scope.png")

---@param map Grid
local function decide_pos(map, map_size_x, map_size_y)
	r1 = math.random(1, map_size_x)
	r2 = math.random(1, map_size_y)
	return map:at(r1, r2) == 1
end

---@class Player
---@field transform Transform
---@field  rotationZ number
---@field model Model
---@field bullets Bullet[]
---@field hp number
---@field hpMax number
---@field hpColor Color
---@field stamina number
---@field stamina_max number
---@field stamina_recover_speed number
---@field stamina_boost_cost number
---@field stamina_run_cost integer
---@field stamina_texture Texture
---@field stamina_rect Rect
---@field stamina_max_texture Texture
---@field stamina_max_rect Rect
---@field aabb AABB
---@field bullet_time number
---@field bullet_timer number
---@field bullet_flag boolean
---@field efks Effect[]
---@field is_shot boolean
---@field boost number
---@field boost_time number
---@field boost_timer number
---@field boost_mag number
---@field boost_sound Sound
---@field is_boost boolean
---@field speed_min number
---@field speed_max number
---@field blur_time number
---@field scopePos Vec2
---@field horizontal number
---@field vertical number
local Player = {}

Player.__index = Player

Player.new = function()
	local self = setmetatable({}, Player)
	self.rotationZ = 0.0
	self.bullets = {}
	self.hp = 0
	self.hpMax = 100
	self.hpColor = Color.new(1, 1, 1, 1)
	self.stamina = 0
	self.stamina_max = 100
	self.stamina_recover_speed = 20
	self.stamina_boost_cost = 20
	self.stamina_run_cost = 5
	self.bullet_time = 0.0
	self.bullet_flag = false
	self.efks = {}
	self.is_shot = true
	self.boost = 0.0
	self.boost_time = 0.3
	self.boost_timer = 0.0
	self.boost_mag = 5
	self.is_boost = false
	self.speed_min = 6.0
	self.speed_max = 16.0
	self.blur_time = 0.0
	self.scopePos = Vec2.new(0)
	self.horizontal = math.pi
	self.vertical = 0.0
	return self
end
---@param self Player
Player.boost_reset = function(self)
	self.boost_timer = 0.0
	self.boost = 0.0
	self.is_boost = false
end
---@param self Player
---@param map Grid
---@param map_size_x number
---@param map_size_y number
Player.setup = function(self, map, map_size_x, map_size_y)
	self.model = Model.new()
	self.model:load("model/triangle.glb")
	self.transform = Transform.new()
	self.aabb = AABB.new()
	self.bullet_time = 0.1
	self.bullet_timer = 0.0
	self.hp = 100
	self.stamina = self.stamina_max
	self.stamina_texture = Texture.new()
	self.stamina_texture:fill(Color.new(1.0, 1.0, 1.0, 0.9))
	self.stamina_max_texture = Texture.new()
	self.stamina_max_texture:fill(Color.new(0.0, 0.0, 0.0, 0.2))
	self.stamina_rect = Rect.new(Vec2.new(0, 350), G.UI_SCALE_Vec2(300, 10))
	self.stamina_max_rect = Rect.new(Vec2.new(0, 350), G.UI_SCALE_Vec2(300, 10))

	self:render_text()
	r1 = 0
	r2 = 0
	while decide_pos(map, map_size_x, map_size_y) == true do
	end
	self.transform.position = Vec3.new(r1 * 2, r2 * 2, 1.0)
	self.transform.scale = Vec3.new(1)
	self.boost_sound = Sound.new()
	self.boost_sound:load("sound/boost.wav")
	self.boost_sound:setVolume(0.2)
end
---@param self Player
---@param map Grid
---@param map_draw3ds World[][]
---@param map_size_x number
---@param map_size_y number
Player.update = function(self, map, map_draw3ds, map_size_x, map_size_y)
	local p = self.transform.position:copy()
	self.aabb:updateWorld(self.transform.position, self.transform.scale, self.model:getAABB())
	input_vector = calc_input_vector()
	local is_move = input_vector.x ~= 0 or input_vector.y ~= 0

	if Keyboard.isDown(Keyboard.LSHIFT) and is_move then
		speed = self.speed_max
		self.stamina = self.stamina - self.stamina_run_cost * Time.delta()
		if self.stamina <= 0.0 then
			self.stamina = 0.0
			speed = self.speed_min
		end
	else
		speed = self.speed_min
		self.stamina = self.stamina + Time.delta() * self.stamina_recover_speed
		if self.stamina > self.stamina_max then
			self.stamina = self.stamina_max
		end
	end
	-- bullet
	if Mouse.isPressed(Mouse.LEFT) then
		self.bullet_flag = true
	end
	self.bullet_timer = self.bullet_timer + Time.delta()
	if self.bullet_flag then
		if self.bullet_timer > self.bullet_time and (Mouse.isDown(Mouse.LEFT)) then
			local forward = Vec3.new(
				math.sin(math.rad(self.transform.rotation.z)),
				math.cos(math.rad(self.transform.rotation.z)),
				math.sin(math.rad(self.transform.rotation.y))
			)
			local cross = Vec3.new(
				math.sin(math.rad(self.transform.rotation.z + 90)),
				math.cos(math.rad(self.transform.rotation.z + 90)),
				-0.5
			)

			local rot = forward * Vec3.new(1000, 1000, 1000) - cross
			rot = rot:normalize()

			local b = Bullet.new()
			b:setup(self.transform, rot)
			b.transform.position = b.transform.position + cross * Vec3.new(0.25, 0.25, 1)
			table.insert(self.bullets, b)
			self.bullet_timer = 0.0
		end
		if Mouse.isReleased(Mouse.LEFT) then
			self.bullet_flag = false
		end
	end
	if self.is_boost then
		if self.boost_timer >= self.boost_time then
			self:boost_reset()
		else
			local t = Periodic.sin0_1(self.boost_time * 2.0, self.boost_timer) - 0.5
			t = t * 0.2
			self.boost_timer = self.boost_timer + Time.delta()
		end
	else
		if Keyboard.isPressed(Keyboard.SPACE) and is_move then
			if self.stamina >= self.stamina_boost_cost then
				self.stamina = self.stamina - self.stamina_boost_cost
				if self.stamina <= 0.0 then
					self.stamina = 0.0
				end
				---@type Effect
				local efk = Effect.new()
				efk.texture:fill(Color.new(0.6, 0.6, 1.0, 1.0))
				efk.impl = function(e)
					for i = 1, e.max_particles do
						local t = Time.delta() * 2
						e.worlds[i].position.x = e.worlds[i].position.x + math.cos(i) * t
						e.worlds[i].position.y = e.worlds[i].position.y + math.sin(i) * t
						e.worlds[i].position.z = e.worlds[i].position.z + t
					end
				end
				for j = 1, efk.max_particles do
					efk.worlds[j].position = self.transform.position:copy()
				end
				efk:play()
				table.insert(self.efks, efk)
				self.boost_sound:play()
				self.boost = self.boost_mag
				self.is_boost = true
				self.boost_timer = 0.0
			end
		end
	end
	for i, v in ipairs(self.bullets) do
		v:update()
		if v.current_time > v.life_time then
			local efk = Effect.new()
			efk.texture:fill(Color.new(1.0, 1.0, 1.0, 1.0))
			for j = 1, efk.max_particles do
				efk.worlds[j].position = v.transform.position:copy()
			end
			efk:play()
			table.insert(self.efks, efk)
			table.remove(self.bullets, i)
		end
	end
	for i, v in ipairs(self.efks) do
		v:update()
		if not v.is_playing then
			table.remove(self.efks, i)
		end
	end
	local before_pos = Vec3.new(self.transform.position.x, self.transform.position.y, self.transform.position.z)
	local final_speed = 0.0
	if self.is_boost then
		final_speed = self.speed_max * self.boost
	else
		final_speed = speed
	end
	if input_vector.x ~= 0 and input_vector.y ~= 0 then
		final_speed = final_speed / math.sqrt(2)
	end

	before_pos = self.transform.position:copy()
	local flag = false
	if input_vector.y ~= 0 then
		flag = true
		self.transform.position = self.transform.position
			+ Vec3.new(
				math.sin(math.rad(self.transform.rotation.z)) * final_speed * Time.delta() * input_vector.y,
				math.cos(math.rad(self.transform.rotation.z)) * final_speed * Time.delta() * input_vector.y,
				0
			)
	end
	if input_vector.x ~= 0 then
		flag = true
		self.transform.position = self.transform.position
			+ Vec3.new(
				math.sin(math.rad(self.transform.rotation.z + 90 * input_vector.x)) * final_speed * Time.delta(),
				math.cos(math.rad(self.transform.rotation.z + 90 * input_vector.x)) * final_speed * Time.delta(),
				0
			)
	end

	if flag then
		local dxy = Vec2.new(self.transform.position.x, self.transform.position.y)
			- Vec2.new(before_pos.x, before_pos.y)
		self.aabb:updateWorld(
			self.transform.position - Vec3.new(0, dxy.y, 0),
			self.transform.scale,
			self.model:getAABB()
		)
		if
			is_collision(
				self.transform.position - Vec3.new(0, dxy.y, 0),
				self.aabb,
				map,
				map_draw3ds,
				map_size_x,
				map_size_y
			)
		then
			self.transform.position.x = before_pos.x
		end
		self.aabb:updateWorld(
			self.transform.position - Vec3.new(dxy.x, 0, 0),
			self.transform.scale,
			self.model:getAABB()
		)
		if
			is_collision(
				self.transform.position - Vec3.new(dxy.x, 0, 0),
				self.aabb,
				map,
				map_draw3ds,
				map_size_x,
				map_size_y
			)
		then
			self.transform.position.y = before_pos.y
		end
	end
	local pos = Mouse.getPositionOnScene()
	local camera2DHalf = Graphics.getCamera2D():half()
	self.transform.rotation.y = self.transform.rotation.y + math.sin(pos.y / camera2DHalf.y) * 32.0

	self.transform.rotation.z = self.transform.rotation.z + math.sin(pos.x / camera2DHalf.x) * 32.0
	Mouse.setPositionOnScene(Vec2.new(0.0, 0.0))
	local s_ratio = self.stamina / self.stamina_max
	self.stamina_rect.width = s_ratio * 300
	if s_ratio <= 0.2 then
		self.stamina_texture:fill(Color.new(1.0, 0.0, 0.0, 0.9))
	else
		self.stamina_texture:fill(Color.new(1.0, 1.0, 1.0, 0.9))
	end
end
---@param self Player
Player.draw3 = function(self)
	for i, v in ipairs(self.efks) do
		Graphics.setTexture(0, v.texture)
		v:draw()
	end
	for i, v in ipairs(self.bullets) do
		Graphics.setTexture(0, v.texture)
		v:draw()
	end
end
---@param self Player
Player.draw2 = function(self)
	Graphics.drawText("HP: " .. self.hp, G.FONT_DEFAULT64, Vec2.new(0, 300), self.hpColor, 32)
	Graphics.drawImage(self.stamina_max_texture, self.stamina_max_rect)
	Graphics.drawImage(self.stamina_texture, self.stamina_rect)
	Graphics.drawImage(scopeTex, Rect.new(self.scopePos, scopeTex:size()))
end
---@param self Player
Player.render_text = function(self)
	if self.hp / self.hpMax <= 0.2 then
		self.hpColor = Color.new(1, 0.0, 0.0, 0.8)
	else
		self.hpColor = Color.new(1, 1, 1, 0.9)
	end
end

return Player
