local player = require "player"
local enemy = require "enemy"
local enemies = {}
local enemy_max_num = 0
local world = require "world"
local map_size_x = 7
local map_size_y = 7
local map = grid(map_size_x, map_size_y)
-- draw object
local map_draw3ds = {}
local box = {}
local sprite = {}
local menu = {}
local stair = {}
-- assets
local tree = model()
local music = music()
local tile = texture()
tile:fill_color(color(0.416, 0.204, 0.153, 1))

local score_font = font()
local score_texture = texture()
local score_drawer = drawui(score_texture)
tree:load("tree.sim", "tree")
local stair_texture = texture()
stair_texture:fill_color(color(1, 0.5, 0.5, 0.5))

local menu = require("gui/menu")
local menu_object = menu()
local scene_switcher = require("scene_switcher")()


local camera_controller = require("camera_controller")()

local equipment_menu = require("gui/equipment_menu")()

score_font:load(DEFAULT_FONT_NAME, 64)
menu_object:setup()
music:load("base.ogg")
music:play()
DEFAULT_TEXTURE = texture()
DEFAULT_TEXTURE:fill_color(color(1, 1, 1, 1))
map:fill(MAP_CHIP.FLOOR)
-- fill around
for i = 1, map_size_x do
  map:set(i, 1, MAP_CHIP.WALL)
  map:set(i, map_size_y, MAP_CHIP.WALL)
end
for i = 1, map_size_y do
  map:set(1, i, MAP_CHIP.WALL)
  map:set(map_size_x, i, MAP_CHIP.WALL)
end
map:set(map_size_x / 2 + 1, map_size_y / 2 + 1, MAP_CHIP.PLAYER)
map:set(2, 2, MAP_CHIP.STAIR)

box = draw3d(DEFAULT_TEXTURE)
box.vertex_name = "tree"
sprite = draw3d(tile)
sprite.is_draw_depth = false
stair = draw3d(stair_texture)
stair.vertex_name = "SPRITE"

for i = 1, COLLISION_SPACE_DIVISION + 2 do
  COLLISION_SPACE[i] = {}
  for j = 1, COLLISION_SPACE_DIVISION + 2 do
    COLLISION_SPACE[i][j] = {}
  end
end
for i = 1, enemy_max_num do table.insert(enemies, enemy()) end
player:setup(map, map_size_x, map_size_y)
for i, v in ipairs(enemies) do v:setup(map, map_size_x, map_size_y) end
for y = 1, map_size_y do
  map_draw3ds[y] = {}
  for x = 1, map_size_x do
    map_draw3ds[y][x] = world()
    map_draw3ds[y][x].position.x = x * TILE_SIZE
    map_draw3ds[y][x].position.y = y * TILE_SIZE
    map_draw3ds[y][x].scale = vector3(TILE_SIZE / 2.0, TILE_SIZE / 2.0, 1)
    if map:at(x, y) ~= MAP_CHIP.STAIR then
      sprite:add(map_draw3ds[y][x].position, map_draw3ds[y][x].rotation,
        map_draw3ds[y][x].scale)
    end
    if map:at(x, y) == MAP_CHIP.WALL then
      map_draw3ds[y][x].position.z = 0
      map_draw3ds[y][x].aabb = aabb()
      map_draw3ds[y][x].aabb.max =
      map_draw3ds[y][x].position:add(map_draw3ds[y][x].scale)
      map_draw3ds[y][x].aabb.min =
      map_draw3ds[y][x].position:sub(map_draw3ds[y][x].scale)
      map_draw3ds[y][x].scale.z = 3


    end
    if map:at(x, y) == MAP_CHIP.PLAYER then
      player.drawer.position.x = x * TILE_SIZE
      player.drawer.position.y = y * TILE_SIZE
    end
    if (map:at(x, y) == MAP_CHIP.STAIR) then
      stair.position.x = x * TILE_SIZE
      stair.position.y = y * TILE_SIZE
      stair.position.z = 0.1
    end
  end
end
score_font:render_text(score_texture, "SCORE: " .. SCORE,
  color(1, 1, 1, 1))
score_drawer.scale = score_texture:size()
score_drawer.position.x = -300
score_drawer.position.y = 300
camera_controller:setup(player)
camera_controller:update()
scene_switcher:setup()
scene_switcher:start("")
equipment_menu:setup()

local function draw()
  player:draw()
  for i, v in ipairs(enemies) do v:draw() end
  box:clear()
  local px = math.floor(camera_controller.position.x / TILE_SIZE + 0.5)
  local py = math.floor(camera_controller.position.y / TILE_SIZE + 0.5)
  local size = 6
  for y = py - size, py + size do
    for x = px - size, px + size do
      if (1 <= x and x <= map_size_x and 1 <= y and y <= map_size_y) then
        if map:at(x, y) == MAP_CHIP.WALL then
          box:add(map_draw3ds[y][x].position, map_draw3ds[y][x].rotation,
            vector3(0.5, 0.5, 0.5))
        end
      end
    end
  end
  box:draw()
  sprite:clear()
  for y = py - size, py + size do
    for x = px - size, px + size do
      if (1 <= x and x <= map_size_x and 1 <= y and y <= map_size_y) then
        local p = map_draw3ds[y][x].position:copy()
        p.z = 0.0
        sprite:add(p, map_draw3ds[y][x].rotation,
          map_draw3ds[y][x].scale)
      end
    end
  end
  sprite:draw()
  stair:draw()
  score_drawer:draw()
  equipment_menu:draw()
  GUI_MANAGER:update()
  menu_object:draw()
end

function update()
  if scene_switcher.flag then
    scene_switcher:update(draw)
    return
  end
  menu_object:update()
  if not menu_object.hide then
    draw()
    return
  end
  if equipment_menu:update() then draw() return end
  stair.position.z = dts.sin_0_1(1.0)
  local player_on_map = point2i(0, 0)
  player_on_map.x = math.floor(player.drawer.position.x / TILE_SIZE + 0.5)
  player_on_map.y = math.floor(player.drawer.position.y / TILE_SIZE + 0.5)
  score_font:render_text(score_texture, "SCORE: " .. SCORE,
    color(1, 1, 1, 1))
  score_drawer.scale = score_texture:size()
  score_drawer.position.x = -300
  score_drawer.position.y = 300
  for i, v in ipairs(player.bullets) do
    if map:at(math.floor(v.drawer
      .position
      .x / TILE_SIZE +
      0.5), math.floor(v.drawer.position.y / TILE_SIZE + 0.5)) < MAP_CHIP_WALKABLE then table.remove(player.bullets
        , i)
    end
  end
  for a, b in ipairs(player.orbits) do
    for i, v in ipairs(b.bullets) do
      if map:at(math.floor(v.drawer
        .position
        .x / TILE_SIZE +
        0.5), math.floor(v.drawer.position.y / TILE_SIZE + 0.5)) < MAP_CHIP_WALKABLE then table.remove(b.bullets, i) end
    end
  end
  player:update(map, map_draw3ds, map_size_x, map_size_y)
  if player.hp <= 0 then
    scene_switcher:start("scene03_gameover")
    player:boost_reset()
  end
  if map:at(player_on_map.x, player_on_map.y) == 2 then
    scene_switcher:start("scene01_stage")
    player:boost_reset()
  end
  camera_controller:update()
  draw()
end
