local bullet = require("./bullet")
local G = require("./global")
local Player = require("./player")
local player = Player.new()
local Enemy = require("./enemy")
local dungeonGenerator = require("./dungeon_generator/dungeon_generator")
local enemies: { Enemy.Enemy } = {}
local enemy_max_num = 100
local World = require("./world")
local Effect = require("./effect")
local map_size_x: number = 64
local map_size_y: number = 64
local map: Grid = Grid.new(map_size_x, map_size_y)
local map_z = Grid.new(map_size_x, map_size_y)
-- draw object
local map_draw3ds = {}
local box = {}
local sprite = {}
local stair = Transform.new()
-- assets
local tile = Texture.new()
tile:fill(Color.new(0.416, 0.204, 0.153, 1))

local sprite_model = Model.new()
sprite_model:loadSprite()

local menu = require("./gui/menu")
local menu_object = menu()
local scene_switcher = require("./scene_switcher").new()
local CameraController = require("./camera_controller")
local camera_controller = CameraController.new()

menu_object:setup()
G.DEFAULT_TEXTURE = Texture.new()
if G.DEFAULT_TEXTURE ~= nil then
	G.DEFAULT_TEXTURE:fill(Color.new(1, 1, 1, 1))
end
map:fill(0)
map_z:fill(0)
dungeonGenerator(map, 0, -1, 4, 3, 2)

local boxModel = Model.new()
boxModel:loadBox()
sprite_model:setTexture(TextureKey.BaseColor, tile)
for i = 1, G.COLLISION_SPACE_DIVISION + 2 do
	G.COLLISION_SPACE[i] = {}
	for j = 1, G.COLLISION_SPACE_DIVISION + 2 do
		G.COLLISION_SPACE[i][j] = {}
	end
end
for i = 1, enemy_max_num do
	table.insert(enemies, Enemy.new())
end
player:setup(map, map_size_x, map_size_y)
for i, v in ipairs(enemies) do
	v:setup(map, map_size_x, map_size_y)
end
for y = 1, map_size_y do
	map_draw3ds[y] = {}
	for x = 1, map_size_x do
		map_draw3ds[y][x] = World.new()
		map_draw3ds[y][x].position.x = x * G.TILE_SIZE
		map_draw3ds[y][x].position.y = y * G.TILE_SIZE
		map_draw3ds[y][x].scale = Vec3.new(G.TILE_SIZE * 0.5, G.TILE_SIZE * 0.5, 1)
		map_draw3ds[y][x].aabb = AABB.new()
		map_draw3ds[y][x].aabb.max = map_draw3ds[y][x].position + map_draw3ds[y][x].scale
		map_draw3ds[y][x].aabb.min = map_draw3ds[y][x].position - map_draw3ds[y][x].scale
		if map:at(x, y) ~= G.MAP_CHIP.STAIR then
			local transform = Transform.new()
			transform.position = map_draw3ds[y][x].position
			transform.rotation = map_draw3ds[y][x].rotation
			transform.scale = map_draw3ds[y][x].scale
			table.insert(sprite, transform)
		end
		if map:at(x, y) == G.MAP_CHIP.WALL then
			map_draw3ds[y][x].position.z = 0.5
			map_draw3ds[y][x].aabb = AABB.new()
			map_draw3ds[y][x].aabb.max = map_draw3ds[y][x].position + map_draw3ds[y][x].scale
			map_draw3ds[y][x].aabb.min = map_draw3ds[y][x].position - map_draw3ds[y][x].scale
			map_z:set(x, y, 0)
			map_draw3ds[y][x].position.z = map_z:at(x, y) / 10.0
			map_draw3ds[y][x].scale.z = 3
		end
		if map:at(x, y) == G.MAP_CHIP.STAIR then
			stair.position.x = x * G.TILE_SIZE
			stair.position.y = y * G.TILE_SIZE + 0.5
			stair.position.z = 0
		end
		if map:at(x, y) == G.MAP_CHIP.KEY then
			stair.position.x = x * G.TILE_SIZE
			stair.position.y = y * G.TILE_SIZE + 0.5
			stair.position.z = 0
		end
		if map:at(x, y) == G.MAP_CHIP.PLAYER then
			player.transform.position.x = x * G.TILE_SIZE
			player.transform.position.y = y * G.TILE_SIZE
		end
	end
