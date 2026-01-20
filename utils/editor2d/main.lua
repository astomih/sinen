if os.getenv("LOCAL_LUA_DEBUGGER_VSCODE") == "1" then
    require("lldebugger").start()
end

local scene_io = require("scene_io")
local button = require("gui/button").new()
GUI_MANAGER = require("gui/gui_manager")()

local FONT = sn.Font.new()
FONT:load(18)

local GRID_SIZE = 32.0
local UI_WIDTH = 250.0
local UI_PADDING = 10.0
local UI_LINE = 30.0

---@class EditorEntity
---@field id number
---@field type "rect"
---@field pos {x:number,y:number}
---@field size {x:number,y:number}
---@field color {r:number,g:number,b:number,a:number}
---@field angle number

---@class EditorScene
---@field version number
---@field nextId number
---@field entities EditorEntity[]

---@class EditorState
---@field file string
---@field scene EditorScene
---@field selectedId number?
---@field dragging boolean
---@field dragOffset {x:number,y:number}
---@field dirty boolean
---@field showGrid boolean
---@field snapToGrid boolean
local state = {
    file = "scene.lua",
    scene = {
        version = 1,
        nextId = 1,
        entities = {}
    },
    selectedId = nil,
    dragging = false,
    dragOffset = {
        x = 0,
        y = 0
    },
    dirty = false,
    showGrid = true,
    snapToGrid = true
}

local function round_to_grid(v, grid)
    return math.floor((v / grid) + 0.5) * grid
end

local function snap_pos(pos)
    if not state.snapToGrid then
        return pos
    end
    return {
        x = round_to_grid(pos.x, GRID_SIZE),
        y = round_to_grid(pos.y, GRID_SIZE)
    }
end

local function make_rect_entity(pos)
    local id = state.scene.nextId
    state.scene.nextId = state.scene.nextId + 1
    return {
        id = id,
        type = "rect",
        pos = {
            x = pos.x,
            y = pos.y
        },
        size = {
            x = 160.0,
            y = 96.0
        },
        color = {
            r = 0.8,
            g = 0.8,
            b = 0.85,
            a = 1.0
        },
        angle = 0.0
    }
end

local function find_entity_index_by_id(id)
    for i, e in ipairs(state.scene.entities) do
        if e.id == id then
            return i
        end
    end
    return nil
end

local function get_selected_entity()
    if state.selectedId == nil then
        return nil
    end
    local i = find_entity_index_by_id(state.selectedId)
    if i == nil then
        return nil
    end
    return state.scene.entities[i]
end

local function point_in_rect(center, size, p)
    return p.x >= center.x - size.x / 2 and p.x <= center.x + size.x / 2 and p.y >= center.y - size.y / 2 and p.y <=
        center.y + size.y / 2
end

local function draw_rect_outline(center, size, color, thickness)
    thickness = thickness or 2.0
    local half_w = size.x / 2
    local half_h = size.y / 2

    local top = sn.Vec2.new(center.x, center.y + half_h - thickness / 2)
    local bottom = sn.Vec2.new(center.x, center.y - half_h + thickness / 2)
    local left = sn.Vec2.new(center.x - half_w + thickness / 2, center.y)
    local right = sn.Vec2.new(center.x + half_w - thickness / 2, center.y)

    sn.Graphics.drawRect(sn.Rect.new(top, sn.Vec2.new(size.x, thickness)), color)
    sn.Graphics.drawRect(sn.Rect.new(bottom, sn.Vec2.new(size.x, thickness)), color)
    sn.Graphics.drawRect(sn.Rect.new(left, sn.Vec2.new(thickness, size.y)), color)
    sn.Graphics.drawRect(sn.Rect.new(right, sn.Vec2.new(thickness, size.y)), color)
end

