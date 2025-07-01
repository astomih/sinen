# Lua API

For type information, refer to the sinen.lua file.

## About Invocation

Place Lua scripts in `sinen.exe directory/script/`

```lua

-- Setup

function Update()
 -- Function called every frame
end

function Draw()
  -- Function called every frame for rendering
end
```

## Classes

Lua does not have classes, but tables are used instead  
Sinen provides several built-in classes

### Naming Conventions

Sinen uses the following naming conventions:

- Class names: CamelCase (e.g., `Vec2`, `Vec3`, `Color`)
- Method names: CamelCase (e.g., `Length`, `Normalize`, `Copy`)
- Property names: lowerCase (e.g., `x`, `y`, `z`, `r`, `g`, `b`, `a`)
  Instances are created using the class name() (e.g., `a = Vec2(1, 2)`)  
  Instance methods can be called with a colon(:) (e.g., `a:Length()`)

All classes start with sn. (e.g., `sn.Vec2`, `sn.Color`)

### Vec2

#### Metatable

- `__add`: Vector addition (a + b)
- `__sub`: Vector subtraction (a - b)
- `__mul`: Vector multiplication (a * b)
- `__div`: Vector division (a / b)

#### Vec2(x, y) -> Vec2

- `x`: The x component of the vector
- `y`: The y component of the vector  
  Creates a Vec2 object.

#### Vec2(value) -> Vec2

- `value`: A number or Vec2 object
  Creates a Vec2 object with x, y components set to the same value

#### Vec2.x = number

The x component of Vec2.

#### Vec2.y = number

The y component of Vec2.

#### Vec2:Length() -> number

Gets the length of the vector

#### Vec2:Normalize() -> Vec2

Normalizes the vector

#### Vec2:Copy() -> Vec2

Copies the vector

#### Vec2:Dot(other) -> number

- `other`: Vec2 object
  Gets the dot product

#### Vec2:Lerp(other, t) -> Vec2

- `other`: Vec2 object
- `t`: Interpolation factor
  Gets linear interpolation

#### Vec2:Reflect(n) -> Vec2

- `n`: Normal vector
  Gets reflection vector

### Vec2i

#### Metatable

- `__add`: Vector addition (a + b)
- `__sub`: Vector subtraction (a - b)

#### Vec2i(x, y) -> Vec2i

- `x`: The x component of the vector (integer)
- `y`: The y component of the vector (integer)  
  Creates a Vec2i object.

#### Vec2i(value) -> Vec2i

- `value`: Integer value
  Creates a Vec2i object with x, y components set to the same value

#### Vec2i.x = integer

The x component of Vec2i.

#### Vec2i.y = integer

The y component of Vec2i.

### Vec3

#### Metatable

- `__add`: Vector addition (a + b)
- `__sub`: Vector subtraction (a - b)
- `__mul`: Vector multiplication (a * b)
- `__div`: Vector division (a / b)

#### Vec3(x, y, z) -> Vec3

- `x`: The x component of the vector
- `y`: The y component of the vector
- `z`: The z component of the vector  
  Creates a Vec3 object

#### Vec3(value) -> Vec3

- `value`: A number or Vec3 object
  Creates a Vec3 object with x, y, z components set to the same value

#### Vec3.x = number

The x component of Vec3

#### Vec3.y = number

The y component of Vec3

#### Vec3.z = number

The z component of Vec3

#### Vec3:Length() -> number

Gets the length of the vector

#### Vec3:Normalize() -> Vec3

Normalizes the vector

#### Vec3:Copy() -> Vec3

Copies the vector  
Since Lua passes by reference, use this when you need a copy

#### Vec3:Forward(rotation) -> Vec3

- `rotation`: Rotation angle vector  
  Gets the forward vector

#### Vec3:Dot(other) -> number

- `other`: Vec3 object
  Gets the dot product

#### Vec3:Cross(other) -> Vec3

- `other`: Vec3 object
  Gets the cross product

#### Vec3:Lerp(other, t) -> Vec3

- `other`: Vec3 object
- `t`: Interpolation factor
  Gets linear interpolation

#### Vec3:Reflect(n) -> Vec3

- `n`: Normal vector
  Gets reflection vector

### Vec3i

#### Metatable

