local player = require "player"
local enemy = require "enemy"
local enemies = {}
local enemy_max_num = 100
local world = require "world"
local map_size_x = 64
local map_size_y = 64
local map = grid(map_size_x, map_size_y)
local map_chip = {
    floor = 0,
    wall = 1,
    stair = 2,
    key = 3,
    player = 4,
}
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

local score_font = font()
local score_texture = texture()
local score_drawer = draw2d(score_texture)
tile:load("tile.png")
tree:load("tree.sim", "tree")
local stair_model = model()
stair_model:load("stair.sim", "stair")

local menu = require("gui/menu")
local menu_object = menu()
local scene_switcher = require("scene_switcher")()


-- key object
local key = {}
local key_model = model()
key_model:load("key.sim", "key")
local key_texture = texture()
key_texture:fill_color(color(1, 1, 1, 1))
local key_texture_2d = texture()
key_texture_2d:load("key.png")
local key_drawer2d = draw2d(key_texture_2d)
key_drawer2d.scale = key_texture_2d:size()
key_drawer2d.scale.x = key_drawer2d.scale.x / 6
key_drawer2d.scale.y = key_drawer2d.scale.y / 6
key_drawer2d.position = vector2(-window.size().x / TILE_SIZE + key_drawer2d.scale.x / TILE_SIZE, -window.size().y / 3)
local key_drawer = draw3d(key_texture)
key_drawer.scale = vector3(0.25, 0.25, 0.25)
key_drawer.position = vector3(0, 0, 1)
key_drawer.rotation = vector3(90, 0, 0)
key_drawer.vertex_name = "key"
local key_hit = false

function Setup()
    score_font:load(DEFAULT_FONT_NAME, 64)
    menu_object:setup()
    music:load("Stage1.ogg")
    music:play()
    DEFAULT_TEXTURE = texture()
    DEFAULT_TEXTURE:fill_color(color(1, 1, 1, 1))
    map:fill(0)
    dts.dungeon_generator(map)

    box = draw3d_instanced(DEFAULT_TEXTURE)
    box.vertex_name = "tree"
    sprite = draw3d_instanced(tile)
    sprite.is_draw_depth = false
    stair = draw3d(DEFAULT_TEXTURE)
    stair.vertex_name = "stair"
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
            if map:at(x, y) ~= map_chip.stair then
                sprite:add(map_draw3ds[y][x].position, map_draw3ds[y][x].rotation,
                    map_draw3ds[y][x].scale)
            end
            if map:at(x, y) == map_chip.wall then
                map_draw3ds[y][x].position.z = 0.5
                map_draw3ds[y][x].aabb = aabb()
                map_draw3ds[y][x].aabb.max =
                map_draw3ds[y][x].position:add(map_draw3ds[y][x].scale)
                map_draw3ds[y][x].aabb.min =
                map_draw3ds[y][x].position:sub(map_draw3ds[y][x].scale)
                map_draw3ds[y][x].scale.z = 3


            end
            if map:at(x, y) == map_chip.stair then
                stair.position.x = x * TILE_SIZE
                stair.position.y = y * TILE_SIZE + 0.5
                stair.position.z = 100
            end
            -- key object
            if map:at(x, y) == map_chip.key then
                key_drawer.position.x = x * TILE_SIZE
                key_drawer.position.y = y * TILE_SIZE
            end
            if map:at(x, y) == map_chip.player then
                player.drawer.position.x = x * TILE_SIZE
                player.drawer.position.y = y * TILE_SIZE
            end
        end
    end
    scene_switcher:setup()
    scene_switcher:start(true, "")
end

local function camera_update()
    scene.main_camera():lookat(vector3(player.drawer.position.x, player.drawer.position.y - 0.5,
        player.drawer.position.z + 15), player.drawer.position, vector3(0, 0, 1))
end

