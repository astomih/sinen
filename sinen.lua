---@diagnostic disable: duplicate-index, lowercase-global, unused-local
---@class sn
sn = {
    ---@class Vec3
    ---@field x number
    ---@field y number
    ---@field z number
    ---@field copy fun(self: Vec3): Vec3
    ---@field length fun(self: Vec3): number
    ---@field forward fun(self: Vec3, rotation: Vec3): Vec3
    ---@field normalize fun(self: Vec3): Vec3
    ---@field dot fun(self: Vec3, other: Vec3): number
    ---@field cross fun(self: Vec3, other: Vec3): Vec3
    ---@field lerp fun(self: Vec3, other: Vec3, t: number): Vec3
    ---@field reflect fun(self: Vec3, n: Vec3): Vec3
    ---@operator add(Vec3): Vec3
    ---@operator sub(Vec3): Vec3
    ---@operator mul(Vec3): Vec3
    ---@operator div(Vec3): Vec3
    ---@param x number?
    ---@param y number?
    ---@param z number?
    ---@return Vec3
    Vec3 = function(x, y, z) return {} end,

    ---@param value number
    ---@return Vec3
    ---Initializes a Vec3 with the same value for x, y, z.
    Vec3 = function(value) return {} end,

    ---@class Vec3i
    ---@field x integer
    ---@field y integer
    ---@field z integer
    ---@operator add(Vec3i): Vec3i
    ---@operator sub(Vec3i): Vec3i
    ---@param x integer?
    ---@param y integer?
    ---@param z integer?
    ---@return Vec3i
    Vec3i = function(x, y, z)
        return {}
    end,

    ---@param value integer
    ---@return Vec3i
    ---Initializes a Vec3i with the same value for x, y, z.
    Vec3i = function(value)
        return {}
    end,

    ---@class Vec2
    ---@field x number
    ---@field y number
    ---@field copy fun(self: Vec2): Vec2
    ---@field length fun(self: Vec2): number
    ---@field normalize fun(self: Vec2): Vec2
    ---@field dot fun(self: Vec2, other: Vec2): number
    ---@field lerp fun(self: Vec2, other: Vec2, t: number): Vec2
    ---@field reflect fun(self: Vec2, n: Vec2): Vec2
    ---@operator add(Vec2): Vec2
    ---@operator sub(Vec2): Vec2
    ---@operator mul(Vec2): Vec2
    ---@operator div(Vec2): Vec2
    ---@param x number?
    ---@param y number?
    ---@return Vec2
    Vec2 = function(x, y) return {} end,

    ---@param value number
    ---@return Vec2
    ---Initializes a Vec2 with the same value for x, y.
    Vec2 = function(value) return {} end,

    ---@class Vec2i
    ---@field x integer
    ---@field y integer
    ---@operator add(Vec2i): Vec2i
    ---@operator sub(Vec2i): Vec2i
    ---@param x integer?
    ---@param y integer?
    ---@return Vec2i
    Vec2i = function(x, y) return {} end,

    ---@param value integer
    ---@return Vec2i
    ---Initializes a Vec2i with the same value for x, y.
    Vec2i = function(value) return {} end,

    ---@class Texture
    ---@field fill fun(self: Texture, color: Color)
    ---@field blend fun(self: Texture, color: Color)
    ---@field copy fun(self: Texture): Texture
    ---@field load fun(self: Texture, path: string)
    ---@field size fun(self: Texture): Vec2
    ---@return Texture
    Texture = function() return {} end,

    ---@class Material
    ---Set Texture. index is 1-based, optional.
    ---@field set_texture fun(self:Material, texture: Texture, index: integer?)
    ---@field append_texture fun(self: Material, texture: Texture)
    ---@field clear fun(self: Material)
    ---@field get_texture fun(self: Material, index: integer): Texture
    ---@return Material
    Material = function() return {} end,

    ---@class RenderTexture
    ---@field create fun(self: RenderTexture, x: integer, y: integer)
    ---@return RenderTexture
    RenderTexture = function() return {} end,

    ---@class Music
    ---@field load fun(self: Music, path: string)
    ---@field play fun(self: Music)
    ---@field set_volume fun(self: Music, volume: number)
    ---@return Music
    Music = function() return {} end,

    ---@class Sound
    ---@field load fun(self: Sound, path: string)
    ---@field play fun(self: Sound)
    ---@field set_volume fun(self: Sound, volume: number)
    ---@field set_pitch fun(self: Sound, pitch: number)
    ---@field set_listener fun(self: Sound, position: Vec3)
    ---@field set_position fun(self: Sound, position: Vec3)
    ---@return Sound
    Sound = function() return {} end,

    ---@class Camera
    ---@field lookat fun(self: Camera, position: Vec3, target: Vec3, up: Vec3)
    ---@field perspective fun(self: Camera, fov: number, aspect: number, near: number, far: number)
    ---@field orthographic fun(self: Camera, left: number, right: number, bottom: number, top: number, near: number, far: number)
    ---@field get_position fun(self: Camera): Vec3
    ---@field get_target fun(self: Camera): Vec3
    ---@field get_up fun(self: Camera): Vec3
    ---@field is_aabb_in_frustum fun(self: Camera, aabb: AABB): boolean
    Camera = function() end,

    ---@class Camera2D
    ---@field resize fun(self: Camera2D, size: Vec2)
    ---@field window_ratio fun(self: Camera2D): Vec2
    ---@field inv_window_ratio fun(self: Camera2D): Vec2
    ---@field size fun(self: Camera2D): Vec2
    ---@field half fun(self: Camera2D): Vec2
    Camera2D = function() end,

    ---@class Model
    ---@field get_aabb fun(self: Model): AABB
    ---@field load fun(self: Model, path: string)
    ---@field load_sprite fun(self: Model)
    ---@field load_box fun(self: Model)
    ---@field get_bone_uniform_data fun(self: Model): UniformData
    ---@field play fun(self: Model, positon: number)
    ---@field update fun(self: Model, delta: number)
    ---@return Model
    Model = function() return {} end,

    ---@class AABB
    ---@field min Vec3
    ---@field max Vec3
    ---@field update_world fun(self: AABB, position: Vec3, scale: Vec3, modelAABB: AABB)
    ---@return AABB
    AABB = function() return {} end,

    ---@class Timer
    ---@field start fun(self: Timer)
    ---@field stop fun(self: Timer)
    ---@field is_started fun(self: Timer): boolean
    ---@field set_time fun(self: Timer, time: number)
    ---@field check fun(self: Timer): boolean
    Timer = function() end,

    ---@class UniformData
    ---@field add fun(self: UniformData, value: any)
    ---@field change fun(self: UniformData, index: integer, value: number)
    ---@return UniformData
    UniformData = function() return {} end,

    ---@class Shader
    ---@field load_vertex_shader fun(self: Shader, path: string, uniform_count: integer)
    ---@field load_fragment_shader fun(self: Shader, path: string, uniform_count: integer)
    ---@field compile_load_vertex_shader fun(self: Shader, source: string, uniform_count: integer)
    ---@field compile_load_fragment_shader fun(self: Shader, source: string, uniform_count: integer)
    ---@return Shader
    Shader = function() return {} end,

    ---@class Font
    ---@field load fun(self: Font, size: integer, path: string?): nil
    ---@field render_text fun(self: Font, texture: Texture, text: string, color: Color): Texture
    ---@field resize fun(self: Font, size: integer)
    ---@return Font
    Font = function() return {} end,

    ---@class Color
    ---@field r number
    ---@field g number
    ---@field b number
    ---@field a number
    ---@param r number
    ---@param g number
    ---@param b number
    ---@param a number
    ---@return Color
    Color = function(r, g, b, a) return {} end,

    ---@return Color
    Color = function() return {} end,

    ---@class Draw2D
    ---@field scale Vec2
    ---@field position Vec2
    ---@field rotation number
    ---@field material Material
    ---@field add fun(self: Draw2D, drawable: any)
    ---@field at fun(self: Draw2D, x: number, y: number)
    ---@field clear fun(self: Draw2D)
    ---@param texture Texture?
    ---@return Draw2D
    Draw2D = function(texture) return {} end,

    ---@class Draw3D
    ---@field scale Vec3
    ---@field position Vec3
    ---@field rotation Vec3
    ---@field material Material
    ---@field model Model
    ---@field isDrawDepth boolean
    ---@field add fun(self: Draw3D, positon: Vec3, rotation: Vec3, scale: Vec3)
    ---@field at fun(self: Draw3D, x: number, y: number, z: number)
    ---@field clear fun(self: Draw3D)
    ---@param texture Texture?
    ---@return Draw3D
    Draw3D = function(texture) return {} end,

    ---@class Grid
    ---@field at fun(self: Grid, x: integer, y: integer): integer
    ---@field set fun(self: Grid, x: integer, y: integer, v: integer)
    ---@field width fun(self: Grid): integer
    ---@field height fun(self: Grid): integer
    ---@field size fun(self: Grid): integer
    ---@field clear fun(self: Grid)
    ---@field resize fun(self: Grid, w: integer, h: integer)
    ---@field fill fun(self: Grid, value: integer)
    ---@param w integer
    ---@param h integer
    ---@return Grid
    Grid = function(w, h) return {} end,

    ---@class BFSGrid
    ---@field width fun(self: BFSGrid): integer
    ---@field height fun(self: BFSGrid): integer
    ---@field find_path fun(self: BFSGrid, start: Vec2i, end_: Vec2i): any
    ---@field trace fun(self: BFSGrid): Vec2
    ---@field traceable fun(self: BFSGrid): boolean
    ---@field reset fun(self: BFSGrid)
    ---@param grid Grid
    ---@return BFSGrid
    BFSGrid = function(grid)
        return {}
    end,

    ---@class GraphicsPipeline2D
    ---@field set_vertex_shader fun(self: GraphicsPipeline2D, shader: Shader)
    ---@field set_fragment_shader fun(self: GraphicsPipeline2D, shader: Shader)
    ---@field build fun(self: GraphicsPipeline2D)
    ---@return GraphicsPipeline2D
    GraphicsPipeline2D = function() return {} end,

    ---@class GraphicsPipeline3D
    ---@field set_vertex_shader fun(self: GraphicsPipeline3D, shader: Shader)
    ---@field set_vertex_instanced_shader fun(self: GraphicsPipeline3D, shader: Shader)
    ---@field set_fragment_shader fun(self: GraphicsPipeline3D, shader: Shader)
    ---@field set_animation fun(self: GraphicsPipeline3D, anim: any)
    ---@field build fun(self: GraphicsPipeline3D)
    ---@return GraphicsPipeline3D
    GraphicsPipeline3D = function() return {} end,

    ---@class Rect
    ---@field x number
    ---@field y number
    ---@field width number
    ---@field height number
    ---@param x number?
    ---@param y number?
    ---@param width number?
    ---@param height number?
    ---@return Rect
    Rect = function(x, y, width, height) return {} end,
    ---@param position Vec2
    ---@param size Vec2
    ---@return Rect
    Rect = function(position, size) return {} end,

    ---@class Transform
    ---@field position Vec3
    ---@field rotation Vec3
    ---@field scale Vec3
    ---@return Transform
    Transform = function() return {} end,

    ---@class Collider
    ---@field get_position fun(self: Collider): Vec3
    ---@field get_velocity fun(self: Collider): Vec3
    ---@field set_linear_velocity fun(self: Collider, velocity: Vec3)
    Collider = {},

    ---Static class
    ---@class Script
    ---@field load fun(path: string, basePath?: string)
    Script = {},

    ---Static class
    ---@class FileSystem
    ---@field enumerate_directory fun(path: string): table
    FileSystem = {},

    ---Static class
    ---@class Random
    ---@field get_range fun(a: number, b: number): number
    ---@field get_int_range fun(a: number, b: number): number
    Random = {},

    ---Static class
    ---@class Window
    ---@field get_name fun(): string
    ---@field size fun(): Vec2
    ---@field half fun(): Vec2
    ---@field resize fun(size: Vec2)
    ---@field set_fullscreen fun(full: boolean)
    ---@field rename fun(name: string)
    ---@field resized fun(): boolean
    Window = {},

    ---Static class
    ---@class Graphics
    ---@field draw2d fun(draw2D: Draw2D)
    ---@field draw3d fun(draw3D: Draw3D)
    ---@field draw_rect fun(rect: Rect, color: Color, angle: number?)
    ---@field draw_image fun(texture: Texture, rect: Rect, angle: number?)
    ---@field draw_text fun(text: string, position: Vec2, color: Color?, fontSize: number?, angle: number?)
    ---@field draw_model fun(model: Model, transform: Transform, material: Material)
    ---@field set_camera fun(camera: Camera)
    ---@field get_camera fun(): Camera
    ---@field set_camera2d fun(camera: Camera2D)
    ---@field get_camera2d fun(): Camera2D
    ---@field get_clear_color fun(): Color
    ---@field set_clear_color fun(c: Color)
    ---@field bind_pipeline2d fun(pipe: GraphicsPipeline2D)
    ---@field bind_default_pipeline2d fun()
    ---@field bind_pipeline3d fun(pipe: GraphicsPipeline3D)
    ---@field bind_default_pipeline3d fun()
    ---@field set_uniform_data fun(binding: integer, data: UniformData)
    ---@field set_render_target fun(rt: RenderTexture)
    ---@field flush fun()
    ---@field readback_texture fun(rt: RenderTexture): Texture
    Graphics = {},

    ---Static class
    ---@class Physics
    ---@field create_box_collider fun(transform: Transform, isStatic: boolean): Collider
    ---@field create_sphere_collider fun(position: Vec3, radius: number, isStatic: boolean): Collider
    ---@field add_collider fun(collider: Collider, isActive: boolean)
    Physics = {},

    ---Static class
    ---@class Collision
    ---@field aabb_vs_aabb fun(a: AABB, b: AABB): boolean
    Collision = {},

    ---Static class
    ---@class Keyboard
    ---@field A integer
    ---@field B integer
    ---@field C integer
    ---@field D integer
    ---@field E integer
    ---@field F integer
    ---@field G integer
    ---@field H integer
    ---@field I integer
    ---@field J integer
    ---@field K integer
    ---@field L integer
    ---@field M integer
    ---@field N integer
    ---@field O integer
    ---@field P integer
    ---@field Q integer
    ---@field R integer
    ---@field S integer
    ---@field T integer
    ---@field U integer
    ---@field V integer
    ---@field W integer
    ---@field X integer
    ---@field Y integer
    ---@field Z integer
    ---@field Key0 integer
    ---@field Key1 integer
    ---@field Key2 integer
    ---@field Key3 integer
    ---@field Key4 integer
    ---@field Key5 integer
    ---@field Key6 integer
    ---@field Key7 integer
    ---@field Key8 integer
    ---@field Key9 integer
    ---@field F1 integer
    ---@field F2 integer
    ---@field F3 integer
    ---@field F4 integer
    ---@field F5 integer
    ---@field F6 integer
    ---@field F7 integer
    ---@field F8 integer
    ---@field F9 integer
    ---@field F10 integer
    ---@field F11 integer
    ---@field F12 integer
    ---@field UP integer
    ---@field DOWN integer
    ---@field LEFT integer
    ---@field RIGHT integer
    ---@field ESCAPE integer
    ---@field SPACE integer
    ---@field BACKSPACE integer
    ---@field TAB integer
    ---@field RETURN integer
    ---@field LSHIFT integer
    ---@field RSHIFT integer
    ---@field LCTRL integer
    ---@field RCTRL integer
    ---@field LALT integer
    ---@field is_pressed fun(scancode: integer): boolean
    ---@field is_released fun(scancode: integer): boolean
    ---@field is_down fun(scancode: integer): boolean
    Keyboard = {},

    ---Static class
    ---@class Mouse
    ---@field LEFT integer
    ---@field RIGHT integer
    ---@field MIDDLE integer
    ---@field X1 integer
    ---@field X2 integer
    ---@field is_pressed fun(btn: integer): boolean
    ---@field is_released fun(btn: integer): boolean
    ---@field is_down fun(btn: integer): boolean
    ---@field get_position fun(): Vec2
    ---@field get_position_on_scene fun(): Vec2
    ---@field set_position fun(pos: Vec2)
    ---@field set_position_on_scene fun(pos: Vec2)
    ---@field get_scroll_wheel fun(): number
    ---@field hide_cursor fun(isHide: boolean)
    ---@field set_relative fun(isRelative: boolean)
    Mouse = {},

    ---Static class
    ---@class GamePad
    ---@field INVALID integer
    ---@field A integer
    ---@field B integer
    ---@field X integer
    ---@field Y integer
    ---@field BACK integer
    ---@field GUIDE integer
    ---@field START integer
    ---@field LEFTSTICK integer
    ---@field RIGHTSTICK integer
    ---@field LEFTSHOULDER integer
    ---@field RIGHTSHOULDER integer
    ---@field DPAD_UP integer
    ---@field DPAD_DOWN integer
    ---@field DPAD_LEFT integer
    ---@field DPAD_RIGHT integer
    ---@field MISC1 integer
    ---@field PADDLE1 integer
    ---@field PADDLE2 integer
    ---@field PADDLE3 integer
    ---@field PADDLE4 integer
    ---@field TOUCHPAD integer
    ---@field is_pressed fun(btn: integer): boolean
    ---@field is_released fun(btn: integer): boolean
    ---@field is_down fun(btn: integer): boolean
    ---@field get_left_stick fun(): Vec2
    ---@field get_right_stick fun(): Vec2
    ---@field is_connected fun(): boolean
    Gamepad = {},

    ---Static class
    ---@class Periodic
    ---@field sin0_1 fun(t1: number,t2:number): number
    ---@field cos0_1 fun(t1: number,t2:number): number
    Periodic = {},

    ---Static class
    ---@class Time
    ---@field seconds fun(): number
    ---@field milli fun(): integer
    ---@field deltatime fun(): number
    Time = {},

    ---Static class
    ---@class Logger
    ---@field verbose fun(msg: string)
    ---@field info fun(msg: string)
    ---@field error fun(msg: string)
    ---@field warn fun(msg: string)
    ---@field critical fun(msg: string)
    Logger = {}
}