- `__add`: Vector addition (a + b)
- `__sub`: Vector subtraction (a - b)

#### Vec3i(x, y, z) -> Vec3i

- `x`: The x component of the vector (integer)
- `y`: The y component of the vector (integer)
- `z`: The z component of the vector (integer)
  Creates a Vec3i object

#### Vec3i(value) -> Vec3i

- `value`: Integer value
  Creates a Vec3i object with x, y, z components set to the same value

#### Vec3i.x = integer

The x component of Vec3i

#### Vec3i.y = integer

The y component of Vec3i

#### Vec3i.z = integer

The z component of Vec3i

### Color

#### Color() -> Color

Creates a Color object

#### Color(r, g, b, a) -> color

- `r`: Red component (0.0~1.0)
- `g`: Green component (0.0~1.0)
- `b`: Blue component (0.0~1.0)
- `a`: Alpha component (0.0~1.0)  
  Creates a color object

#### Color.r = number

Red component (0.0~1.0)

#### Color.g = number

Green component (0.0~1.0)

#### Color.b = number

Blue component (0.0~1.0)

#### Color.a = number

Alpha component (0.0~1.0)

### Texture

#### Texture()

Creates a texture object

#### Texture:Load(string)

- `string`: Name of texture file in data/texture/  
  Loads a texture file

#### Texture:FillColor(Color)

- `Color`: Color to fill the texture  
  Fills the texture

#### Texture:BlendColor(Color)

- `Color`: Color to blend with the texture  
  Blends the texture

#### Texture:Copy() -> Texture

Copies the texture  
Since Lua passes by reference, use this when you need a copy

#### Texture:Size() -> Vec2

Gets the size of the texture

### RenderTexture

#### RenderTexture()

Creates a render texture object

#### RenderTexture:Create(number, number)

- `number`: Width
- `number`: Height  
  Creates a render texture

### Material

#### Material() -> Material

Creates a material object

#### Material:SetTexture(texture, index)

- `texture`: Texture
- `index`: Index (1-based, optional)
  Sets a texture to the material

#### Material:AppendTexture(texture)

- `texture`: Texture  
  Appends a texture to the material

#### Material:Clear()

Clears all textures in the material

#### Material:GetTexture(index) -> Texture

- `index`: Index
  Gets the texture at the specified index

### Font

#### Font() -> Font

Creates a font object

#### Font:Load(size, path)

- `size`: Font size in pixels
- `path`: Name of font file in data/font/ (optional)
  Loads a .ttf file

#### Font:RenderText(texture, text, color) -> Texture

- `texture`: Target texture
- `text`: String to render
- `color`: Text color
  Renders text on CPU  
  Texture is resized according to the rendering result

#### Font:Resize(size)

- `size`: Font size in pixels
  Resizes the font

### Timer

#### Timer() -> Timer

Creates a timer object

#### Timer:Start()

Starts the timer

#### Timer:Stop()

Stops the timer

#### Timer:IsStarted() -> boolean

Checks if the timer is started

#### Timer:SetTime(time)

- `time`: Time in seconds
  Sets the timer time

#### Timer:Check() -> boolean

Checks if the timer has reached the set time

### Grid

#### Grid(w, h) -> Grid

- `w`: Width
- `h`: Height
  Creates a grid object

#### Grid:At(x, y) -> integer

- `x`: X coordinate
- `y`: Y coordinate
  Gets the value at the specified coordinates

#### Grid:Set(x, y, v)

- `x`: X coordinate
- `y`: Y coordinate
- `v`: Value
  Sets a value at the specified coordinates

#### Grid:Width() -> integer

Gets the width of the grid

#### Grid:Height() -> integer

Gets the height of the grid

#### Grid:Size() -> integer

Gets the size (width × height) of the grid

#### Grid:Clear()

Clears the grid

#### Grid:Resize(w, h)

- `w`: Width
- `h`: Height
  Resizes the grid

#### Grid:Fill(value)

- `value`: Value
  Fills the grid with the specified value

### BFSGrid

#### BFSGrid(grid) -> BFSGrid

- `grid`: Grid object
  Creates a BFS (Breadth-First Search) grid object

#### BFSGrid:Width() -> integer

Gets the width of the grid

#### BFSGrid:Height() -> integer

Gets the height of the grid