local function draw()
    player:draw()
    for i, v in ipairs(enemies) do v:draw() end
    box:clear()
    local px = math.floor(player.drawer.position.x / TILE_SIZE + 0.5)
    local py = math.floor(player.drawer.position.y / TILE_SIZE + 0.5)
    local size = 5
    for y = py - size, py + size do
        for x = px - size, px + size do
            if (1 <= x and x <= map_size_x and 1 <= y and y <= map_size_y) then
                if map:at(x, y) == map_chip.wall then
                    box:add(map_draw3ds[y][x].position, map_draw3ds[y][x].rotation,
                        map_draw3ds[y][x].scale)
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
    if not key_hit then
        key_drawer:draw()
    end
    if key_hit then
        key_drawer2d:draw()
    end
    score_drawer:draw()
    menu_object:draw()
end

function Update()
    if scene_switcher.flag then
        scene_switcher:update(draw)
        return
    end
    menu_object:update()
    if not menu_object.hide then
        draw()
        return
    end
    key_drawer.rotation.y = key_drawer.rotation.y + delta_time * 100
    score_font:render_text(score_texture, "SCORE: " .. SCORE,
        color(1, 1, 1, 1))
    score_drawer.scale = score_texture:size()
    score_drawer.position.x = -300
    score_drawer.position.y = 300
    for i, v in ipairs(player.bullets) do
        for j, w in ipairs(enemies) do
            if collision.aabb_aabb(v.aabb, w.aabb) then
                local efk = effect()
                efk:setup()
                for k = 1, efk.max_particles do
                    efk.worlds[k].position = w.drawer.position:copy()
                end
                efk:play()
                table.insert(player.efks, efk)

                table.remove(player.bullets, i)
                -- hp
                w.hp = w.hp - 10
                if w.hp < 0 then
                    SCORE = SCORE + 10
                    table.remove(enemies, j)
                end
                if #enemies <= 0 then
                    stair.position.z = -2

                end
            end
        end
        if map:at(math.floor(v.drawer
            .position
            .x / TILE_SIZE +
            0.5), math.floor(v.drawer.position.y / TILE_SIZE + 0.5)) ==
            1 then table.remove(player.bullets, i) end
    end
    for a, b in ipairs(player.orbits) do
        for i, v in ipairs(b.bullets) do
            for j, w in ipairs(enemies) do
                if collision.aabb_aabb(v.aabb, w.aabb) then
                    local efk = effect()
                    efk:setup()
                    for k = 1, efk.max_particles do
                        efk.worlds[k].position = w.drawer.position:copy()
                    end
                    efk:play()
                    table.insert(b.efks, efk)

                    table.remove(b.bullets, i)
                    -- hp
                    w.hp = w.hp - 10
                    if w.hp < 0 then
                        SCORE = SCORE + 10
                        table.remove(enemies, j)
                    end
                    if #enemies <= 0 then
                        stair.position.z = -2

                    end
                end
            end
            if map:at(math.floor(v.drawer
                .position
                .x / TILE_SIZE +
                0.5), math.floor(v.drawer.position.y / TILE_SIZE + 0.5)) ==
                1 then table.remove(b.bullets, i) end
        end
    end
    player:update(map, map_draw3ds, map_size_x, map_size_y)
    -- Player hit key
    if math.floor(player.drawer.position.x + 0.5) == math.floor(key_drawer.position.x) and
        math.floor(player.drawer.position.y + 0.5) == math.floor(key_drawer.position.y) then
        key_hit = true
        stair.position.z = -2.0
    end
    for i, v in ipairs(enemies) do
        v:update(player)
        v:player_collision(player)
    end
    if math.floor(player.drawer.position.x + 0.5) == math.floor(stair.position.x) and
        math.floor(player.drawer.position.y + 0.5) == math.floor(stair.position.y) then

        if keyboard:key_state(keyENTER) == buttonPRESSED and stair.position.z ==
            -2 then
            NOW_STAGE = NOW_STAGE + 1
            if NOW_STAGE == 4 then
                scene_switcher:start(false, "scene02_clear")
            else
                scene_switcher:start(false, "scene01_stage")
            end
        end
    end
    if player.hp <= 0 then
        scene_switcher:start(false, "scene03_gameover")
    end
    camera_update()
    draw()
end
