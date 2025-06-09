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
function Vec3(x, y, z) return {} end

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
function Vec2(x, y) return {} end

---@class Texture
---@field fill_color fun(self: Texture, color: Color)
---@field blend_color fun(self: Texture, color: Color)
---@field copy fun(self: Texture): Texture
---@field load fun(self: Texture, path: string)
---@field size fun(self: Texture): Vec2
function Texture() end

---@class Material
---@field append fun(self: Material, texture: Texture)
---@field clear fun(self: Material)
---@field get_texture fun(self: Material, index: integer): Texture
function Material() end

---@class RenderTexture
---@field create fun(self: RenderTexture, x: integer, y: integer)
function RenderTexture() end

---@class Music
---@field load fun(self: Music, path: string)
---@field play fun(self: Music)
---@field set_volume fun(self: Music, volume: number)
function Music() end

---@class Sound
---@field load fun(self: Sound, path: string)
---@field play fun(self: Sound)
---@field set_volume fun(self: Sound, volume: number)
---@field set_pitch fun(self: Sound, pitch: number)
---@field set_listener fun(self: Sound, position: Vec3)
---@field set_position fun(self: Sound, position: Vec3)
function Sound() end

---@class Camera
---@field lookat fun(self: Camera, position: Vec3, target: Vec3, up: Vec3)
---@field perspective fun(self: Camera, fov: number, aspect: number, near: number, far: number)
---@field orthographic fun(self: Camera, left: number, right: number, bottom: number, top: number, near: number, far: number)
---@field position fun(self: Camera): Vec3
---@field target fun(self: Camera): Vec3
---@field up fun(self: Camera): Vec3
---@field is_aabb_in_frustum fun(self: Camera, aabb: AABB): boolean
function Camera() end

---@class Model
---@field aabb fun(self: Model): AABB
---@field load fun(self: Model, path: string)
---@field load_sprite fun(self: Model, path: string)
---@field load_box fun(self: Model, size: Vec3)
---@field bone_uniform_data fun(self: Model): UniformData
---@field play fun(self: Model, name: string)
---@field update fun(self: Model, delta: number)
function Model() end

---@class AABB
---@field min Vec3
---@field max Vec3
---@field update_world fun(self: AABB, mat: any)
function AABB() end

---@class Timer
---@field start fun(self: Timer)
---@field stop fun(self: Timer)
---@field is_started fun(self: Timer): boolean
---@field set_time fun(self: Timer, time: number)
---@field check fun(self: Timer): boolean
function Timer() end

---@class UniformData
---@field add fun(self: UniformData, value: any)
---@field change fun(self: UniformData, index: integer, value: number)
function UniformData() end

---@class Shader
---@field load_vertex_shader fun(self: Shader, path: string, uniform_count: integer)
---@field load_fragment_shader fun(self: Shader, path: string, uniform_count: integer)
---@field compile_and_load_vertex_shader fun(self: Shader, source: string, uniform_count: integer)
---@field compile_and_load_fragment_shader fun(self: Shader, source: string, uniform_count: integer)
function Shader() end

---@class Font
---@field load fun(self: Font, size: integer, path: string?): nil
---@field render_text fun(self: Font, texture: Texture, text: string, color: Color): Texture
---@field resize fun(self: Font, size: integer)
function Font() end

---@class Color
---@field r number
---@field g number
---@field b number
---@field a number
---@param r number?
---@param g number?
---@param b number?
---@param a number?
---@return Color
function Color(r, g, b, a) return {} end

---@class Draw2D
---@field scale Vec2
---@field position Vec2
---@field rotation number
---@field material Material
---@field draw fun(self: Draw2D)
---@field add fun(self: Draw2D, drawable: any)
---@field at fun(self: Draw2D, x: number, y: number)
---@field clear fun(self: Draw2D)
function Draw2D() end

---@class Draw3D
---@field scale Vec3
---@field position Vec3
---@field rotation Vec3
---@field material Material
---@field model Model
---@field is_draw_depth boolean
---@field draw fun(self: Draw3D)
---@field add fun(self: Draw3D, drawable: any)
---@field at fun(self: Draw3D, x: number, y: number, z: number)
---@field clear fun(self: Draw3D)
function Draw3D() end

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
function Grid(w, h) return {} end