#### BFSGrid:FindPath(start, end_) -> any

- `start`: Start position (Vec2)
- `end_`: End position (Vec2)
  Searches for a path

#### BFSGrid:Trace() -> Vec2

Gets the next position

#### BFSGrid:Traceable() -> boolean

Checks if tracing is possible

#### BFSGrid:Reset()

Resets the search state

### Draw2D

#### Draw2D() -> Draw2D

Creates a Draw2D object

#### Draw2D.position = Vec2

Position information  
The center of the Sinen window is (0, 0)

#### Draw2D.scale = Vec2

Scale

#### Draw2D.rotation = number

Clockwise rotation angle

#### Draw2D.material = Material

Material

#### Draw2D.model = Model

Model

#### Draw2D:Draw()

Draws the 2D object

#### Draw2D:Add(drawable)

- `drawable`: Drawable object
  Adds and draws multiple objects with the same texture/model  
  One object already exists at instantiation

#### Draw2D:At(x, y)

- `x`: X coordinate
- `y`: Y coordinate
  Sets the position of the object

#### Draw2D:Clear()

Clears the added objects  
However, the object that exists at instantiation is not cleared

### Draw3D

#### Draw3D() -> Draw3D

Creates a Draw3D object

#### Draw3D.position = Vec3

Position information (world coordinates)

#### Draw3D.scale = Vec3

Scale

#### Draw3D.rotation = Vec3

Rotation angle (Euler angles)

#### Draw3D.material = Material

Material

#### Draw3D.model = Model

Model

#### Draw3D.isDrawDepth = boolean

Whether to perform depth drawing

#### Draw3D:Draw()

Draws the 3D object

#### Draw3D:Add(position, rotation, scale)

- `position`: Position information (Vec3)
- `rotation`: Rotation angle (Vec3)
- `scale`: Scale (Vec3)  
  Adds and draws multiple objects with the same texture/model  
  One object already exists at instantiation

#### Draw3D:At(x, y, z)

- `x`: X coordinate
- `y`: Y coordinate
- `z`: Z coordinate
  Sets the position of the object

#### Draw3D:Clear()

Clears the added objects  
However, the object that exists at instantiation is not cleared

### UniformData

#### UniformData() -> UniformData

Creates a uniform data object

#### UniformData:Add(value)

- `value`: Data  
  Adds uniform data

#### UniformData:Change(index, value)

- `index`: Index
- `value`: Data
  Changes uniform data at the specified index

### Shader

#### Shader() -> Shader

Creates a shader object

#### Shader:LoadVertexShader(string,number)

- `string`: Name of vertex shader file in data/shader/
- `number`: Number of UniformData  
  Loads a vertex shader

#### Shader:LoadFragmentShader(string,number)

- `string`: Name of fragment shader file in data/shader/
- `number`: Number of UniformData  
  Loads a fragment shader

#### Shader:CompileAndLoadVertexShader(string,number)

- `string`: Vertex shader source code
- `number`: Number of UniformData
  Compiles and loads a vertex shader  
  Requires `slangc` to be installed

#### Shader:CompileAndLoadFragmentShader(string,number)

- `string`: Fragment shader source code
- `number`: Number of UniformData
  Compiles and loads a fragment shader  
  Requires `slangc` to be installed

### GraphicsPipeline2D

#### GraphicsPipeline2D() -> GraphicsPipeline2D

Creates a 2D graphics pipeline object

#### GraphicsPipeline2D:SetVertexShader(shader)

- `shader`: Shader object  
  Sets the vertex shader

#### GraphicsPipeline2D:SetFragmentShader(shader)

- `shader`: Shader object  
  Sets the fragment shader

#### GraphicsPipeline2D:Build()

Builds the graphics pipeline

### GraphicsPipeline3D

#### GraphicsPipeline3D() -> GraphicsPipeline3D

Creates a 3D graphics pipeline object

#### GraphicsPipeline3D:SetVertexShader(shader)

- `shader`: Shader object
  Sets the vertex shader

#### GraphicsPipeline3D:SetVertexInstancedShader(shader)

- `shader`: Shader object
  Sets the instanced vertex shader

#### GraphicsPipeline3D:SetFragmentShader(shader)

- `shader`: Shader object
  Sets the fragment shader

