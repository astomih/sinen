---@diagnostic disable: duplicate-index
---@class sn
sn = {
    ---@class sn.Vec3
    ---@field x number
    ---@field y number
    ---@field z number
    ---@field new fun(x: number, y: number, z: number): sn.Vec3
    ---@field new fun(value: number): sn.Vec3
    ---@field copy fun(self: sn.Vec3): sn.Vec3
    ---@field length fun(self: sn.Vec3): number
    ---@field forward fun(self: sn.Vec3, rotation: sn.Vec3): sn.Vec3
    ---@field normalize fun(self: sn.Vec3): sn.Vec3
    ---@field dot fun(self: sn.Vec3, other: sn.Vec3): number
    ---@field cross fun(self: sn.Vec3, other: sn.Vec3): sn.Vec3
    ---@field lerp fun(self: sn.Vec3, other: sn.Vec3, t: number): sn.Vec3
    ---@field reflect fun(self: sn.Vec3, n: sn.Vec3): sn.Vec3
    ---@operator add(sn.Vec3): sn.Vec3
    ---@operator sub(sn.Vec3): sn.Vec3
    ---@operator mul(sn.Vec3): sn.Vec3
    ---@operator div(sn.Vec3): sn.Vec3
    Vec3 = {},

    ---@class sn.Vec3i
    ---@field x integer
    ---@field y integer
    ---@field z integer
    ---@field new fun(x: integer, y: integer, z: integer): sn.Vec3i
    ---@field new fun(value: integer): sn.Vec3i
    ---@operator add(sn.Vec3i): sn.Vec3i
    ---@operator sub(sn.Vec3i): sn.Vec3i
    Vec3i = {},

    ---@class sn.Vec2
    ---@field x number
    ---@field y number
    ---@field new fun(value: number): sn.Vec2
    ---@field new fun(x: number, y: number): sn.Vec2
    ---@field copy fun(self: sn.Vec2): sn.Vec2
    ---@field length fun(self: sn.Vec2): number
    ---@field normalize fun(self: sn.Vec2): sn.Vec2
    ---@field dot fun(self: sn.Vec2, other: sn.Vec2): number
    ---@field lerp fun(self: sn.Vec2, other: sn.Vec2, t: number): sn.Vec2
    ---@field reflect fun(self: sn.Vec2, n: sn.Vec2): sn.Vec2
    ---@operator add(sn.Vec2): sn.Vec2
    ---@operator sub(sn.Vec2): sn.Vec2
    ---@operator mul(sn.Vec2): sn.Vec2
    ---@operator div(sn.Vec2): sn.Vec2
    Vec2 = {},

    ---@class sn.Vec2i
    ---@field x integer
    ---@field y integer
    ---@field new fun(x: integer, y: integer): sn.Vec2i
    ---@field new fun(value: integer): sn.Vec2i
    ---@operator add(sn.Vec2i): sn.Vec2i
    ---@operator sub(sn.Vec2i): sn.Vec2i
    Vec2i = {},

    ---@class sn.Texture
    ---@field new fun(): sn.Texture
    ---@field new fun(width: integer, height: integer): sn.Texture
    ---@field fill fun(self: sn.Texture, color: sn.Color)
    ---@field blend fun(self: sn.Texture, color: sn.Color)
    ---@field copy fun(self: sn.Texture): sn.Texture
    ---@field load fun(self: sn.Texture, path: string)
    ---@field size fun(self: sn.Texture): sn.Vec2
    Texture = {},

    ---@class sn.Cubemap
    ---@field new fun(): sn.Cubemap
    ---@field load fun(self: sn.Cubemap, path: string)
    Cubemap = {},

    ---@class sn.Material
    ---@field new fun(): sn.Material
    ---Set Texture. index is 1-based, optional.
    ---@field setTexture fun(self:sn.Material, texture: sn.Texture, index: integer?)
    ---@field appendTexture fun(self: sn.Material, texture: sn.Texture)
    ---@field clear fun(self: sn.Material)
    ---@field getTexture fun(self: sn.Material, index: integer): sn.Texture
    ---@field setCubemap fun(self: sn.Material, cubemap: sn.Cubemap, index: integer)
    Material = {},

    ---@class sn.RenderTexture
    ---@field new fun(): sn.RenderTexture
    ---@field create fun(self: sn.RenderTexture, x: integer, y: integer)
    RenderTexture = {},

    ---@class sn.Music
    ---@field new fun():sn.Music
    ---@field load fun(self: sn.Music, path: string)
    ---@field play fun(self: sn.Music)
    ---@field setVolume fun(self: sn.Music, volume: number)
    Music = {},

    ---@class sn.Sound
    ---@field new fun(): sn.Sound
    ---@field load fun(self: sn.Sound, path: string)
    ---@field play fun(self: sn.Sound)
    ---@field setVolume fun(self: sn.Sound, volume: number)
    ---@field setPitch fun(self: sn.Sound, pitch: number)
    ---@field setListener fun(self: sn.Sound, position: sn.Vec3)
    ---@field setPosition fun(self: sn.Sound, position: sn.Vec3)
    Sound = {},

    ---@class sn.Camera
    ---@field new fun(): sn.Camera
    ---@field lookat fun(self: sn.Camera, position: sn.Vec3, target: sn.Vec3, up: sn.Vec3)
    ---@field perspective fun(self: sn.Camera, fov: number, aspect: number, near: number, far: number)
    ---@field orthographic fun(self: sn.Camera, width: number, height: number, near: number, far: number)
    ---@field getPosition fun(self: sn.Camera): sn.Vec3
    ---@field getTarget fun(self: sn.Camera): sn.Vec3
    ---@field getUp fun(self: sn.Camera): sn.Vec3
    ---@field isAABBInFrustum fun(self: sn.Camera, aabb: sn.AABB): boolean
    Camera = {},

    ---@class sn.Camera2D
    ---@field new fun(): sn.Camera2D
    ---@field resize fun(self: sn.Camera2D, size: sn.Vec2)
    ---@field windowRatio fun(self: sn.Camera2D): sn.Vec2
    ---@field invWindowRatio fun(self: sn.Camera2D): sn.Vec2
    ---@field size fun(self: sn.Camera2D): sn.Vec2
    ---@field half fun(self: sn.Camera2D): sn.Vec2
    Camera2D = {},

    ---@class sn.Model
    ---@field new fun(): sn.Model
    ---@field getAABB fun(self: sn.Model): sn.AABB
    ---@field load fun(self: sn.Model, path: string)
    ---@field loadSprite fun(self: sn.Model)
    ---@field loadBox fun(self: sn.Model)
    ---@field getBoneUniformData fun(self: sn.Model): sn.UniformData
    ---@field play fun(self: sn.Model, positon: number)
    ---@field update fun(self: sn.Model, delta: number)
    ---@field getMaterial fun(self: sn.Model): sn.Material
    Model = {},

    ---@class sn.AABB
    ---Constructor
    ---@field new fun(): sn.AABB
    ---@field min sn.Vec3
    ---@field max sn.Vec3
    ---@field updateWorld fun(self: sn.AABB, position: sn.Vec3, scale: sn.Vec3, modelsn.AABB: sn.AABB)
    AABB = {},

    ---@class sn.Timer
    ---@field new fun(): sn.Timer
    ---@field start fun(self: sn.Timer)
    ---@field stop fun(self: sn.Timer)
    ---@field isStarted fun(self: sn.Timer): boolean
    ---@field setTime fun(self: sn.Timer, time: number)
    ---@field check fun(self: sn.Timer): boolean
    Timer = {},

    ---@class sn.UniformData
    ---@field new fun(): sn.UniformData
    ---@field add fun(self: sn.UniformData, value: any)
    ---@field change fun(self: sn.UniformData, value: number,  index: integer)
    ---@field addCamera fun(self: sn.UniformData, camera : sn.Camera)
    ---@field addVec3 fun(self: sn.UniformData, sn.Vec3 : sn.Vec3)
    UniformData = {},

    ---@class sn.Shader
    ---@field new fun(): sn.Shader
    ---@field loadVertexShader fun(self: sn.Shader, path: string, uniform_count: integer)
    ---@field loadFragmentShader fun(self: sn.Shader, path: string, uniform_count: integer)
    ---@field compileLoadVertexShader fun(self: sn.Shader, source: string)
    ---@field compileLoadFragmentShader fun(self: sn.Shader, source: string)
    Shader = {},

    ---@class sn.Font
    ---@field new fun(): sn.Font
    ---@field load fun(self: sn.Font, size: integer, path: string?): nil
    ---@field renderText fun(self: sn.Font, texture: sn.Texture, text: string, color: sn.Color): sn.Texture
    ---@field resize fun(self: sn.Font, size: integer)
    Font = {},

    ---@class sn.Color
    ---@field r number
    ---@field g number
    ---@field b number
    ---@field a number
    ---@field new fun(r: number, g: number, b: number, a: number): sn.Color
    ---@field new fun(value: number, a: number): sn.Color
    ---@field new fun(value: number): sn.Color
    ---@field new fun(): sn.Color
    Color = {},

    ---@class sn.Draw2D
    ---@field new fun(): sn.Draw2D
    ---@field new fun(texture: sn.Texture): sn.Draw2D
    ---@field scale sn.Vec2
    ---@field position sn.Vec2
    ---@field rotation number
    ---@field material sn.Material
    ---@field add fun(self: sn.Draw2D, drawable: any)
    ---@field at fun(self: sn.Draw2D, x: number, y: number)
    ---@field clear fun(self: sn.Draw2D)
    Draw2D = {},

    ---@class sn.Draw3D
    ---@field new fun(): sn.Draw3D
    ---@field new fun(texture: sn.Texture): sn.Draw3D
    ---@field scale sn.Vec3
    ---@field position sn.Vec3
    ---@field rotation sn.Vec3
    ---@field material sn.Material
    ---@field model sn.Model
    ---@field add fun(self: sn.Draw3D, positon: sn.Vec3, rotation: sn.Vec3, scale: sn.Vec3)
    ---@field at fun(self: sn.Draw3D, x: number, y: number, z: number)
    ---@field clear fun(self: sn.Draw3D)
    Draw3D = {},

    ---@class sn.Grid
    ---@field new fun(w: integer, h: integer): sn.Grid
    ---@field at fun(self: sn.Grid, x: integer, y: integer): integer
    ---@field set fun(self: sn.Grid, x: integer, y: integer, v: integer)
    ---@field width fun(self: sn.Grid): integer
    ---@field height fun(self: sn.Grid): integer
    ---@field size fun(self: sn.Grid): integer
    ---@field clear fun(self: sn.Grid)
    ---@field resize fun(self: sn.Grid, w: integer, h: integer)
    ---@field fill fun(self: sn.Grid, value: integer)
    Grid = {},

    ---@class sn.BFSGrid
    ---@field new fun(grid: sn.Grid): sn.BFSGrid
    ---@field width fun(self: sn.BFSGrid): integer
    ---@field height fun(self: sn.BFSGrid): integer
    ---@field findPath fun(self: sn.BFSGrid, start: sn.Vec2i, end_: sn.Vec2i): any
    ---@field trace fun(self: sn.BFSGrid): sn.Vec2
    ---@field traceable fun(self: sn.BFSGrid): boolean
    ---@field reset fun(self: sn.BFSGrid)
    BFSGrid = {},

    ---@class sn.GraphicsPipeline
    ---@field new fun(): sn.GraphicsPipeline
    ---@field setVertexShader fun(self: sn.GraphicsPipeline, shader: sn.Shader)
    ---@field setFragmentShader fun(self: sn.GraphicsPipeline, shader: sn.Shader)
    ---@field setEnableDepthTest fun(self: sn.GraphicsPipeline, enable: boolean)
    ---@field setEnableInstanced fun(self: sn.GraphicsPipeline, enable: boolean)
    ---@field setEnableAnimation fun(self: sn.GraphicsPipeline, enable: boolean)
    ---@field setEnableTangent fun(self: sn.GraphicsPipeline, enable: boolean)
    ---@field build fun(self: sn.GraphicsPipeline)
    GraphicsPipeline = {},

    ---@class sn.Rect
    ---@field x number
    ---@field y number
    ---@field new fun(x: number, y: number, width: number, height: number):sn.Rect
    ---@field new fun(position: sn.Vec2, size: sn.Vec2):sn.Rect
    ---@field width number
    ---@field height number
    Rect = {},

    ---@class sn.Transform
    ---@field position sn.Vec3
    ---@field rotation sn.Vec3
    ---@field scale sn.Vec3
    ---@field new fun(): sn.Transform
    Transform = {},

    ---@class sn.Collider
    ---@field getPosition fun(self: sn.Collider): sn.Vec3
    ---@field getVelocity fun(self: sn.Collider): sn.Vec3
    ---@field setLinearVelocity fun(self: sn.Collider, velocity: sn.Vec3)
    Collider = {},

    ---Static class
    ---@class sn.Script
    ---@field load fun(path: string, basePath?: string)
    Script = {},

    ---Static class
    ---@class sn.FileSystem
    ---@field enumerateDirectory fun(path: string): table
    FileSystem = {},

    ---Static class
    ---@class sn.Random
    ---@field getRange fun(a: number, b: number): number
    ---@field getIntRange fun(a: number, b: number): number
    Random = {},

    ---Static class
    ---@class sn.Window
    ---@field getName fun(): string
    ---@field size fun(): sn.Vec2
    ---@field half fun(): sn.Vec2
    ---@field resize fun(size: sn.Vec2)
    ---@field setFullscreen fun(full: boolean)
    ---@field rename fun(name: string)
    ---@field resized fun(): boolean
    Window = {},

    ---Static class
    ---@class sn.BuiltinPipelines
    ---@field get3D fun(): sn.GraphicsPipeline
    ---@field get3DInstanced fun(): sn.GraphicsPipeline
    ---@field get2D fun(): sn.GraphicsPipeline
    BuiltinPipelines = {},

    ---Static class
    ---@class sn.Graphics
    ---@field draw2D fun(draw2D: sn.Draw2D)
    ---@field draw3D fun(draw3D: sn.Draw3D)
    ---@field drawRect fun(rect:sn.Rect, color: sn.Color, angle: number?)
    ---@field drawImage fun(texture: sn.Texture, rect:sn.Rect, angle: number?)
    ---@field drawText fun(text: string, position: sn.Vec2, color: sn.Color?, fontSize: number?, angle: number?)
    ---@field drawModel fun(model: sn.Model, transform: sn.Transform, material: sn.Material)
    ---@field drawModelInstanced fun(model: sn.Model, transforms: table, material: sn.Material)
    ---@field drawCubemap fun(cubemap: sn.Cubemap)
    ---@field setCamera fun(camera: sn.Camera)
    ---@field getCamera fun(): sn.Camera
    ---@field setCamera2d fun(camera: sn.Camera2D)
    ---@field getCamera2d fun(): sn.Camera2D
    ---@field getClearColor fun(): sn.Color
    ---@field setClearColor fun(c: sn.Color)
    ---@field bindPipeline fun(pipe: sn.GraphicsPipeline)
    ---@field setUniformData fun(binding: integer, data: sn.UniformData)
    ---@field setRenderTarget fun(rt: sn.RenderTexture)
    ---@field flush fun()
    ---@field readbackTexture fun(rt: sn.RenderTexture, out: sn.Texture): Texture
    Graphics = {},

    ---Static class
    ---@class sn.Physics
    ---@field createBoxCollider fun(transform: sn.Transform, isStatic: boolean): sn.Collider
    ---@field createSphereCollider fun(position: sn.Vec3, radius: number, isStatic: boolean): sn.Collider
    ---@field addCollider fun(collider: sn.Collider, isActive: boolean)
    Physics = {},

    ---Static class
    ---@class sn.Collision
    ---@field AABBvsAABB fun(a: sn.AABB, b: sn.AABB): boolean
    Collision = {},

    ---Static class
    ---@class sn.Keyboard
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

    ---Static class
    ---@class sn.Mouse
    ---@field LEFT integer
    ---@field RIGHT integer
    ---@field MIDDLE integer
    ---@field X1 integer
    ---@field X2 integer
    ---@field isPressed fun(btn: integer): boolean
    ---@field isReleased fun(btn: integer): boolean
    ---@field isDown fun(btn: integer): boolean
    ---@field getPosition fun(): sn.Vec2
    ---@field getPositionOnScene fun(): sn.Vec2
    ---@field setPosition fun(pos: sn.Vec2)
    ---@field setPositionOnScene fun(pos: sn.Vec2)
    ---@field getScrollWheel fun(): number
    ---@field hideCursor fun(isHide: boolean)
    ---@field setRelative fun(isRelative: boolean)
    Mouse = {},

    ---Static class
    ---@class sn.GamePad
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
    ---@field getLeftStick fun(): sn.Vec2
    ---@field getRightStick fun(): sn.Vec2
    ---@field isConnected fun(): boolean
    Gamepad = {},

    ---Static class
    ---@class sn.Periodic
    ---@field sin0_1 fun(t1: number,t2:number): number
    ---@field cos0_1 fun(t1: number,t2:number): number
    Periodic = {},

    ---Static class
    ---@class sn.Time
    ---@field seconds fun(): number
    ---@field milli fun(): integer
    ---@field delta fun(): number
    Time = {},

    ---Static class
    ---@class sn.Logger
    ---@field verbose fun(msg: string)
    ---@field info fun(msg: string)
    ---@field error fun(msg: string)
    ---@field warn fun(msg: string)
    ---@field critical fun(msg: string)
    Logger = {}
}
