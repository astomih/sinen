---@class Vec3
---@field x number
---@field y number
---@field z number
---@field Copy fun(self: Vec3): Vec3
---@field Length fun(self: Vec3): number
---@field Forward fun(self: Vec3, rotation: Vec3): Vec3
---@field Normalize fun(self: Vec3): Vec3
---@field Dot fun(self: Vec3, other: Vec3): number
---@field Cross fun(self: Vec3, other: Vec3): Vec3
---@field Lerp fun(self: Vec3, other: Vec3, t: number): Vec3
---@field Reflect fun(self: Vec3, n: Vec3): Vec3
---@operator add(Vec3): Vec3
---@operator sub(Vec3): Vec3
---@operator mul(Vec3): Vec3
---@operator div(Vec3): Vec3
---@param x number?
---@param y number?
---@param z number?
---@return Vec3
function Vec3(x, y, z) return {} end

---@param value number
---@return Vec3
---Initializes a Vec3 with the same value for x, y, z.
function Vec3(value) return {} end

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
function Vec3i(x, y, z)
    return {}
end

---@param value integer
---@return Vec3i
---Initializes a Vec3i with the same value for x, y, z.
function Vec3i(value)
    return {}
end

---@class Vec2
---@field x number
---@field y number
---@field Copy fun(self: Vec2): Vec2
---@field Length fun(self: Vec2): number
---@field Normalize fun(self: Vec2): Vec2
---@field Dot fun(self: Vec2, other: Vec2): number
---@field Lerp fun(self: Vec2, other: Vec2, t: number): Vec2
---@field Reflect fun(self: Vec2, n: Vec2): Vec2
---@operator add(Vec2): Vec2
---@operator sub(Vec2): Vec2
---@operator mul(Vec2): Vec2
---@operator div(Vec2): Vec2
---@param x number?
---@param y number?
---@return Vec2
function Vec2(x, y) return {} end

---@param value number
---@return Vec2
---Initializes a Vec2 with the same value for x, y.
function Vec2(value) return {} end

---@class Vec2i
---@field x integer
---@field y integer
---@operator add(Vec2i): Vec2i
---@operator sub(Vec2i): Vec2i
---@param x integer?
---@param y integer?
---@return Vec2i
function Vec2i(x, y) return {} end

---@param value integer
---@return Vec2i
---Initializes a Vec2i with the same value for x, y.
function Vec2i(value) return {} end

---@class Texture
---@field FillColor fun(self: Texture, color: Color)
---@field BlendColor fun(self: Texture, color: Color)
---@field Copy fun(self: Texture): Texture
---@field Load fun(self: Texture, path: string)
---@field Size fun(self: Texture): Vec2
---@return Texture
function Texture() return {} end

---@class Material
---Set Texture. index is 1-based, optional.
---@field SetTexture fun(self:Material, texture: Texture, index: integer?)
---@field AppendTexture fun(self: Material, texture: Texture)
---@field Clear fun(self: Material)
---@field GetTexture fun(self: Material, index: integer): Texture
---@return Material
function Material() return {} end

---@class RenderTexture
---@field Create fun(self: RenderTexture, x: integer, y: integer)
---@return RenderTexture
function RenderTexture() return {} end

---@class Music
---@field Load fun(self: Music, path: string)
---@field Play fun(self: Music)
---@field SetVolume fun(self: Music, volume: number)
---@return Music
function Music() return {} end

---@class Sound
---@field Load fun(self: Sound, path: string)
---@field Play fun(self: Sound)
---@field SetVolume fun(self: Sound, volume: number)
---@field SetPitch fun(self: Sound, pitch: number)
---@field SetListener fun(self: Sound, position: Vec3)
---@field SetPosition fun(self: Sound, position: Vec3)
---@return Sound
function Sound() return {} end

---@class Camera
---@field LookAt fun(self: Camera, position: Vec3, target: Vec3, up: Vec3)
---@field Perspective fun(self: Camera, fov: number, aspect: number, near: number, far: number)
---@field Orthographic fun(self: Camera, left: number, right: number, bottom: number, top: number, near: number, far: number)
---@field GetPosition fun(self: Camera): Vec3
---@field GetTarget fun(self: Camera): Vec3
---@field GetUp fun(self: Camera): Vec3
---@field IsAABBInFrustum fun(self: Camera, aabb: AABB): boolean
function Camera() end

