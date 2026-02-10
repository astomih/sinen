---@diagnostic disable: duplicate-index, lowercase-global

--- Tutorial
--- Sinen is read lua files that defined setup()/update()/draw(). But these function is optional.
--- In 2D rendering, screen origin is center. So, +y equals top of screen direction. This is the same as math coordinates.
---  Of course, some people will not like this, so we provide some helper functions such as sn.Rect:leftTop().

sn = {
    ---@class sn.Buffer
    ---@field new fun(array: table): sn.Buffer
    Buffer = {},

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

    ---@class sn.Texture
    ---@field new fun(): sn.Texture
    ---@field new fun(width: number, height: number): sn.Texture
    ---@field fill fun(self: sn.Texture, color: sn.Color)
    ---@field blend fun(self: sn.Texture, color: sn.Color)
    ---@field copy fun(self: sn.Texture): sn.Texture
    ---@field load fun(self: sn.Texture, path: string)
    ---@field loadCubemap fun(self: sn.Texture, path: string)
    ---@field loadFromPath fun(self: sn.Texture, path: string)
    ---@field size fun(self: sn.Texture): sn.Vec2
    Texture = {},

    ---@class sn.RenderTexture
    ---@field new fun(): sn.RenderTexture
    ---@field create fun(self: sn.RenderTexture, x: number, y: number)
    RenderTexture = {},

    ---@class sn.Sound
    ---@field new fun(): sn.Sound
    ---@field load fun(self: sn.Sound, path: string)
    ---@field loadFromPath fun(self: sn.Sound, path: string)
    ---@field play fun(self: sn.Sound)
    ---@field setVolume fun(self: sn.Sound, volume: number)
    ---@field setPitch fun(self: sn.Sound, pitch: number)
    ---@field setListener fun(self: sn.Sound, position: sn.Vec3)
    ---@field setPosition fun(self: sn.Sound, position: sn.Vec3)
    Sound = {},

    ---@class sn.Camera3D
    ---@field new fun(): sn.Camera3D
    ---@field lookat fun(self: sn.Camera3D, position: sn.Vec3, target: sn.Vec3, up: sn.Vec3)
    ---@field perspective fun(self: sn.Camera3D, fov: number, aspect: number, near: number, far: number)
    ---@field orthographic fun(self: sn.Camera3D, width: number, height: number, near: number, far: number)
    ---@field getPosition fun(self: sn.Camera3D): sn.Vec3
    ---@field getTarget fun(self: sn.Camera3D): sn.Vec3
    ---@field getUp fun(self: sn.Camera3D): sn.Vec3
    ---@field isAABBInFrustum fun(self: sn.Camera3D, aabb: sn.AABB): boolean
    Camera3D = {},

    ---@class sn.Camera2D
    ---@field new fun(): sn.Camera2D
    ---@field resize fun(self: sn.Camera2D, size: sn.Vec2)
    ---@field windowRatio fun(self: sn.Camera2D): sn.Vec2
    ---@field invWindowRatio fun(self: sn.Camera2D): sn.Vec2
    ---@field size fun(self: sn.Camera2D): sn.Vec2
    ---@field half fun(self: sn.Camera2D): sn.Vec2
    Camera2D = {},

    ---@class sn.TextureKey
    ---@field BaseColor number
    ---@field Normal number
    ---@field DiffuseRoughness number
    ---@field Metalness number
    ---@field Emissive number
    ---@field LightMap number
    TextureKey = {},

    ---@class sn.Model
    ---@field new fun(): sn.Model
    ---@field getAABB fun(self: sn.Model): sn.AABB
    ---@field load fun(self: sn.Model, path: string)
    ---@field load fun(self: sn.Model, buffer: sn.Buffer)
    ---@field loadSprite fun(self: sn.Model)
    ---@field loadBox fun(self: sn.Model)
    ---@field getBoneUniformBuffer fun(self: sn.Model): sn.Buffer
    ---@field play fun(self: sn.Model, positon: number)
    ---@field update fun(self: sn.Model, delta: number)
    ---@field hasTexture fun(self:sn.Model, textureKey: number):sn.Texture
    ---@field getTexture fun(self:sn.Model, textureKey:number):sn.Texture
    ---@field setTexture fun(self:sn.Model, textureKey: number,texture : sn.Texture):sn.Texture
    Model = {},

    ---@class sn.AABB
    ---Constructor
    ---@field new fun(): sn.AABB
    ---@field min sn.Vec3
    ---@field max sn.Vec3
    ---@field updateWorld fun(self: sn.AABB, position: sn.Vec3, scale: sn.Vec3, modelsn.AABB: sn.AABB)
    ---@field intersectsAABB fun(self: sn.AABB, aabb: sn.AABB): boolean
    AABB = {},

    ---@class sn.Timer
    ---@field new fun(): sn.Timer
    ---@field start fun(self: sn.Timer)
    ---@field stop fun(self: sn.Timer)
    ---@field isStarted fun(self: sn.Timer): boolean
    ---@field setTime fun(self: sn.Timer, time: number)
    ---@field check fun(self: sn.Timer): boolean
    Timer = {},

    ---@class sn.Shader
    ---@field new fun(): sn.Shader
    ---@field load fun(self: sn.Shader, path: string, shaderStage: sn.ShaderStage, uniform_count: number)
    ---@field compileAndLoad fun(self: sn.Shader, source: string, shaderStage: sn.ShaderStage)
    Shader = {},

    ---@class sn.Font
    ---@field new fun(): sn.Font
    ---@field load fun(self: sn.Font, size: number)
    ---@field load fun(self: sn.Font, size: number, path: string)
    ---@field load fun(self: sn.Font, size: number, buffer: sn.Buffer)
    ---@field resize fun(self: sn.Font, size: number)
    ---@field region fun(self: sn.Font, text: string, fontSize: number, pivot: number, position: sn.Vec2): sn.Rect
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

    ---@class sn.Grid
    ---@field new fun(w: number, h: number): sn.Grid
    --- index starts 0
    ---@field at fun(self: sn.Grid, x: number, y: number): number
    --- index starts 0
    ---@field set fun(self: sn.Grid, x: number, y: number, v: number)
    ---@field width fun(self: sn.Grid): number
    ---@field height fun(self: sn.Grid): number
    ---@field size fun(self: sn.Grid): number
    ---@field clear fun(self: sn.Grid)
    ---@field resize fun(self: sn.Grid, w: number, h: number)
    ---@field fill fun(self: sn.Grid, value: number)
    --- index starts 0
    ---@field fillRect fun(self: sn.Grid, index: number, value: number)
    --- index starts 0
    ---@field setRow fun(self: sn.Grid, index: number, value: number)
    --- index starts 0
    ---@field setColumn fun(self: sn.Grid, index: number, value: number)
    Grid = {},

    ---@class sn.BFSGrid
    ---@field new fun(grid: sn.Grid): sn.BFSGrid
    ---@field width fun(self: sn.BFSGrid): number
    ---@field height fun(self: sn.BFSGrid): number
    --- index starts 0
    ---@field findPath fun(self: sn.BFSGrid, start: sn.Vec2, end_: sn.Vec2): any
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

    ---@class sn.Pivot
    ---@field TopLeft sn.Pivot
    ---@field TopCenter sn.Pivot
    ---@field TopRight sn.Pivot
    ---@field Center sn.Pivot
    ---@field BottomLeft sn.Pivot
    ---@field BottomCenter sn.Pivot
    ---@field BottomRight sn.Pivot
    Pivot = {},

    ---@class sn.Rect
    ---@field x number
    ---@field y number
    ---@field width number
    ---@field height number
    ---@field new fun(x: number, y: number, width: number, height: number):sn.Rect
    ---@field new fun(position: sn.Vec2, size: sn.Vec2):sn.Rect
    ---@field new fun(pivot:sn.Pivot,x:number,y:number,w:number,h:number)
    ---@field new fun(pivot:sn.Pivot,pos:sn.Vec2,size:sn.Vec2)
    ---@field topLeft fun(self:sn.Rect) :sn.Vec2
    ---@field topCenter fun(self:sn.Rect) :sn.Vec2
    ---@field topRight fun(self:sn.Rect) :sn.Vec2
    ---@field bottomLeft fun(self:sn.Rect) :sn.Vec2
    ---@field bottomCenter fun(self:sn.Rect) :sn.Vec2
    ---@field bottomRight fun(self:sn.Rect) :sn.Vec2
    ---@field position fun(self:sn.Rect):sn.Vec2
    ---@field positionFromPivot fun(self: sn.Rect, pivot: sn.Pivot):sn.Vec2
    ---@field size fun(self:sn.Rect):sn.Vec2
    ---@field intersectsRect fun(self: sn.Rect, rect: sn.Rect) : boolean
    Rect = {},

    ---@class sn.Transform
    ---@field position sn.Vec3
    ---@field rotation sn.Vec3
    ---@field scale sn.Vec3
    ---@field new fun(): sn.Transform
    Transform = {},

    ---@class sn.Collider
    ---@field isValid fun(self: sn.Collider): boolean
    ---@field isAdded fun(self: sn.Collider): boolean
    ---@field getPosition fun(self: sn.Collider): sn.Vec3
    ---@field getRotation fun(self: sn.Collider): sn.Vec3
    ---@field getVelocity fun(self: sn.Collider): sn.Vec3
    ---@field getAngularVelocity fun(self: sn.Collider): sn.Vec3
    ---@field setPosition fun(self: sn.Collider, position: sn.Vec3, activate: boolean?)
    ---@field setRotation fun(self: sn.Collider, rotation: sn.Vec3, activate: boolean?)
    ---@field setPositionAndRotation fun(self: sn.Collider, position: sn.Vec3, rotation: sn.Vec3, activate: boolean?)
    ---@field setLinearVelocity fun(self: sn.Collider, velocity: sn.Vec3)
    ---@field setAngularVelocity fun(self: sn.Collider, velocity: sn.Vec3)
    ---@field addForce fun(self: sn.Collider, force: sn.Vec3, activate: boolean?)
    ---@field addImpulse fun(self: sn.Collider, impulse: sn.Vec3)
    ---@field setFriction fun(self: sn.Collider, friction: number)
    ---@field setRestitution fun(self: sn.Collider, restitution: number)
    ---@field activate fun(self: sn.Collider)
    ---@field deactivate fun(self: sn.Collider)
    ---@field remove fun(self: sn.Collider)
    ---@field destroy fun(self: sn.Collider)
    Collider = {},

    ---@class sn.World
    ---@field new fun(): sn.World
    ---@field createBoxCollider fun(self: sn.World, transform: sn.Transform, isStatic: boolean): sn.Collider
    ---@field createSphereCollider fun(self: sn.World, position: sn.Vec3, radius: number, isStatic: boolean): sn.Collider
    ---@field createCylinderCollider fun(self: sn.World, position: sn.Vec3, rotation: sn.Vec3, halfHeight: number, radius: number, isStatic: boolean): sn.Collider
    ---@field createCapsuleCollider fun(self: sn.World, position: sn.Vec3, rotation: sn.Vec3, halfHeight: number, radius: number, isStatic: boolean): sn.Collider
    ---@field addCollider fun(self:sn.World, collider: sn.Collider, isActive: boolean)
    ---@field removeCollider fun(self:sn.World, collider: sn.Collider)
    ---@field destroyCollider fun(self:sn.World, collider: sn.Collider)
    World = {},

    ---Static class
    ---@class sn.Arguments
    ---@field getArgc fun(): number
    ---@field getArgv fun(): string[]
    Arguments = {},

    ---Static class
    ---@class sn.Event
    ---@field quit fun()
    Event = {},

    ---Static class
    ---@class sn.Script
    ---@field load fun(path: string, basePath?: string)
    Script = {},

    ---Static class
    ---@class sn.Filesystem
    ---@field enumerateDirectory fun(path: string): table
    Filesystem = {},

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
    ---@field topLeft fun() :sn.Vec2
    ---@field topCenter fun() :sn.Vec2
    ---@field topRight fun() :sn.Vec2
    ---@field bottomLeft fun() :sn.Vec2
    ---@field bottomCenter fun() :sn.Vec2
    ---@field bottomRight fun() :sn.Vec2
    Window = {},

    ---Static class
    ---@enum sn.ShaderStage
    ShaderStage = {
        Vertex = 0,
        Fragment = 1,
        Compute = 2,
    },

    ---Static class
    ---@class sn.BuiltinShader
    ---@field getDefaultVS fun():sn.Shader
    ---@field getDefaultFS fun():sn.Shader
    ---@field getDefaultInstancedVS fun():sn.Shader
    ---@field getCubemapVS fun():sn.Shader
    ---@field getCubemapFS fun():sn.Shader
    BuiltinShader = {},

    ---Static class
    ---@class sn.BuiltinPipeline
    ---@field getDefault3D fun(): sn.GraphicsPipeline
    ---@field getInstanced3D fun(): sn.GraphicsPipeline
    ---@field getDefault2D fun(): sn.GraphicsPipeline
    ---@field getCubemap fun(): sn.GraphicsPipeline
    BuiltinPipelines = {},

    ---Static class
    ---@class sn.Graphics
    ---@field drawRect fun(rect:sn.Rect, color: sn.Color, angle: number?)
    ---@field drawImage fun(texture: sn.Texture, rect:sn.Rect, angle: number?)
    ---@field drawText fun(text: string, font: sn.Font, position: sn.Vec2, color: sn.Color?, fontSize: number?,angle: number?)
    ---@field drawModel fun(model: sn.Model, transform: sn.Transform)
    ---@field drawModelInstanced fun(model: sn.Model, transforms: table)
    ---@field drawCubemap fun(cubemap: sn.Texture)
    ---@field setCamera3D fun(camera: sn.Camera3D)
    ---@field getCamera3D fun(): sn.Camera3D
    ---@field setCamera2D fun(camera: sn.Camera2D)
    ---@field getCamera2D fun(): sn.Camera2D
    ---@field getClearColor fun(): sn.Color
    ---@field setClearColor fun(c: sn.Color)
    ---@field setGraphicsPipeline fun(pipe: sn.GraphicsPipeline)
    ---@field resetGraphicsPipeline fun()
    ---@field setTexture fun(slotIndex : number, texture:sn.Texture)
    ---@field resetTexture fun(slotIndex : number)
    ---@field resetAllTexture fun()
    ---@field setUniformBuffer fun(binding: number, data: sn.Buffer)
    ---@field beginRenderTarget fun(rt: sn.RenderTexture)
    ---@field endRenderTarget fun()
    ---@field readbackTexture fun(rt: sn.RenderTexture, out: sn.Texture): sn.Texture
    Graphics = {},

    ---Static class
    ---@class sn.Physics
    ---@field setGravity fun(gravity: sn.Vec3)
    ---@field getGravity fun(): sn.Vec3
    ---@field bodyCount fun(): number
    ---@field optimizeBroadPhase fun()
    Physics = {},

    ---Static class
    ---@class sn.Keyboard
    ---@field A number
    ---@field B number
    ---@field C number
    ---@field D number
    ---@field E number
    ---@field F number
    ---@field G number
    ---@field H number
    ---@field I number
    ---@field J number
    ---@field K number
    ---@field L number
    ---@field M number
    ---@field N number
    ---@field O number
    ---@field P number
    ---@field Q number
    ---@field R number
    ---@field S number
    ---@field T number
    ---@field U number
    ---@field V number
    ---@field W number
    ---@field X number
    ---@field Y number
    ---@field Z number
    ---@field Key0 number
    ---@field Key1 number
    ---@field Key2 number
    ---@field Key3 number
    ---@field Key4 number
    ---@field Key5 number
    ---@field Key6 number
    ---@field Key7 number
    ---@field Key8 number
    ---@field Key9 number
    ---@field F1 number
    ---@field F2 number
    ---@field F3 number
    ---@field F4 number
    ---@field F5 number
    ---@field F6 number
    ---@field F7 number
    ---@field F8 number
    ---@field F9 number
    ---@field F10 number
    ---@field F11 number
    ---@field F12 number
    ---@field UP number
    ---@field DOWN number
    ---@field LEFT number
    ---@field RIGHT number
    ---@field ESCAPE number
    ---@field SPACE number
    ---@field BACKSPACE number
    ---@field TAB number
    ---@field RETURN number
    ---@field LSHIFT number
    ---@field RSHIFT number
    ---@field LCTRL number
    ---@field RCTRL number
    ---@field LALT number
    ---@field isPressed fun(scancode: number): boolean
    ---@field isReleased fun(scancode: number): boolean
    ---@field isDown fun(scancode: number): boolean
    Keyboard = {},

    ---Static class
    ---@class sn.Mouse
    ---@field LEFT number
    ---@field RIGHT number
    ---@field MIDDLE number
    ---@field X1 number
    ---@field X2 number
    ---@field isPressed fun(btn: number): boolean
    ---@field isReleased fun(btn: number): boolean
    ---@field isDown fun(btn: number): boolean
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
    ---@field INVALID number
    ---@field A number
    ---@field B number
    ---@field X number
    ---@field Y number
    ---@field BACK number
    ---@field GUIDE number
    ---@field START number
    ---@field LEFTSTICK number
    ---@field RIGHTSTICK number
    ---@field LEFTSHOULDER number
    ---@field RIGHTSHOULDER number
    ---@field DPAD_UP number
    ---@field DPAD_DOWN number
    ---@field DPAD_LEFT number
    ---@field DPAD_RIGHT number
    ---@field MISC1 number
    ---@field PADDLE1 number
    ---@field PADDLE2 number
    ---@field PADDLE3 number
    ---@field PADDLE4 number
    ---@field TOUCHPAD number
    ---@field isPressed fun(btn: number): boolean
    ---@field isReleased fun(btn: number): boolean
    ---@field isDown fun(btn: number): boolean
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
    ---@field milli fun(): number
    ---@field delta fun(): number
    Time = {},

    ---Static class
    ---@class sn.Log
    ---@field verbose fun(msg: any)
    ---@field info fun(msg: any)
    ---@field error fun(msg: any)
    ---@field warn fun(msg: any)
    ---@field critical fun(msg: any)
    Log = {},

    ---Static class
    ---@class sn.ImGui
    ---@field Begin fun(name:string , flags:sn.ImGui.WindowFlags?)
    ---@field End fun()
    ---@field button fun(name:string)
    ---@field text fun(name:string)
    ---@field setNextWindowPos fun(pos:sn.Vec2)
    ---@field setNextWindowSize fun(size:sn.Vec2)
    ImGui = {
        ---@class sn.ImGui.WindowFlags
        ---@field None number
        ---@field NoTitleBar number
        ---@field NoResize number
        ---@field NoMove number
        ---@field NoScrollBar number
        ---@field NoScrollWithMouse number
        ---@field NoCollapse number
        ---@field AlwaysAutoResize number
        ---@field NoBackground number
        ---@field NoSavedSettings number
        ---@field NoMouseInputs number
        ---@field MenuBar number
        ---@field HorizontalScrollbar number
        ---@field NoFocusOnAppearing number
        ---@field NoBringToFrontOnFocus number
        ---@field AlwaysVerticalScrollbar number
        ---@field AlwaysHorizontalScrollbar number
        ---@field NoNavInputs number
        ---@field NoNavFocus number
        ---@field UnsavedDocument number
        ---@field NoNav number
        ---@field NoDecoration number
        ---@field NoInputs number
    }
}
