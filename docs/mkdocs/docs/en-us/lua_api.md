# Lua API

There may be discrepancies with the main branch.  
For type information, refer to the sinen.lua file.

## About Invocation

Place Lua scripts in the `/script/` directory where `sinen.exe` exists.

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

Lua does not have classes, but tables are used instead.  
Sinen provides several built-in classes.

### Naming Conventions

Sinen uses the following naming conventions:  
- Class names: CamelCase (e.g., `Vec2`, `Vec3`, `Color`)
- Method names: CamelCase (e.g., `Length`, `Normalize`, `Copy`)
- Property names: lowerCase (e.g., `x`, `y`, `z`, `r`, `g`, `b`, `a`)

Instances are created using the class name (e.g., `a = Vec2(1, 2)`).  
Instance methods can be called with a colon (e.g., `a:Length()`).

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

- `value`: A number or a Vec2 object  
Creates a Vec2 object where both x and y components are set to the same value.

#### Vec2.x = number

The x component of the Vec2.

#### Vec2.y = number

The y component of the Vec2.

#### Vec2:Length() -> number

Returns the length of the vector.

#### Vec2:Normalize() -> Vec2

Returns a normalized version of the vector.

#### Vec2:Copy() -> Vec2

Returns a copy of the vector.

#### Vec2:Dot(Vec2, Vec2) -> number

- `Vec2`: Vector 1
- `Vec2`: Vector 2  
Returns the dot product.

#### Vec2:Lerp(Vec2, Vec2, number) -> Vec2

- `Vec2`: Vector 1
- `Vec2`: Vector 2
- `number`: Interpolation factor  
Returns the linear interpolation.

#### Vec2:Reflect(Vec2, Vec2) -> Vec2

- `Vec2`: Vector
- `Vec2`: Normal vector  
Returns the reflection vector.

### Vec3

#### Metatable

- `__add`: Vector addition (a + b)
- `__sub`: Vector subtraction (a - b)
- `__mul`: Vector multiplication (a * b)
- `__div`: Vector division (a / b)

#### Vec3(x, y, z) -> Vec3

- `x`: x component
- `y`: y component
- `z`: z component  
Creates a Vec3 object.

#### Vec3(value) -> Vec3

- `value`: A number or a Vec3 object  
Creates a Vec3 object where all x, y, z components are set to the same value.

#### Vec3.x = number

The x component of the Vec3.

#### Vec3.y = number

The y component of the Vec3.

#### Vec3.z = number

The z component of the Vec3.

#### Vec3:Length() -> number

Returns the length of the vector.

#### Vec3:Normalize() -> Vec3

Returns a normalized version of the vector.

#### Vec3:Copy() -> Vec3

Returns a copy of the vector.  
Because Lua passes by reference, use this when you need an actual copy.

#### Vec3:Forward(Vec3) -> Vec3

- `Vec3`: Axis vector  
Returns the forward vector.

#### Vec3:Dot(Vec3, Vec3) -> number

- `Vec3`: Vector 1
- `Vec3`: Vector 2  
Returns the dot product.

#### Vec3:Cross(Vec3, Vec3) -> Vec3

- `Vec3`: Vector 1
- `Vec3`: Vector 2  
Returns the cross product.

#### Vec3:Lerp(Vec3, Vec3, number) -> Vec3

- `Vec3`: Vector 1
- `Vec3`: Vector 2
- `number`: Interpolation factor  
Returns the linear interpolation.

#### Vec3:Reflect(Vec3, Vec3) -> Vec3

- `Vec3`: Vector
- `Vec3`: Normal vector  
Returns the reflection vector.

### Color

#### Color() -> Color

Creates a Color object.

#### Color(r, g, b, a) -> Color

- `r`: Red component (0.0~1.0)
- `g`: Green component (0.0~1.0)
- `b`: Blue component (0.0~1.0)
- `a`: Alpha component (0.0~1.0)  
Creates a color object.

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

Creates a texture object.

#### Texture:Load(string)

- `string`: The name of a texture file in `data/texture/`  
Loads a texture file.

#### Texture:FillColor(Color)

- `Color`: The color to fill the texture  
Fills the texture with a color.

#### Texture:BlendColor(Color)

- `Color`: The color to blend with the texture  
Blends the texture with a color.

#### Texture:Copy() -> Texture

Returns a copy of the texture.  
Because Lua passes by reference, use this when you need an actual copy.