---@class BFSGrid
---@field width fun(self: BFSGrid): integer
---@field height fun(self: BFSGrid): integer
---@field find_path fun(self: BFSGrid, start: Vec2, end_: Vec2): any
---@field trace fun(self: BFSGrid): Vec2
---@field traceable fun(self: BFSGrid): boolean
---@field reset fun(self: BFSGrid)
---@param grid Grid
---@return BFSGrid
function BFSGrid(grid)
  return {}
end

---@class GraphicsPipeline2D
---@field set_vertex_shader fun(self: GraphicsPipeline2D, shader: Shader)
---@field set_fragment_shader fun(self: GraphicsPipeline2D, shader: Shader)
---@field build fun(self: GraphicsPipeline2D)
function GraphicsPipeline2D() end

---@class GraphicsPipeline3D
---@field set_vertex_shader fun(self: GraphicsPipeline3D, shader: Shader)
---@field set_vertex_instanced_shader fun(self: GraphicsPipeline3D, shader: Shader)
---@field set_fragment_shader fun(self: GraphicsPipeline3D, shader: Shader)
---@field set_animation fun(self: GraphicsPipeline3D, anim: any)
---@field build fun(self: GraphicsPipeline3D)
function GraphicsPipeline3D() end

---@class Random
---@field get_int_range fun(a: integer, b: integer): integer
---@field get_float_range fun(a: number, b: number): number

---@class Window
---@field name fun(): string
---@field size fun(): Vec2
---@field half fun(): Vec2
---@field resize fun(size: Vec2)
---@field set_fullscreen fun(full: boolean)
---@field rename fun(name: string)
---@field resized fun(): boolean

---@class Graphics
---@field clear_color fun(): Color
---@field set_clear_color fun(c: Color)
---@field at_render_texture_user_data fun(rt: RenderTexture): any
---@field bind_pipeline2d fun(pipe: GraphicsPipeline2D)
---@field bind_default_pipeline2d fun()
---@field bind_pipeline3d fun(pipe: GraphicsPipeline3D)
---@field bind_default_pipeline3d fun()
---@field set_uniform_data fun(binding: integer, data: UniformData)
---@field begin_target2d fun(rt: RenderTexture)
---@field begin_target3d fun(rt: RenderTexture)
---@field end_target fun(rt: RenderTexture, texture_ref: Texture)
Graphics = {}

---@class Scene
---@field camera fun(): Camera
---@field size fun(): Vec2
---@field resize fun(size: Vec2)
---@field half fun(): Vec2
---@field ratio fun(): number
---@field inv_ratio fun(): number
---@field delta_time fun(): number
---@field change fun(name: string)
Scene = {}

---@class Collision
---@field aabb_aabb fun(a: AABB, b: AABB): boolean
Collision = {}

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
Keyboard = {}

---@class Mouse
---@field LEFT integer
---@field RIGHT integer
---@field MIDDLE integer
---@field X1 integer
---@field X2 integer
---@field is_pressed fun(btn: integer): boolean
---@field is_released fun(btn: integer): boolean
---@field is_down fun(btn: integer): boolean
---@field position fun(): Vec2
---@field position_on_scene fun(): Vec2
---@field set_position fun(pos: Vec2)
---@field set_position_on_scene fun(pos: Vec2)
---@field scroll_wheel fun(): number
---@field hide_cursor fun()
Mouse = {}

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
---@field left_stick fun(): Vec2
---@field right_stick fun(): Vec2
---@field is_connected fun(): boolean
Gamepad = {}

---@class Periodic
---@field sin0_1 fun(time: number): number
---@field cos0_1 fun(time: number): number

---@class Time
---@field seconds fun(): number
---@field milli fun(): integer

---@class Logger
---@field verbose fun(msg: string)
---@field debug fun(msg: string)
---@field info fun(msg: string)
---@field error fun(msg: string)
---@field warn fun(msg: string)
---@field critical fun(msg: string)