---@class Model
---@field GetAABB fun(self: Model): AABB
---@field Load fun(self: Model, path: string)
---@field LoadSprite fun(self: Model)
---@field LoadBox fun(self: Model)
---@field GetBoneUniformData fun(self: Model): UniformData
---@field Play fun(self: Model, positon: number)
---@field Update fun(self: Model, delta: number)
---@return Model
function Model() return {} end

---@class AABB
---@field min Vec3
---@field max Vec3
---@field UpdateWorld fun(self: AABB, position: Vec3, scale: Vec3, modelAABB: AABB)
---@return AABB
function AABB() return {} end

---@class Timer
---@field Start fun(self: Timer)
---@field Stop fun(self: Timer)
---@field IsStarted fun(self: Timer): boolean
---@field SetTime fun(self: Timer, time: number)
---@field Check fun(self: Timer): boolean
function Timer() end

---@class UniformData
---@field Add fun(self: UniformData, value: any)
---@field Change fun(self: UniformData, index: integer, value: number)
---@return UniformData
function UniformData() return {} end

---@class Shader
---@field LoadVertexShader fun(self: Shader, path: string, uniform_count: integer)
---@field LoadFragmentShader fun(self: Shader, path: string, uniform_count: integer)
---@field CompileAndLoadVertexShader fun(self: Shader, source: string, uniform_count: integer)
---@field CompileAndLoadFragmentShader fun(self: Shader, source: string, uniform_count: integer)
---@return Shader
function Shader() return {} end

---@class Font
---@field Load fun(self: Font, size: integer, path: string?): nil
---@field RenderText fun(self: Font, texture: Texture, text: string, color: Color): Texture
---@field Resize fun(self: Font, size: integer)
---@return Font
function Font() return {} end

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
function Color(r, g, b, a) return {} end

---@return Color
function Color() return {} end

---@class Draw2D
---@field scale Vec2
---@field position Vec2
---@field rotation number
---@field material Material
---@field Draw fun(self: Draw2D)
---@field Add fun(self: Draw2D, drawable: any)
---@field At fun(self: Draw2D, x: number, y: number)
---@field Clear fun(self: Draw2D)
---@param texture Texture?
---@return Draw2D
function Draw2D(texture) return {} end

---@class Draw3D
---@field scale Vec3
---@field position Vec3
---@field rotation Vec3
---@field material Material
---@field model Model
---@field isDrawDepth boolean
---@field Draw fun(self: Draw3D)
---@field Add fun(self: Draw3D, positon: Vec3, rotation: Vec3, scale: Vec3)
---@field At fun(self: Draw3D, x: number, y: number, z: number)
---@field Clear fun(self: Draw3D)
---@param texture Texture?
---@return Draw3D
function Draw3D(texture) return {} end

---@class Grid
---@field At fun(self: Grid, x: integer, y: integer): integer
---@field Set fun(self: Grid, x: integer, y: integer, v: integer)
---@field Width fun(self: Grid): integer
---@field Height fun(self: Grid): integer
---@field Size fun(self: Grid): integer
---@field Clear fun(self: Grid)
---@field Resize fun(self: Grid, w: integer, h: integer)
---@field Fill fun(self: Grid, value: integer)
---@param w integer
---@param h integer
---@return Grid
function Grid(w, h) return {} end

---@class BFSGrid
---@field Width fun(self: BFSGrid): integer
---@field Height fun(self: BFSGrid): integer
---@field FindPath fun(self: BFSGrid, start: Vec2, end_: Vec2): any
---@field Trace fun(self: BFSGrid): Vec2
---@field Traceable fun(self: BFSGrid): boolean
---@field Reset fun(self: BFSGrid)
---@param grid Grid
---@return BFSGrid
function BFSGrid(grid)
    return {}
end