#### Texture:Size() -> Vec2

Returns the size of the texture.

### RenderTexture

#### RenderTexture()

Creates a render texture object.

#### RenderTexture:Create(number, number)

- `number`: Width
- `number`: Height  
Creates a render texture.

### Material

#### Material() -> Material

Creates a material object.

#### Material:AppendTexture(Texture)

- `Texture`: Texture  
Appends a texture to the material.

### Font

#### Font() -> Font

Creates a font object.

#### Font:Load(string, number)

- `string`: Name of a font file in `data/font/`
- `number`: Font size (in pixels)  
Loads a .ttf file.

#### Font:RenderText(Texture, string, Color)

- `Texture`: Target texture
- `string`: Text to render
- `Color`: Text color  
Renders text on the CPU.  
The texture is resized according to the rendering result.

### Draw2D

#### Draw2D() -> Draw2D

Creates a Draw2D object.

#### Draw2D.position = Vec2

Position information.  
The center of the Sinen window is (0, 0).

#### Draw2D.scale = Vec2

Scale.

#### Draw2D.rotation = number

Clockwise rotation angle.

#### Draw2D.material = Material

Material.

#### Draw2D.model = Model

Model.

#### Draw2D:Draw()

Draws the 2D object.

#### Draw2D:Add(Vec2, number, Vec2)

- `Vec2`: Position
- `number`: Clockwise rotation angle
- `Vec2`: Scale  
Adds and draws multiple objects with the same texture/model.  
A single object already exists at instantiation.

#### Draw2D:Clear()

Clears the added objects.  
However, the object created at instantiation is not cleared.

### Draw3D

#### Draw3D() -> Draw3D

Creates a Draw3D object.

#### Draw3D.position = Vec3

Position information (world coordinates).

#### Draw3D.scale = Vec3

Scale.

#### Draw3D.rotation = Vec3

Rotation angle (Euler angles).

#### Draw3D.material = Material

Material.

#### Draw3D.model = Model

Model.

#### Draw3D:Draw()

Draws the 3D object.

#### Draw3D:Add(Vec3, Vec3, Vec3)

- `Vec3`: Position
- `Vec3`: Rotation
- `Vec3`: Scale  
Adds and draws multiple objects with the same texture/model.  
A single object already exists at instantiation.

#### Draw3D:Clear()

Clears the added objects.  
However, the object created at instantiation is not cleared.

### UniformData

#### UniformData() -> UniformData

Creates a UniformData object.

#### UniformData:Add(number)

- `number`: Data  
Adds uniform data.

#### UniformData:At(number, number)

- `number`: Data
- `number`: Index  
Adds uniform data at the specified index.

### Shader

#### Shader() -> Shader

Creates a shader object.

#### Shader:LoadVertexShader(string, number)

- `string`: Name of the vertex shader file in `data/shader/`
- `number`: Number of UniformData  
Loads a vertex shader.

#### Shader:LoadFragmentShader(string, number)

- `string`: Name of the fragment shader file in `data/shader/`
- `number`: Number of UniformData  
Loads a fragment shader.

#### Shader:CompileAndLoadVertexShader(string, number)

- `string`: Vertex shader source code
- `number`: Number of UniformData  
Compiles and loads a vertex shader.  
Requires `slangc` to be installed.

#### Shader:CompileAndLoadFragmentShader(string, number)

- `string`: Fragment shader source code
- `number`: Number of UniformData  
Compiles and loads a fragment shader.  
Requires `slangc` to be installed.

### RenderPipeline2D

#### RenderPipeline2D() -> RenderPipeline2D

Creates a 2D render pipeline object.

#### RenderPipeline2D:SetVertexShader(Shader)

- `Shader`: Vertex shader  
Sets the vertex shader.

#### RenderPipeline2D:SetFragmentShader(Shader)

- `Shader`: Fragment shader  
Sets the fragment shader.

#### RenderPipeline2D:Build()

Builds the render pipeline.

### RenderPipeline3D

#### RenderPipeline3D() -> RenderPipeline3D

Creates a 3D render pipeline object.

#### RenderPipeline3D:SetVertexShader(Shader)

- `Shader`: Vertex shader  
Sets the vertex shader.

#### RenderPipeline3D:SetFragmentShader(Shader)

- `Shader`: Fragment shader  
Sets the fragment shader.

#### RenderPipeline3D:Build()

Builds the render pipeline.

### Camera

#### Camera() -> Camera