#### GraphicsPipeline3D:SetAnimation(anim)

- `anim`: Animation
  Sets the animation

#### GraphicsPipeline3D:Build()

Builds the graphics pipeline

### Camera

#### Camera() -> Camera

Creates a camera object for 3D space

#### Camera:LookAt(Vec3, Vec3, Vec3)

- `Vec3`: Position
- `Vec3`: Target
- `Vec3`: Up direction  
  Sets the camera position, target, and up direction

#### Camera:Perspective(number, number, number, number)

- `number`: Field of view
- `number`: Aspect ratio
- `number`: Near plane
- `number`: Far plane  
  Sets the camera's perspective projection

#### Camera:Orthographic(number, number, number, number, number, number)

- `number`: Left
- `number`: Right
- `number`: Bottom
- `number`: Top
- `number`: Near plane
- `number`: Far plane  
  Sets the camera's orthographic projection

#### Camera:IsAABBInFrustum(aabb) -> boolean

- `aabb`: AABB object  
  Checks if the AABB is contained within the camera's frustum

#### Camera:GetPosition() -> Vec3

Gets the camera position

#### Camera:GetTarget() -> Vec3

Gets the camera target

#### Camera:GetUp() -> Vec3

Gets the camera up direction

### Music

#### Music() -> Music

#### Music:Load(string)

- `string`: Name of music file in data/music/  
  Loads music from file  
  Supports .ogg, .wav, .mp3 formats

#### Music:Play()

Plays music

#### Music:SetVolume(number)

- `number`: Volume  
  Sets the music volume

### Sound

#### Sound() -> Sound

#### Sound:Load(string)

- `string`: Name of sound file in data/sound/  
  Loads sound from file  
  Supports .wav format

#### Sound:Play()

Plays sound

#### Sound:SetVolume(number)

- `number`: Volume  
  Sets the sound volume

#### Sound:SetPitch(number)

- `number`: Pitch  
  Sets the sound pitch

#### Sound:SetListener(Vec3)

- `Vec3`: Listener position  
  Sets the listener position

#### Sound:SetPosition(Vec3)

- `Vec3`: Sound source position  
  Sets the sound source position

### AABB

#### AABB() -> AABB

Creates an AABB object

#### AABB.min = Vec3

Minimum point of the AABB

#### AABB.max = Vec3

Maximum point of the AABB

#### AABB:UpdateWorld(position, scale, modelAABB)

- `position`: Position (Vec3)
- `scale`: Scale (Vec3)
- `modelAABB`: Model's AABB
  Updates the AABB in world coordinates

### Model

#### Model() -> Model

Creates a model object

#### Model:GetAABB() -> AABB

Gets the model's AABB

#### Model:Load(path)

- `path`: Name of model file in data/model/  
  Supports .gltf, .glb formats

#### Model:LoadSprite()

Loads a sprite model

#### Model:LoadBox()

Loads a box-shaped model

#### Model:GetBoneUniformData() -> UniformData

Gets the bone uniform data

#### Model:Play(position)

- `position`: Playback position
  Plays animation

#### Model:Update(delta)

- `delta`: Delta time
  Updates the model

## Global Functions

Functions that can be called directly without creating an instance

### Random

#### Random.GetRange(a, b) -> number

- `a`: Minimum value
- `b`: Maximum value  
  Gets a random floating-point number within the specified range

#### Random.GetIntRange(a, b) -> number

- `a`: Minimum value (integer)
- `b`: Maximum value (integer)  
  Gets a random integer within the specified range

### Window

#### Window.GetName() -> string

Gets the window name

#### Window.Rename(name)

- `name`: Window name  
  Changes the window name

#### Window.Size() -> Vec2

Gets the window size

#### Window.Resize(size)

- `size`: Window size (Vec2)  
  Changes the window size

#### Window.Resized() -> boolean

Gets whether the window was resized in the current frame

#### Window.Half() -> Vec2

Gets half the window size

#### Window.SetFullscreen(full)

- `full`: Whether to make the window fullscreen  
  Sets the window to fullscreen or windowed mode

### Graphics

#### Graphics.GetClearColor() -> Color

Gets the screen clear color

#### Graphics.SetClearColor(c)

- `c`: Screen clear color (Color)  
  Sets the screen clear color