local function draw_grid()
    if not state.showGrid then
        return
    end

    local window = sn.Window.size()
    local lines_x = math.floor(window.x / GRID_SIZE) + 2
    local lines_y = math.floor(window.y / GRID_SIZE) + 2

    local thin = 1.0
    local grid_color = sn.Color.new(0.12, 0.12, 0.12, 1.0)
    local axis_color = sn.Color.new(0.25, 0.25, 0.25, 1.0)

    for i = -lines_x, lines_x do
        local x = i * GRID_SIZE
        sn.Graphics.drawRect(sn.Rect.new(sn.Vec2.new(x, 0), sn.Vec2.new(thin, window.y + GRID_SIZE)), grid_color)
    end
    for i = -lines_y, lines_y do
        local y = i * GRID_SIZE
        sn.Graphics.drawRect(sn.Rect.new(sn.Vec2.new(0, y), sn.Vec2.new(window.x + GRID_SIZE, thin)), grid_color)
    end

    sn.Graphics.drawRect(sn.Rect.new(sn.Vec2.new(0, 0), sn.Vec2.new(thin, window.y + GRID_SIZE)), axis_color)
    sn.Graphics.drawRect(sn.Rect.new(sn.Vec2.new(0, 0), sn.Vec2.new(window.x + GRID_SIZE, thin)), axis_color)
end

local function ui_panel_rect()
    local tl = sn.Window.topLeft()
    local center = {
        x = tl.x + UI_WIDTH / 2 + UI_PADDING,
        y = tl.y - (UI_PADDING + (UI_LINE * 4) / 2)
    }
    local size = {
        x = UI_WIDTH,
        y = UI_LINE * 4 + UI_PADDING * 2
    }
    return center, size
end

local function add_entity_at_mouse()
    local p = sn.Mouse.getPositionOnScene()
    local snapped = snap_pos({
        x = p.x,
        y = p.y
    })
    local e = make_rect_entity(snapped)
    table.insert(state.scene.entities, e)
    state.selectedId = e.id
    state.dirty = true
end

local function delete_selected()
    local i = state.selectedId and find_entity_index_by_id(state.selectedId) or nil
    if i == nil then
        return
    end
    table.remove(state.scene.entities, i)
    state.selectedId = nil
    state.dirty = true
end

local function duplicate_selected()
    local e = get_selected_entity()
    if not e then
        return
    end
    local copy = make_rect_entity({
        x = e.pos.x + GRID_SIZE,
        y = e.pos.y - GRID_SIZE
    })
    copy.size = {
        x = e.size.x,
        y = e.size.y
    }
    copy.color = {
        r = e.color.r,
        g = e.color.g,
        b = e.color.b,
        a = e.color.a
    }
    copy.angle = e.angle
    table.insert(state.scene.entities, copy)
    state.selectedId = copy.id
    state.dirty = true
end

local function save_scene()
    local ok, err = scene_io.save(state.file, state.scene)
    if ok then
        state.dirty = false
        sn.Logger.info("saved: " .. state.file)
    else
        sn.Logger.error("save failed: " .. tostring(err))
    end
end