Creates a camera object for 3D space.

#### Camera:LookAt(Vec3, Vec3, Vec3)

- `Vec3`: Position
- `Vec3`: Target
- `Vec3`: Up direction  
Sets the camera position, target, and up direction.

#### Camera:Perspective(number, number, number, number)

- `number`: Field of view
- `number`: Aspect ratio
- `number`: Near plane
- `number`: Far plane  
Sets perspective projection.

#### Camera:Orthographic(number, number, number, number, number, number)

- `number`: Left
- `number`: Right
- `number`: Bottom
- `number`: Top
- `number`: Near plane
- `number`: Far plane  
Sets orthographic projection.

#### Camera:IsAABBInFrustum(AABB) -> bool

- `AABB`: AABB  
Checks if the AABB is inside the camera frustum.

#### Camera:GetPosition = Vec3

Camera position.

#### Camera:GetTarget = Vec3

Camera target.

#### Camera.GetUp = Vec3

Camera up direction.

### Music

#### Music() -> Music

Creates a music object.

#### Music:Load(string)

- `string`: Name of a music file in `data/music/`  
Loads music from a file.  
Supports .ogg, .wav, .mp3 formats.

#### Music:Play()

Plays music.

#### Music:SetVolume(number)

- `number`: Volume  
Sets music volume.

### Sound

#### Sound() -> Sound

Creates a sound object.

#### Sound:Load(string)

- `string`: Name of a sound file in `data/sound/`  
Loads sound from a file.  
Supports .wav format.

#### Sound:Play()

Plays sound.

#### Sound:SetVolume(number)

- `number`: Volume  
Sets sound volume.

#### Sound:SetPitch(number)

- `number`: Pitch  
Sets sound pitch.

#### Sound:SetListener(Vec3)

- `Vec3`: Listener position  
Sets the position of the listener.

#### Sound:SetPosition(Vec3)

- `Vec3`: Sound source position  
Sets the position of the sound source.

### AABB

#### AABB.min = Vec3

Minimum point of the AABB.

#### AABB.max = Vec3

Maximum point of the AABB.

### Model

#### Model.AABB = AABB

AABB of the model.

#### Model:Load(string)

- `string`: Name of a model file in `data/model/`  
Supports .gltf and .glb formats.

## Global Functions

Functions that can be called directly without creating an instance.

### Random

#### Random:GetRange(number, number) -> number

- `number`: Minimum value
- `number`: Maximum value  
Returns a random floating-point number within the specified range.

### Window

#### Window:Name() -> string

Gets the window name.

#### Window:Rename(string)

- `string`: Window name  
Sets the window name.

#### Window:Size() -> Vec2

Gets the window size.

#### Window:Resize(Vec2)

- `Vec2`: Window size  
Sets the window size.

#### Window:Resized() -> bool

Checks if the window was resized this frame.

#### Window:Half() -> Vec2

Gets half the window size.

#### Window:SetFullscreen(bool)

- `bool`: Set fullscreen mode  
Sets window to fullscreen or windowed mode.

### Graphics

#### Graphics:SetClearColor() -> Color

Gets the clear color for the screen.

#### Graphics:SetClearColor(Color)

- `Color`: Clear color  
Sets the clear color for the screen.

#### Graphics:BeginPipeline2D(RenderPipeline2D)

- `RenderPipeline2D`: Render pipeline  
Begins 2D rendering.

#### Graphics:EndPipeline2D()

Ends 2D rendering.

#### Graphics:BeginPipeline3D(RenderPipeline3D)

- `RenderPipeline3D`: Render pipeline  
Begins 3D rendering.

#### Graphics:EndPipeline3D()

Ends 3D rendering.

#### Graphics:BeginRenderTexture2D(RenderTexture)

- `RenderTexture`: Render texture  
Begins rendering to a render texture.

#### Graphics:BeginRenderTexture3D(RenderTexture)

- `RenderTexture`: Render texture

#### Graphics:EndRenderTexture(RenderTexture, Texture)

- `RenderTexture`: Render texture
- `Texture`: Result texture  
Ends rendering to a render texture.  
`Texture` must be the same size as `RenderTexture`.

### Scene

#### Scene:Camera() -> Camera

Gets the main camera of the scene.

#### Scene:Size() -> Vec2

Gets the size of the scene.

#### Scene:Resize(Vec2)

- `Vec2`: Scene size  
Sets the size of the scene.