#### Graphics.BindPipeline2D(pipe)

- `pipe`: GraphicsPipeline2D  
  Binds a 2D graphics pipeline

#### Graphics.BindDefaultPipeline2D()

Binds the default 2D graphics pipeline

#### Graphics.BindPipeline3D(pipe)

- `pipe`: GraphicsPipeline3D  
  Binds a 3D graphics pipeline

#### Graphics.BindDefaultPipeline3D()

Binds the default 3D graphics pipeline

#### Graphics.SetUniformData(binding, data)

- `binding`: Binding number (integer)
- `data`: UniformData object
  Sets uniform data

#### Graphics.BeginTarget2D(rt)

- `rt`: RenderTexture object
  Begins 2D render target

#### Graphics.BeginTarget3D(rt)

- `rt`: RenderTexture object
  Begins 3D render target

#### Graphics.EndTarget(rt, texture_ref)

- `rt`: RenderTexture object
- `texture_ref`: Result Texture object  
  Ends render target

### Scene

#### Scene.GetCamera() -> Camera

Gets the scene's main camera

#### Scene.Size() -> Vec2

Gets the scene size

#### Scene.Resize(size)

- `size`: Scene size (Vec2)  
  Changes the scene size

#### Scene.Half() -> Vec2

Gets half the scene size

#### Scene.Ratio() -> number

Gets the scene aspect ratio

#### Scene.InvRatio() -> number

Gets the scene inverse aspect ratio

#### Scene.DeltaTime() -> number

Gets the delta time

#### Scene.Change(name)

- `name`: Scene name
  Changes the scene

### Collision

#### Collision.AABBvsAABB(a, b) -> boolean

- `a`: AABB object
- `b`: AABB object  
  Performs collision detection between AABBs

### Keyboard

#### Keyboard.IsPressed(scancode) -> boolean

- `scancode`: Key code  
  Checks if the key was pressed in the current frame

#### Keyboard.IsReleased(scancode) -> boolean

- `scancode`: Key code  
  Checks if the key was released in the current frame

#### Keyboard.IsDown(scancode) -> boolean

- `scancode`: Key code  
  Checks if the key is being pressed

#### Keyboard.A = code

#### Keyboard.B = code

#### Keyboard.C = code

#### Keyboard.D = code

#### Keyboard.E = code

#### Keyboard.F = code

#### Keyboard.G = code

#### Keyboard.H = code

#### Keyboard.I = code

#### Keyboard.J = code

#### Keyboard.K = code

#### Keyboard.L = code

#### Keyboard.M = code

#### Keyboard.N = code

#### Keyboard.O = code

#### Keyboard.P = code

#### Keyboard.Q = code

#### Keyboard.R = code

#### Keyboard.S = code

#### Keyboard.T = code

#### Keyboard.U = code

#### Keyboard.V = code

#### Keyboard.W = code

#### Keyboard.X = code

#### Keyboard.Y = code

#### Keyboard.Z = code

#### Keyboard.Key0 = code

#### Keyboard.Key1 = code

#### Keyboard.Key2 = code

#### Keyboard.Key3 = code

#### Keyboard.Key4 = code

#### Keyboard.Key5 = code

#### Keyboard.Key6 = code

#### Keyboard.Key7 = code

#### Keyboard.Key8 = code

#### Keyboard.Key9 = code

#### Keyboard.F1 = code

#### Keyboard.F2 = code

#### Keyboard.F3 = code

#### Keyboard.F4 = code

#### Keyboard.F5 = code

#### Keyboard.F6 = code

#### Keyboard.F7 = code

#### Keyboard.F8 = code

#### Keyboard.F9 = code

#### Keyboard.F10 = code

#### Keyboard.F11 = code

#### Keyboard.F12 = code

#### Keyboard.UP = code

#### Keyboard.DOWN = code

#### Keyboard.LEFT = code

#### Keyboard.RIGHT = code

#### Keyboard.ESCAPE = code

#### Keyboard.SPACE = code

#### Keyboard.RETURN = code

#### Keyboard.BACKSPACE = code

#### Keyboard.TAB = code

#### Keyboard.LSHIFT = code

#### Keyboard.RSHIFT = code

#### Keyboard.LCTRL = code

#### Keyboard.RCTRL = code

#### Keyboard.LALT = code