end

camera_controller:setup(player)
camera_controller:update()
scene_switcher:start("")

---@param map_draw3ds World[][]
local FrustumCullingMapDraw = function(map_draw3ds)
	for y = 1, map_size_y do
		for x = 1, map_size_x do
			if Graphics.getCamera3D():isAABBInFrustum(map_draw3ds[y][x].aabb) then
				if map:at(x, y) == G.MAP_CHIP.WALL then
					local transform = Transform.new()
					transform.position = map_draw3ds[y][x].position
					transform.rotation = map_draw3ds[y][x].rotation
					transform.scale = Vec3.new(2)
					table.insert(box, transform)
				end
				local p = map_draw3ds[y][x].position:copy()
				p.z = 0.0
				local transform = Transform.new()
				transform.position = p
				transform.rotation = map_draw3ds[y][x].rotation
				transform.scale = map_draw3ds[y][x].scale
				table.insert(sprite, transform)
			end
		end
	end
end
function draw()
	player:draw3()

	for i, v in ipairs(enemies) do
		v:draw()
	end
	box = {}
	sprite = {}
	FrustumCullingMapDraw(map_draw3ds)
	Graphics.drawModelInstanced(boxModel, box)
	Graphics.drawModelInstanced(sprite_model, sprite)
	Graphics.drawModel(sprite_model, stair)

	player:draw2()
	Graphics.drawText("SCORE: " .. G.SCORE, G.FONT_DEFAULT64, Vec2.new(-300, 300), Color.new(1, 1, 1, 1), 32)
	menu_object:draw()
	scene_switcher:draw()
	G.GUI_MANAGER:draw()
end

local collision_bullets = function(_bullets: { bullet.Bullet })
	---@param v Bullet
	for i, v in ipairs(_bullets) do
		---@param w Enemy
		for j, w in ipairs(enemies) do
			if v.aabb:intersectsAABB(w.aabb) then
				local efk = Effect.new()
				efk.texture:fill(Color.new(1, 0.2, 0.2, 1))
				for k = 1, efk.max_particles do
					efk.worlds[k].position = w.transform.position:copy()
				end
				efk:play()
				table.insert(player.efks, efk)

				table.remove(player.bullets, i)
				-- hp
				if w:add_damage(10) then
					G.SCORE = G.SCORE + 10
					table.remove(enemies, j)
				end
				if #enemies <= 0 then
					stair.position.z = 0
				end
			end
		end
		if
			map:at(
				math.floor(v.transform.position.x / G.TILE_SIZE + 0.5),
				math.floor(v.transform.position.y / G.TILE_SIZE + 0.5)
			) < G.MAP_CHIP_WALKABLE
		then
			table.remove(player.bullets, i)
		end
	end
end
function update()
	G.GUI_MANAGER:update()
	if scene_switcher.flag then
		scene_switcher:update()
		return
	end
	menu_object:update()
	if not menu_object.hide then
		return
	end

	Mouse.setRelative(true)
	collision_bullets(player.bullets)
	player:update(map, map_draw3ds, map_size_x, map_size_y)
	stair.position.z = 0.0
	for i, v in ipairs(enemies) do
		v:update(player)
		v:player_collision(player)
	end
	stair.position.z = Periodic.sin0_1(1.0, Time.seconds())
	if
		math.floor(player.transform.position.x + 0.5) == math.floor(stair.position.x)
		and math.floor(player.transform.position.y + 0.5) == math.floor(stair.position.y)
	then
		scene_switcher:start("scene02_clear")
	end
	if player.hp <= 0 then
		scene_switcher:start("scene03_gameover")
	end
	camera_controller:update()
end

---@param _bullets Bullet[]