---@class GraphicsPipeline2D
---@field SetVertexShader fun(self: GraphicsPipeline2D, shader: Shader)
---@field SetFragmentShader fun(self: GraphicsPipeline2D, shader: Shader)
---@field Build fun(self: GraphicsPipeline2D)
---@return GraphicsPipeline2D
function GraphicsPipeline2D() return {} end

---@class GraphicsPipeline3D
---@field SetVertexShader fun(self: GraphicsPipeline3D, shader: Shader)
---@field SetVertexInstancedShader fun(self: GraphicsPipeline3D, shader: Shader)
---@field SetFragmentShader fun(self: GraphicsPipeline3D, shader: Shader)
---@field SetAnimation fun(self: GraphicsPipeline3D, anim: any)
---@field Build fun(self: GraphicsPipeline3D)
---@return GraphicsPipeline3D
function GraphicsPipeline3D() return {} end

---Static class
---@class Random
---@field GetRange fun(a: number, b: number): number
---@field GetIntRange fun(a: number, b: number): number
Random = {}

---Static class
---@class Window
---@field GetName fun(): string
---@field Size fun(): Vec2
---@field Half fun(): Vec2
---@field Resize fun(size: Vec2)
---@field SetFullscreen fun(full: boolean)
---@field Rename fun(name: string)
---@field Resized fun(): boolean
Window = {}

---Static class
---@class Graphics
---@field GetClearColor fun(): Color
---@field SetClearColor fun(c: Color)
---@field BindPipeline2D fun(pipe: GraphicsPipeline2D)
---@field BindDefaultPipeline2D fun()
---@field BindPipeline3D fun(pipe: GraphicsPipeline3D)
---@field BindDefaultPipeline3D fun()
---@field SetUniformData fun(binding: integer, data: UniformData)
---@field BeginTarget2D fun(rt: RenderTexture)
---@field BeginTarget3D fun(rt: RenderTexture)
---@field EndTarget fun(rt: RenderTexture, texture_ref: Texture)
Graphics = {}

---Static class
---@class Scene
---@field GetCamera fun(): Camera
---@field Size fun(): Vec2
---@field Resize fun(size: Vec2)
---@field Half fun(): Vec2
---@field Ratio fun(): number
---@field InvRatio fun(): number
---@field DeltaTime fun(): number
---@field Change fun(name: string)
Scene = {}

---Static class
---@class Collision
---@field AABBvsAABB fun(a: AABB, b: AABB): boolean
Collision = {}

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
---@field IsPressed fun(scancode: integer): boolean
---@field IsReleased fun(scancode: integer): boolean
---@field IsDown fun(scancode: integer): boolean
Keyboard = {}

---Static class
---@class Mouse
---@field LEFT integer
---@field RIGHT integer
---@field MIDDLE integer
---@field X1 integer
---@field X2 integer
---@field IsPressed fun(btn: integer): boolean
---@field IsReleased fun(btn: integer): boolean
---@field IsDown fun(btn: integer): boolean
---@field GetPosition fun(): Vec2
---@field GetPositionOnScene fun(): Vec2
---@field SetPosition fun(pos: Vec2)
---@field SetPositionOnScene fun(pos: Vec2)
---@field GetScrollWheel fun(): number
---@field HideCursor fun(isHide: boolean)
---@field SetRelative fun(isRelative: boolean)
Mouse = {}

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
---@field IsPressed fun(btn: integer): boolean
---@field IsReleased fun(btn: integer): boolean
---@field IsDown fun(btn: integer): boolean
---@field GetLeftStick fun(): Vec2
---@field GetRightStick fun(): Vec2
---@field IsConnected fun(): boolean
Gamepad = {}

---Static class
---@class Periodic
---@field Sin0_1 fun(t1: number,t2:number): number
---@field Cos0_1 fun(t1: number,t2:number): number
Periodic = {}

---Static class
---@class Time
---@field Seconds fun(): number
---@field Milli fun(): integer
Time = {}

---Static class
---@class Logger
---@field Verbose fun(msg: string)
---@field Info fun(msg: string)
---@field Error fun(msg: string)
---@field Warn fun(msg: string)
---@field Critical fun(msg: string)
Logger = {}