local function load_scene()
    local scene, err = scene_io.load(state.file)
    if not scene then
        sn.Logger.error("load failed: " .. tostring(err))
        return
    end
    if type(scene.entities) ~= "table" then
        sn.Logger.error("load failed: invalid scene (missing entities)")
        return
    end
    state.scene = scene
    state.scene.version = state.scene.version or 1
    state.scene.nextId = state.scene.nextId or (#state.scene.entities + 1)
    state.selectedId = nil
    state.dragging = false
    state.dirty = false
    sn.Logger.info("loaded: " .. state.file)
end

function setup()
    sn.Window.rename("Sinen Scene Editor (simple)")
end

function update()
    sn.Graphics.getCamera2d():resize(sn.Window.size())
    GUI_MANAGER:update()

    local tl = sn.Window.topLeft()
    local panel_center, panel_size = ui_panel_rect()
    local mpos = sn.Mouse.getPositionOnScene()
    local in_ui = point_in_rect(panel_center, panel_size, {
        x = mpos.x,
        y = mpos.y
    })

    local ui_x = tl.x + UI_PADDING + UI_WIDTH / 2
    local ui_y = tl.y - UI_PADDING - UI_LINE / 2

    if button:show("New (N)", sn.Vec2.new(ui_x, ui_y), sn.Vec2.new(UI_WIDTH, UI_LINE)) then
        add_entity_at_mouse()
    end
    ui_y = ui_y - UI_LINE
    if button:show("Save (Ctrl+S)", sn.Vec2.new(ui_x, ui_y), sn.Vec2.new(UI_WIDTH, UI_LINE)) then
        save_scene()
    end
    ui_y = ui_y - UI_LINE
    if button:show("Load (Ctrl+L)", sn.Vec2.new(ui_x, ui_y), sn.Vec2.new(UI_WIDTH, UI_LINE)) then
        load_scene()
    end
    ui_y = ui_y - UI_LINE
    local snap_label = state.snapToGrid and "Snap: ON (G)" or "Snap: OFF (G)"
    if button:show(snap_label, sn.Vec2.new(ui_x, ui_y), sn.Vec2.new(UI_WIDTH, UI_LINE)) then
        state.snapToGrid = not state.snapToGrid
    end

    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end

    if sn.Keyboard.isPressed(sn.Keyboard.N) then
        add_entity_at_mouse()
    end

    local ctrl = sn.Keyboard.isDown(sn.Keyboard.LCTRL) or sn.Keyboard.isDown(sn.Keyboard.RCTRL)
    if ctrl and sn.Keyboard.isPressed(sn.Keyboard.S) then
        save_scene()
    end
    if ctrl and sn.Keyboard.isPressed(sn.Keyboard.L) then
        load_scene()
    end

    if sn.Keyboard.isPressed(sn.Keyboard.G) then
        state.snapToGrid = not state.snapToGrid
    end
    if sn.Keyboard.isPressed(sn.Keyboard.BACKSPACE) then
        delete_selected()
    end
    if sn.Keyboard.isPressed(sn.Keyboard.D) then
        duplicate_selected()
    end

    local selected = get_selected_entity()
    if selected and not in_ui then
        local wheel = sn.Mouse.getScrollWheel()
        if wheel ~= 0 then
            selected.size.x = math.max(8.0, selected.size.x + wheel * 8.0)
            selected.size.y = math.max(8.0, selected.size.y + wheel * 8.0)
            state.dirty = true
        end
    end

    if sn.Mouse.isPressed(sn.Mouse.LEFT) and not in_ui then
        local clicked_id = nil
        for i = #state.scene.entities, 1, -1 do
            local e = state.scene.entities[i]
            if point_in_rect(e.pos, e.size, {
                    x = mpos.x,
                    y = mpos.y
                }) then
                clicked_id = e.id
                break
            end
        end

        state.selectedId = clicked_id
        if clicked_id then
            local e = get_selected_entity()
            if (e ~= nil)
            then
                state.dragging = true
                state.dragOffset = {
                    x = e.pos.x - mpos.x,
                    y = e.pos.y - mpos.y
                }
            end
        else
            state.dragging = false
        end
    end

    if state.dragging and sn.Mouse.isDown(sn.Mouse.LEFT) then
        local e = get_selected_entity()
        if e then
            local next_pos = {
                x = mpos.x + state.dragOffset.x,
                y = mpos.y + state.dragOffset.y
            }
            next_pos = snap_pos(next_pos)
            e.pos.x = next_pos.x
            e.pos.y = next_pos.y
            state.dirty = true
        else
            state.dragging = false
        end
    end

    if state.dragging and sn.Mouse.isReleased(sn.Mouse.LEFT) then
        state.dragging = false
    end
end

function draw()
    draw_grid()

    for _, e in ipairs(state.scene.entities) do
        if e.type == "rect" then
            local c = sn.Color.new(e.color.r, e.color.g, e.color.b, e.color.a)
            sn.Graphics
                .drawRect(sn.Rect.new(sn.Vec2.new(e.pos.x, e.pos.y), sn.Vec2.new(e.size.x, e.size.y)), c, e.angle)
        end
    end

    local selected = get_selected_entity()
    if selected then
        draw_rect_outline(selected.pos, selected.size, sn.Color.new(1, 1, 0.2, 1), 2.0)
    end

    GUI_MANAGER:draw()

    local bl = sn.Window.bottomLeft()
    local help =
    "LMB: select/drag | MouseWheel: resize | N: new | D: duplicate | Backspace: delete | Ctrl+S: save | Ctrl+L: load | G: snap"
    sn.Graphics.drawText(help, FONT, sn.Vec2.new(bl.x + 10, bl.y + 10), sn.Color.new(1, 1, 1, 0.9), 18)

    local tl = sn.Window.topLeft()
    local dirty = state.dirty and "*" or ""
    local status = string.format("File: %s%s | Entities: %d", state.file, dirty, #state.scene.entities)
    sn.Graphics.drawText(status, FONT, sn.Vec2.new(tl.x + UI_PADDING, tl.y - (UI_LINE * 4 + UI_PADDING * 2 + 10)),
        sn.Color.new(1, 1, 1, 0.9), 18)
end