#### Scene:Half() -> Vec2

Gets half the scene size.

### Collision

#### Collision:AABBvsAABB(AABB, AABB) -> bool

- `AABB`: AABB
- `AABB`: AABB  
Performs collision detection between AABBs.

### Keyboard

#### Keyboard:IsPressed(code) -> bool

- `code`: Key code  
Checks if the key was pressed this frame.

#### Keyboard:IsReleased(code) -> bool

- `code`: Key code  
Checks if the key was released this frame.

#### Keyboard:IsDown(code) -> bool

- `code`: Key code  
Checks if the key is currently pressed.

#### Keyboard.[A-Z, 0-9, F1-F12, UP, DOWN, LEFT, RIGHT, ESCAPE, SPACE, ENTER, BACKSPACE, TAB, LSHIFT, RSHIFT, LCTRL, RCTRL, ALT]

Various key code constants.

### Mouse

#### Mouse.GetPosition() -> Vec2

Gets the mouse position within the window.  
The center of the window is the origin (0, 0).

#### Mouse.GetPositionOnScene() -> Vec2

Gets the mouse position within the scene.  
The center of the scene is the origin (0, 0).

#### Mouse.SetPosition(Vec2)

- `Vec2`: Position within the window  
Sets the mouse position within the window.  
The center of the window is the origin (0, 0).

#### Mouse.SetPositionOnScene(Vec2)

- `Vec2`: Position on the scene  
Sets the mouse position within the scene.  
The center of the scene is the origin (0, 0).

#### Mouse.IsPressed(code) -> bool

- `code`: Mouse button code  
Checks if the button was pressed this frame.

#### Mouse.IsReleased(code) -> bool

- `code`: Mouse button code  
Checks if the button was released this frame.

#### Mouse.IsDown(code) -> bool

- `code`: Mouse button code  
Checks if the button is currently pressed.

#### Mouse.ScrollWheel() -> Vec2

Gets the scroll wheel delta.

#### Mouse.HideCursor(bool)

- `bool`: Hide the mouse cursor  
Shows or hides the cursor.

#### Mouse.SetRelative(bool)

- `bool`: Get mouse position as relative  
Sets whether to obtain mouse position in relative mode.

#### Mouse.IsRelative() -> bool

Checks if the mouse position is being obtained in relative mode.

#### Mouse.[LEFT, RIGHT, MIDDLE, X1, X2]

Various mouse button code constants.

### Gamepad

#### Gamepad.IsConnected() -> bool

Checks if a gamepad is connected.

#### Gamepad.IsPressed(code) -> bool

- `code`: Gamepad button code  
Checks if the button was pressed this frame.

#### Gamepad.IsReleased(code) -> bool

- `code`: Gamepad button code  
Checks if the button was released this frame.

#### Gamepad.IsDown(code) -> bool

- `code`: Gamepad button code  
Checks if the button is currently pressed.

#### Gamepad.GetLeftStick() -> Vec2

Gets the delta of the left stick.

#### Gamepad.GetRightStick() -> Vec2

Gets the delta of the right stick.

#### Gamepad.[INVALID, A, B, X, Y, BACK, START, GUIDE, LEFTSTICK, RIGHTSTICK, LEFTSHOULDER, RIGHTSHOULDER, DPAD_UP, DPAD_DOWN, DPAD_LEFT, DPAD_RIGHT, MISC1, PADDLE1, PADDLE2, PADDLE3, PADDLE4, TOUCHPAD]

Various gamepad button code constants.

### Periodic

#### Periodic.Sin0_1(time: number) -> number

- `time`: Time in seconds  
Returns a value oscillating between 0.0 and 1.0 periodically.

#### Periodic.Cos0_1(time: number) -> number

- `time`: Time in seconds  
Returns a value oscillating between 0.0 and 1.0 periodically.

### Time

#### Time.Seconds() -> number

Gets the elapsed time of the current frame in seconds.

#### Time.Milli() -> number

Gets the elapsed time of the current frame in milliseconds.

### Logger

#### Logger.Info(string)

- `string`: Log message  
Outputs an info level log.

#### Logger.Warn(string)

- `string`: Log message  
Outputs a warning level log.

#### Logger.Error(string)

- `string`: Log message  
Outputs an error level log.

#### Logger.Critical(string)

- `string`: Log message  
Outputs a critical error level log.

#### Logger.Debug(string)

- `string`: Log message  
Outputs a debug level log.