### Mouse

#### Mouse.GetPosition() -> Vec2

Gets the mouse position within the window  
The center of the window is the origin (0, 0)

#### Mouse.GetPositionOnScene() -> Vec2

Gets the mouse position within the scene  
The center of the scene is the origin (0, 0)

#### Mouse.SetPosition(Vec2)

- `Vec2`: Mouse position on the window  
  Sets the mouse position on the window  
  The center of the window is the origin (0, 0)

#### Mouse.SetPositionOnScene(Vec2)

- `Vec2`: Mouse position on the scene  
  Sets the mouse position on the scene  
  The center of the scene is the origin (0, 0)

#### Mouse.IsPressed(btn) -> boolean

- `btn`: Mouse button code  
  Checks if the button was pressed in the current frame

#### Mouse.IsReleased(btn) -> boolean

- `btn`: Mouse button code  
  Checks if the button was released in the current frame

#### Mouse.IsDown(btn) -> boolean

- `btn`: Mouse button code  
  Checks if the button is being pressed

#### Mouse.GetScrollWheel() -> number

Gets the mouse scroll wheel delta

#### Mouse.HideCursor(isHide)

- `isHide`: Whether to hide the mouse cursor  
  Shows or hides the cursor

#### Mouse.SetRelative(isRelative)

- `isRelative`: Whether to get mouse position as relative coordinates  
  Sets whether to get mouse position in relative coordinates

#### Mouse.LEFT = code

#### Mouse.RIGHT = code

#### Mouse.MIDDLE = code

#### Mouse.X1 = code

#### Mouse.X2 = code

### Gamepad

#### Gamepad.IsConnected() -> boolean

Checks if a gamepad is connected

#### Gamepad.IsPressed(btn) -> boolean

- `btn`: Gamepad button code  
  Checks if the button was pressed in the current frame

#### Gamepad.IsReleased(btn) -> boolean

- `btn`: Gamepad button code  
  Checks if the button was released in the current frame

#### Gamepad.IsDown(btn) -> boolean

- `btn`: Gamepad button code  
  Checks if the button is being pressed

#### Gamepad.GetLeftStick() -> Vec2

Gets the left stick delta

#### Gamepad.GetRightStick() -> Vec2

Gets the right stick delta

#### Gamepad.INVALID = code

#### Gamepad.A = code

#### Gamepad.B = code

#### Gamepad.X = code

#### Gamepad.Y = code

#### Gamepad.BACK = code

#### Gamepad.START = code

#### Gamepad.GUIDE = code

#### Gamepad.LEFTSTICK = code

#### Gamepad.RIGHTSTICK = code

#### Gamepad.LEFTSHOULDER = code

#### Gamepad.RIGHTSHOULDER = code

#### Gamepad.DPAD_UP = code

#### Gamepad.DPAD_DOWN = code

#### Gamepad.DPAD_LEFT = code

#### Gamepad.DPAD_RIGHT = code

#### Gamepad.MISC1 = code

#### Gamepad.PADDLE1 = code

#### Gamepad.PADDLE2 = code

#### Gamepad.PADDLE3 = code

#### Gamepad.PADDLE4 = code

#### Gamepad.TOUCHPAD = code

### Periodic

#### Periodic.Sin0_1(t1, t2) -> number

- `t1`: Time parameter 1
- `t2`: Time parameter 2  
  Gets a value that changes periodically between 0.0 and 1.0 with sine wave

#### Periodic.Cos0_1(t1, t2) -> number

- `t1`: Time parameter 1
- `t2`: Time parameter 2  
  Gets a value that changes periodically between 0.0 and 1.0 with cosine wave

### Time

#### Time.Seconds() -> number

Gets the elapsed time since application start in seconds

#### Time.Milli() -> integer

Gets the elapsed time since application start in milliseconds

### Logger

#### Logger.Verbose(msg)

- `msg`: Log message  
  Outputs a verbose level log

#### Logger.Info(msg)

- `msg`: Log message  
  Outputs an info level log

#### Logger.Warn(msg)

- `msg`: Log message  
  Outputs a warning level log

#### Logger.Error(msg)

- `msg`: Log message  
  Outputs an error level log

#### Logger.Critical(msg)

- `msg`: Log message  
  Outputs a critical error level log