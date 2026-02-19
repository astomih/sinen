local G = require("./global")
local Player = require("./player")
local bombedSound = Sound.new()
bombedSound:load("sound/bombed.wav")
local r1 = 0
local r2 = 0

---@param map Grid
---@param mapSizeX number
---@param mapSizeY number
local function decidePos(map, mapSizeX, mapSizeY)
	r1 = Random.getRange(1, mapSizeX)
	r2 = Random.getRange(1, mapSizeY)
	return map:at(r1, r2) < G.MAP_CHIP_WALKABLE
end

local enemyModel = Model.new()
enemyModel:load("model/enemy.glb")
enemyModel:getAABB().max.z = 10.0
enemyModel:getAABB().min.z = -10.0

local enemyTexture = Texture.new()
enemyTexture:fill(Color.new(0, 1, 0, 1))

---@class Enemy
---@field transform Transform
---@field model Model
---@field speed number
---@field search_length number
---@field hp number
---@field aabb AABB
---@field is_collision_first boolean
---@field collision_time number
---@field collision_timer number
---@field map Grid
---@field bfs BFSGrid
local Enemy = {}

Enemy.__index = Enemy

Enemy.new = function()
	local self = setmetatable(
		{} :: {
			transform: Transform,
			model: Model,
			speed: number,
			search_length: number,
			hp: number,
			aabb: AABB,
			is_collision_first: boolean,
			collision_time: number,
			collision_timer: number,
			map: Grid,
			bfs: BFSGrid,

			get_forward_z: (drawer: Transform) -> Vec2,
			add_damage: (self: Enemy, damage: number) -> boolean,
			setup: (self: Enemy, map: Grid, mapSizeX: number, mapSizeY: number) -> nil,
			update: (self: Enemy, player: Player.Player) -> nil,
			draw: (self: Enemy) -> nil,
			player_collision: (self: Enemy, player: Player.Player) -> nil,
		},
		Enemy :: typeof(Enemy)
	)
	self.speed = 10
	self.search_length = 15
	self.hp = 30
	self.is_collision_first = true
	self.collision_time = 0
	self.collision_timer = 0.0
	return self
end

Enemy.get_forward_z = function(drawer)
	return Vec2.new(-math.sin(math.rad(drawer.rotation.z)), math.cos(math.rad(-drawer.rotation.z)))
end

---Add damage to enemy
---@param self Enemy
---@param damage number Damage value
---@return boolean Returns true if the enemy is dead
Enemy.add_damage = function(self: Enemy, damage)
	self.hp = self.hp - damage
	if self.hp <= 0 then
		return true
	end
	return false
end

---@param self Enemy
---@param map Grid
---@param mapSizeX number
---@param mapSizeY number
Enemy.setup = function(self, map, mapSizeX, mapSizeY)
	self.bfs = BFSGrid.new(map)
	self.transform = Transform.new()
	self.model = enemyModel
	self.model:setTexture(TextureKey.BaseColor, enemyTexture)
	self.transform.scale = Vec3.new(0.5, 0.5, 0.5)
	self.aabb = AABB.new()
	self.map = map
	r1 = 0
	r2 = 0
	while decidePos(map, mapSizeX, mapSizeY) == true do
	end
	self.transform.position = Vec3.new(r1 * G.TILE_SIZE, r2 * G.TILE_SIZE, 0.5)
	self.is_collision_first = true
	self.collision_time = 1.0
	self.collision_timer = 0.0
end

---@param self Enemy
---@param player Player
Enemy.update = function(self: Enemy, player)
	local dT = Time.delta()
	local length = (self.transform.position - player.transform.position):length()
	if length > self.search_length then
		return
	end
	self.aabb:updateWorld(self.transform.position, self.transform.scale, enemyModel:getAABB())
	-- If there is a wall between the player and the enemy, the enemy will not move.
	local start = Vec2.new(self.transform.position.x / G.TILE_SIZE, self.transform.position.y / G.TILE_SIZE)
	local goal = Vec2.new(player.transform.position.x / G.TILE_SIZE, player.transform.position.y / G.TILE_SIZE)

	start.x = math.floor(start.x + 0.5)
	start.y = math.floor(start.y + 0.5)
	goal.x = math.floor(goal.x + 0.5)
	goal.y = math.floor(goal.y + 0.5)

	local min_x = math.min(start.x, goal.x)
	local max_x = math.max(start.x, goal.x)
	local min_y = math.min(start.y, goal.y)
	local max_y = math.max(start.y, goal.y)
	for i = min_x, max_x do
		if self.map:at(i, start.y) < G.MAP_CHIP_WALKABLE then
			return
		end
	end
	for i = min_y, max_y do
		if self.map:at(start.x, i) < G.MAP_CHIP_WALKABLE then
			return
		end
	end
	self.transform.rotation = Vec3.new(
		0,
		0,
		math.deg(
			-math.atan2(
				player.transform.position.x - self.transform.position.x,
				player.transform.position.y - self.transform.position.y
			)
		)
	)
	local foundPath = self.bfs:findPath(start, goal)
	if foundPath then
		local path = self.bfs:trace()
		path = self.bfs:trace()

		local dir =
			Vec2.new(path.x * G.TILE_SIZE - self.transform.position.x, path.y * G.TILE_SIZE - self.transform.position.y)
		if dir.x < -1 then
			dir.x = -1
		end
		if dir.x > 1 then
			dir.x = 1
		end
		if dir.y < -1 then
			dir.y = -1
		end
		if dir.y > 1 then
			dir.y = 1
		end

		self.transform.position.x = self.transform.position.x + dir.x * dT * self.speed
		self.transform.position.y = self.transform.position.y + dir.y * dT * self.speed
	else
		self.transform.position.x = self.transform.position.x + dT * self.speed * self.get_forward_z(self.transform).x
		self.transform.position.y = self.transform.position.y + dT * self.speed * self.get_forward_z(self.transform).y
	end
	self.bfs:reset()
end

---@param self Enemy
Enemy.draw = function(self)
	Graphics.drawModel(self.model, self.transform)
end

---@param self Enemy
---@param player Player
Enemy.player_collision = function(self: Enemy, player: Player.Player)
	if self.aabb:intersectsAABB(player.aabb) then
		if self.is_collision_first then
			bombedSound:play()
			player.hp = player.hp - 1
			if player.hp <= 0 then
				player.hp = 0
			end
			player:render_text()
			self.is_collision_first = false
		else
			self.collision_timer = self.collision_timer + Time.delta()
			if self.collision_timer > self.collision_time then
				bombedSound:play()
				player.hp = player.hp - 10
				if player.hp <= 0 then
					player.hp = 0
				end
				player:render_text()
				self.collision_timer = 0.0
			end
		end
	else
		self.is_collision_first = true
	end
end
export type Enemy = typeof(Enemy.new())
return Enemy
