---@diagnostic disable: duplicate-index, unused-local
---@class sn
sn = {
    ---@class Vec3
    ---@field x number
    ---@field y number
    ---@field z number
    ---@field new fun(x: number, y: number, z: number): Vec3
    ---@field new fun(value: number): Vec3
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
    Vec3 = {},
    Vec3 = function()
        return nil
    end,

    ---@class Vec3i
    ---@field x integer
    ---@field y integer
    ---@field z integer
    ---@field new fun(x: integer, y: integer, z: integer): Vec3i
    ---@field new fun(value: integer): Vec3i
    ---@operator add(Vec3i): Vec3i
    ---@operator sub(Vec3i): Vec3i
    Vec3i = {},
    Vec3i = function()
        return nil
    end,

    ---@class Vec2
    ---@field x number
    ---@field y number
    ---@field new fun(value: number): Vec2
    ---@field new fun(x: number, y: number): Vec2
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
    Vec2 = {},
    Vec2 = function()
        return nil
    end,

    ---@class Vec2i
    ---@field x integer
    ---@field y integer
    ---@field new fun(x: integer, y: integer): Vec2i
    ---@field new fun(value: integer): Vec2i
    ---@operator add(Vec2i): Vec2i
    ---@operator sub(Vec2i): Vec2i
    Vec2i = {},
    Vec2i = function()
        return nil
    end,

    ---@class Texture
    ---@field new fun(): Texture
    ---@field new fun(width: integer, height: integer): Texture
    ---@field fill fun(self: Texture, color: Color)
    ---@field blend fun(self: Texture, color: Color)
    ---@field copy fun(self: Texture): Texture
    ---@field load fun(self: Texture, path: string)
    ---@field size fun(self: Texture): Vec2
    Texture = {},
    Texture = function()
        return nil
    end,

    ---@class Cubemap
    ---@field new fun(): Cubemap
    ---@field load fun(self: Cubemap, path: string)
    Cubemap = {},
    Cubemap = function()
        return nil
    end,

    ---@class Material
    ---@field new fun(): Material
    ---Set Texture. index is 1-based, optional.
    ---@field setTexture fun(self:Material, texture: Texture, index: integer?)
    ---@field appendTexture fun(self: Material, texture: Texture)
    ---@field clear fun(self: Material)
    ---@field getTexture fun(self: Material, index: integer): Texture
    ---@field setCubemap fun(self: Material, cubemap: Cubemap, index: integer)
    Material = {},
    Material = function()
        return nil
    end,

    ---@class RenderTexture
    ---@field new fun(): RenderTexture
    ---@field create fun(self: RenderTexture, x: integer, y: integer)
    RenderTexture = {},
    RenderTexture = function()
        return nil
    end,

    ---@class Music
    ---@field new fun():Music
    ---@field load fun(self: Music, path: string)
    ---@field play fun(self: Music)
    ---@field setVolume fun(self: Music, volume: number)
    Music = {},
    Music = function()
        return nil
    end,

    ---@class Sound
    ---@field new fun(): Sound
    ---@field load fun(self: Sound, path: string)
    ---@field play fun(self: Sound)
    ---@field setVolume fun(self: Sound, volume: number)
    ---@field setPitch fun(self: Sound, pitch: number)
    ---@field setListener fun(self: Sound, position: Vec3)
    ---@field setPosition fun(self: Sound, position: Vec3)
    Sound = {},
    Sound = function()
        return nil
    end,

    ---@class Camera
    ---@field new fun(): Camera
    ---@field lookat fun(self: Camera, position: Vec3, target: Vec3, up: Vec3)
    ---@field perspective fun(self: Camera, fov: number, aspect: number, near: number, far: number)
    ---@field orthographic fun(self: Camera, width: number, height: number, near: number, far: number)
    ---@field getPosition fun(self: Camera): Vec3
    ---@field getTarget fun(self: Camera): Vec3
    ---@field getUp fun(self: Camera): Vec3
    ---@field isAABBInFrustum fun(self: Camera, aabb: AABB): boolean
    Camera = {},
    Camera = function()
        return nil
    end,

    ---@class Camera2D
    ---@field new fun(): Camera2D
    ---@field resize fun(self: Camera2D, size: Vec2)
    ---@field windowRatio fun(self: Camera2D): Vec2
    ---@field invWindowRatio fun(self: Camera2D): Vec2
    ---@field size fun(self: Camera2D): Vec2
    ---@field half fun(self: Camera2D): Vec2
    Camera2D = {},
    Camera2D = function()
        return nil
    end,

    ---@class Model
    ---@field new fun(): Model
    ---@field getAABB fun(self: Model): AABB
    ---@field load fun(self: Model, path: string)
    ---@field loadSprite fun(self: Model)
    ---@field loadBox fun(self: Model)
    ---@field getBoneUniformData fun(self: Model): UniformData
    ---@field play fun(self: Model, positon: number)
    ---@field update fun(self: Model, delta: number)
    ---@field getMaterial fun(self: Model): Material
    Model = {},
    Model = function()
        return nil
    end,

    ---@class AABB
    ---Constructor
    ---@field new fun(): AABB
    ---@field min Vec3
    ---@field max Vec3
    ---@field updateWorld fun(self: AABB, position: Vec3, scale: Vec3, modelAABB: AABB)
    AABB = {},
    AABB = function()
        return nil
    end,

    ---@class Timer
    ---@field new fun(): Timer
    ---@field start fun(self: Timer)
    ---@field stop fun(self: Timer)
    ---@field isStarted fun(self: Timer): boolean
    ---@field setTime fun(self: Timer, time: number)
    ---@field check fun(self: Timer): boolean
    Timer = {},
    Timer = function()
        return nil
    end,

    ---@class UniformData
    ---@field new fun(): UniformData
    ---@field add fun(self: UniformData, value: any)
    ---@field change fun(self: UniformData, value: number,  index: integer)
    ---@field addCamera fun(self: UniformData, camera : Camera)
    ---@field addVec3 fun(self: UniformData, vec3 : Vec3)
    UniformData = {},
    UniformData = function()
        return nil
    end,

    ---@class Shader
    ---@field new fun(): Shader
    ---@field loadVertexShader fun(self: Shader, path: string, uniform_count: integer)
    ---@field loadFragmentShader fun(self: Shader, path: string, uniform_count: integer)
    ---@field compileLoadVertexShader fun(self: Shader, source: string)
    ---@field compileLoadFragmentShader fun(self: Shader, source: string)
    Shader = {},
    Shader = function()
        return nil
    end,

    ---@class Font
    ---@field new fun(): Font
    ---@field load fun(self: Font, size: integer, path: string?): nil
    ---@field renderText fun(self: Font, texture: Texture, text: string, color: Color): Texture
    ---@field resize fun(self: Font, size: integer)
    Font = {},
    Font = function()
        return nil
    end,

    ---@class Color
    ---@field r number
    ---@field g number
    ---@field b number
    ---@field a number
    ---@field new fun(r: number, g: number, b: number, a: number): Color
    ---@field new fun(value: number, a: number): Color
    ---@field new fun(value: number): Color
    ---@field new fun(): Color
    Color = {},
    Color = function()
        return nil
    end,

    ---@class Draw2D
    ---@field new fun(): Draw2D
    ---@field new fun(texture: Texture): Draw2D
    ---@field scale Vec2
    ---@field position Vec2
    ---@field rotation number
    ---@field material Material
    ---@field add fun(self: Draw2D, drawable: any)
    ---@field at fun(self: Draw2D, x: number, y: number)
    ---@field clear fun(self: Draw2D)
    Draw2D = {},
    Draw2D = function()
        return nil
    end,

    ---@class Draw3D
    ---@field new fun(): Draw3D
    ---@field new fun(texture: Texture): Draw3D
    ---@field scale Vec3
    ---@field position Vec3
    ---@field rotation Vec3
    ---@field material Material
    ---@field model Model
    ---@field add fun(self: Draw3D, positon: Vec3, rotation: Vec3, scale: Vec3)
    ---@field at fun(self: Draw3D, x: number, y: number, z: number)
    ---@field clear fun(self: Draw3D)
    Draw3D = {},
    Draw3D = function()
        return nil
    end,

    ---@class Grid
    ---@field new fun(w: integer, h: integer): Grid
    ---@field at fun(self: Grid, x: integer, y: integer): integer
    ---@field set fun(self: Grid, x: integer, y: integer, v: integer)
    ---@field width fun(self: Grid): integer
    ---@field height fun(self: Grid): integer
    ---@field size fun(self: Grid): integer
    ---@field clear fun(self: Grid)
    ---@field resize fun(self: Grid, w: integer, h: integer)
    ---@field fill fun(self: Grid, value: integer)
    Grid = {},
    Grid = function()
        return nil
    end,

    ---@class BFSGrid
    ---@field new fun(grid: Grid): BFSGrid
    ---@field width fun(self: BFSGrid): integer
    ---@field height fun(self: BFSGrid): integer
    ---@field findPath fun(self: BFSGrid, start: Vec2i, end_: Vec2i): any
    ---@field trace fun(self: BFSGrid): Vec2
    ---@field traceable fun(self: BFSGrid): boolean
    ---@field reset fun(self: BFSGrid)
    BFSGrid = {},
    BFSGrid = function()
        return nil
    end,

    ---@class GraphicsPipeline
    ---@field new fun(): GraphicsPipeline
    ---@field setVertexShader fun(self: GraphicsPipeline, shader: Shader)
    ---@field setFragmentShader fun(self: GraphicsPipeline, shader: Shader)
    ---@field setEnableDepthTest fun(self: GraphicsPipeline, enable: boolean)
    ---@field setEnableInstanced fun(self: GraphicsPipeline, enable: boolean)
    ---@field setEnableAnimation fun(self: GraphicsPipeline, enable: boolean)
    ---@field setEnableTangent fun(self: GraphicsPipeline, enable: boolean)
    ---@field build fun(self: GraphicsPipeline)
    GraphicsPipeline = {},
    GraphicsPipeline = function()
        return nil
    end,

    ---@class Rect
    ---@field x number
    ---@field y number
    ---@field new fun(x: number, y: number, width: number, height: number): Rect
    ---@field new fun(position: Vec2, size: Vec2): Rect
    ---@field width number
    ---@field height number
    Rect = {},
    Rect = function()
        return nil
    end,

    ---@class Transform
    ---@field position Vec3
    ---@field rotation Vec3
    ---@field scale Vec3
    ---@field new fun(): Transform
    Transform = {},
    Transform = function()
        return nil
    end,

    ---@class Collider
    ---@field getPosition fun(self: Collider): Vec3
    ---@field getVelocity fun(self: Collider): Vec3
    ---@field setLinearVelocity fun(self: Collider, velocity: Vec3)
    Collider = {},
    Collider = function()
        return nil
    end,

    ---Static class
    ---@class Script
    ---@field load fun(path: string, basePath?: string)
    Script = {},
    Script = function()
        return nil
    end,

    ---Static class
    ---@class FileSystem
    ---@field enumerateDirectory fun(path: string): table
    FileSystem = {},
    FileSystem = function()
        return nil
    end,

    ---Static class
    ---@class Random
    ---@field getRange fun(a: number, b: number): number
    ---@field getIntRange fun(a: number, b: number): number
    Random = {},
    Random = function()
        return nil
    end,

    ---Static class
    ---@class Window
    ---@field getName fun(): string
    ---@field size fun(): Vec2
    ---@field half fun(): Vec2
    ---@field resize fun(size: Vec2)
    ---@field setFullscreen fun(full: boolean)
    ---@field rename fun(name: string)
    ---@field resized fun(): boolean
    Window = {},
    Window = function()
        return nil
    end,

    ---Static class
    ---@class BuiltinPipelines
    ---@field get3D fun(): GraphicsPipeline
    ---@field get3DInstanced fun(): GraphicsPipeline
    ---@field get2D fun(): GraphicsPipeline
    BuiltinPipelines = {},
    BuiltinPipelines = function()
        return nil
    end,

    ---Static class
    ---@class Graphics
    ---@field draw2D fun(draw2D: Draw2D)
    ---@field draw3D fun(draw3D: Draw3D)
    ---@field drawRect fun(rect: Rect, color: Color, angle: number?)
    ---@field drawImage fun(texture: Texture, rect: Rect, angle: number?)
    ---@field drawText fun(text: string, position: Vec2, color: Color?, fontSize: number?, angle: number?)
    ---@field drawModel fun(model: Model, transform: Transform, material: Material)
    ---@field drawModelInstanced fun(model: Model, transforms: table, material: Material)
    ---@field drawCubemap fun(cubemap: Cubemap)
    ---@field setCamera fun(camera: Camera)
    ---@field getCamera fun(): Camera
    ---@field setCamera2d fun(camera: Camera2D)
    ---@field getCamera2d fun(): Camera2D
    ---@field getClearColor fun(): Color
    ---@field setClearColor fun(c: Color)
    ---@field bindPipeline fun(pipe: GraphicsPipeline)
    ---@field setUniformData fun(binding: integer, data: UniformData)
    ---@field setRenderTarget fun(rt: RenderTexture)
    ---@field flush fun()
    ---@field readbackTexture fun(rt: RenderTexture, out: Texture): Texture
    Graphics = {},
    Graphics = function()
        return nil
    end,

    ---Static class
    ---@class Physics
    ---@field createBoxCollider fun(transform: Transform, isStatic: boolean): Collider
    ---@field createSphereCollider fun(position: Vec3, radius: number, isStatic: boolean): Collider
    ---@field addCollider fun(collider: Collider, isActive: boolean)
    Physics = {},
    Physics = function()
        return nil
    end,

    ---Static class
    ---@class Collision
    ---@field AABBvsAABB fun(a: AABB, b: AABB): boolean
    Collision = {},
    Collision = function()
        return nil
    end,

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
    ---@field isPressed fun(scancode: integer): boolean
    ---@field isReleased fun(scancode: integer): boolean
    ---@field isDown fun(scancode: integer): boolean
    Keyboard = {},
    Keyboard = function()
        return nil
    end,

    ---Static class
    ---@class Mouse
    ---@field LEFT integer
    ---@field RIGHT integer
    ---@field MIDDLE integer
    ---@field X1 integer
    ---@field X2 integer
    ---@field isPressed fun(btn: integer): boolean
    ---@field isReleased fun(btn: integer): boolean
    ---@field isDown fun(btn: integer): boolean
    ---@field getPosition fun(): Vec2
    ---@field getPositionOnScene fun(): Vec2
    ---@field setPosition fun(pos: Vec2)
    ---@field setPositionOnScene fun(pos: Vec2)
    ---@field getScrollWheel fun(): number
    ---@field hideCursor fun(isHide: boolean)
    ---@field setRelative fun(isRelative: boolean)
    Mouse = {},
    Mouse = function()
        return nil
    end,

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
    ---@field isPressed fun(btn: integer): boolean
    ---@field isReleased fun(btn: integer): boolean
    ---@field isDown fun(btn: integer): boolean
    ---@field getLeftStick fun(): Vec2
    ---@field getRightStick fun(): Vec2
    ---@field isConnected fun(): boolean
    Gamepad = {},
    Gamepad = function()
        return nil
    end,

    ---Static class
    ---@class Periodic
    ---@field sin0_1 fun(t1: number,t2:number): number
    ---@field cos0_1 fun(t1: number,t2:number): number
    Periodic = {},
    Periodic = function()
        return nil
    end,

    ---Static class
    ---@class Time
    ---@field seconds fun(): number
    ---@field milli fun(): integer
    ---@field delta fun(): number
    Time = {},
    Time = function()
        return nil
    end,

    ---Static class
    ---@class Logger
    ---@field verbose fun(msg: string)
    ---@field info fun(msg: string)
    ---@field error fun(msg: string)
    ---@field warn fun(msg: string)
    ---@field critical fun(msg: string)
    Logger = {},
    Logger = function()
        return nil
    end
}
