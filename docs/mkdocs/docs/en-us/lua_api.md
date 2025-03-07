# Lua API
## About call
Lua scripts placed in the `executable/script` directory are executed in the order they are loaded.
```lua

-- setup

function Update()
end

function Draw()
end
```

## Classes
Lua does not have classes, but tables are used instead. 
Sinen provides several predefined classes.

### Naming Conventions
In Sinen, Lua classes start with an uppercase letter. 
Instances are created using the class name followed by parentheses (e.g., `a = Vector2(1.0, 2.0)`). 
Functions can be called using a colon (`:`) (e.g., `a:length()`).

### Vector2
#### Metatable
- `__add`: Performs vector addition (`a + b`).  
- `__sub`: Performs vector subtraction (`a - b`).  
- `__mul`: Performs vector multiplication (`a * b`).  
- `__div`: Performs vector division (`a / b`).  

#### Vector2(x, y) -> Vector2
- `x`: X component of the vector.
- `y`: Y component of the vector.
Creates a Vector2 object.

#### Vector2.x = float
X component of the Vector2.
#### Vector2.y = float
Y component of the Vector2.
#### Vector2:length() -> float
Returns the length of the vector.
#### Vector2:normalize() -> Vector2
Normalizes the vector.

### Vector3
#### Metatable
- `__add`: Performs vector addition (`a + b`).  
- `__sub`: Performs vector subtraction (`a - b`).  
- `__mul`: Performs vector multiplication (`a * b`).  
- `__div`: Performs vector division (`a / b`).  

#### Vector3(x, y, z) -> Vector3
- `x`: X component of the vector.
- `y`: Y component of the vector.
- z: Z component of the vector.
Creates a Vector3 object.

#### Vector3.x = float
X component of the Vector3.
#### Vector3.y = float
Y component of the Vector3.
#### Vector3.z = float
Z component of the Vector3.
#### Vector3:length() -> float
Returns the length of the vector.
#### Vector3:normalize() -> Vector3
Normalizes the vector.
#### Vector3:copy() -> Vector3
Copies the vector. Since Lua primarily passes references, use this method if a copy is needed.
#### Vector3:forward() -> Vector3
Returns the forward vector.

### Point2i
Represents a 2D integer point.
#### Point2i(x, y) -> Point2i
- `x`: X component of Point2i.
- `y`: Y component of Point2i.
#### Point2i.x = int
X component of Point2i.
#### Point2i.y = int
Y component of Point2i.

### Color
Represents a color.
#### Color(r, g, b, a) -> Color
- `r`: Red component (0.0~1.0).
- `g`: Green component (0.0~1.0).
- `b`: Blue component (0.0~1.0).
- `a`: Alpha component (0.0~1.0).
Creates a Color object.

#### Color.r = float
Red component (0.0~1.0).
#### Color.g = float
Green component (0.0~1.0).
#### Color.b = float
Blue component (0.0~1.0).
#### Color.a = float
Alpha component (0.0~1.0).

### Texture
#### Texture()
Creates a Texture object.
#### Texture:load(string)
- `string`: Path to the texture.
Loads a texture from a file.
#### Texture:fill_color(Color)
- `Color`: Color to fill the texture with.
Fills the texture with a color.
#### Texture:blend_color(Color)
- `Color`: Color to blend the texture with.
Blends the texture with a color.
#### Texture:copy() -> Texture
Copies the texture.
#### Texture:size() -> Vector2
Gets the size of the texture.

### Font
#### Font() -> Font
Creates a font object.
#### Font:load(string)
- `string`: Name of the font file in `data/font/`.
Loads a .ttf font file.
#### Font:render_text(Texture, string, color)
- `Texture`: Target texture.
- `string`: Text to render.
- `Color`: Text color.
Renders text on the CPU.

### Draw2D
#### Draw2D(Texture) -> Draw2D
- `Texture`: Texture to render.
Creates a 2D object for rendering.
#### Draw2D:draw()
Renders a 2D object.
#### Draw2D.position = Vector2
Position.  
Sinen uses the center of the screen as the origin.
#### Draw2D.scale = Vector2
Scale.
#### Draw2D.rotation = float
Clockwise rotation angle.
#### Draw2D.texture = Texture
Texture.
#### Draw2D.model = Model
Model.
#### Draw2D:add(Vector2, float, Vector2)
- `Vector2`: Position.
- `float`: Clockwise rotation angle.
- `Vector2`: Scale.  
Adds multiple objects with the same texture and model for rendering. One object already exists at the time of instance creation.
#### Draw2D:clear()
Clears added objects. However, the initially created object is not cleared.

### Draw3D
#### Draw3D:draw(Texture)
- `Texture`: Texture to render.
Renders a 3D object.
#### Draw3D.position = Vector3
World position.
#### Draw3D.scale = Vector3
Scale.
#### Draw3D.rotation = Vector3
Euler angle rotation.
#### Draw3D.texture = Texture
Texture.
#### Draw3D.model = Model
Model.
#### Draw3D:add(Vector3, Vector3, Vector3)
- `Vector3`: Position.
- `Vector3`: Rotation.
- `Vector3`: Scale.  
Adds multiple objects with the same texture and model for rendering. One object already exists at the time of instance creation.
#### Draw3D:clear()
Clears added objects. However, the initially created object is not cleared.
### UniformData
#### UniformData() -> UniformData
Creates a uniform data object.
#### UniformData:add(float)
- float: Data  
Adds uniform data.
#### UniformData:at(float, int)
- `float`: Data
- `int`: Index  
Adds uniform data at the specified index.
### Shader
#### Shader() -> Shader
Creates a shader object.
#### Shader:load_vertex_shader(string, int)
- `string`: Name of the vertex shader file located in data/shader/.
- `int`: Number of UniformData.
Loads a vertex shader.
#### Shader:load_fragment_shader(string, int)
- `string`: Name of the fragment shader file located in data/shader/.
- `int`: Number of UniformData.
Loads a fragment shader.
### RenderPipeline2D
#### RenderPipeline2D() -> RenderPipeline2D
Creates a 2D render pipeline object.
#### RenderPipeline2D:set_vertex_shader(Shader)
- `Shader`: Vertex shader
Sets the vertex shader.
#### RenderPipeline2D:set_fragment_shader(Shader)
- `Shader`: Fragment shader
Sets the fragment shader.
#### RenderPipeline2D:build()
Builds the render pipeline.
### RenderPipeline3D
#### RenderPipeline3D() -> RenderPipeline3D
Creates a 3D render pipeline object.
#### RenderPipeline3D:set_vertex_shader(Shader)
- `Shader`: Vertex shader
Sets the vertex shader.
#### RenderPipeline3D:set_fragment_shader(Shader)
- `Shader`: Fragment shader  
Sets the fragment shader.
#### RenderPipeline3D:build()
Builds the render pipeline.
### Camera
#### Camera:look_at(Vector3, Vector3, Vector3)
- `Vector3`: Position.
- `Vector3`: Target.
- `Vector3`: Up direction.
Sets the camera's position, target, and up direction.
#### Camera:perspective(float, float, float, float)
- `float`: Field of view.
- `float`: Aspect ratio.
- `float`: Near plane.
- `float`: Far plane.
Sets the camera's perspective projection.
#### Camera:orthographic(float, float, float, float, float, float)
- `float`: Left.
- `float`: Right.
- `float`: Bottom.
- `float`: Top.
- `float`: Near plane.
- `float`: Far plane.
Sets the camera's orthographic projection.
#### Camera.position = Vector3
Camera position.
#### Camera.target = Vector3
Camera target.
#### Camera.up = Vector3
Camera up direction.
### Music
#### Music() -> Music
Creates a Music object.
#### Music:load(string)
- `string`: Name of the music file located in `data/music/`.
Loads a music file. Supports .ogg, .wav, and .mp3 formats.
#### Music:play()
Plays the music.
#### Music:set_volume(float)
- `float`: Volume level.
Sets the music volume.
### Sound
#### Sound() -> Sound
Creates a Sound object.
#### Sound:load(string)
- `string`: Name of the sound file located in `data/sound/`.
Loads a sound file. Supports .wav format.
#### Sound:play()
Plays the sound.
#### Sound:set_volume(float)
- `float`: Volume level.
Sets the sound volume.
#### Sound:set_pitch(float)
- `float`: Pitch level.
Sets the sound pitch.
#### Sound:set_listener(Vector3)
- `Vector3`: Listener position.
Sets the listener position.
#### Sound:set_position(Vector3)
- `Vector3`: Sound source position.
Sets the sound source position.
### AABB
#### AABB.min = Vector3
Minimum point of the AABB.
#### AABB.max = Vector3
Maximum point of the AABB.
### Model
#### Model:load(string)
- `string`: Name of the model file located in `data/model/`.
Supports .gltf and .glb formats.
#### Model.AABB = AABB
AABB of the model.

# Lua API
## Global Functions
Functions that can be called directly without creating an instance.

## Naming Conventions
Global functions belong to tables that start with lowercase letters (e.g., `random`).
Functions are called using a dot (`.`) (e.g., `random.get_int_range(0, 10)`).

### random
#### random:get_int_range(integer, integer) -> integer
- `integer`: Minimum value.
- `integer`: Maximum value.
Returns a random integer within the specified range.
#### random:get_float_range(float, float) -> float
- `float`: Minimum value.
- `float`: Maximum value.
Returns a random floating-point number within the specified range.

### window
#### window.name() -> string
Returns the window name.
#### window.rename(string)
- `string`: New window name.
Changes the window name.
#### window.size() -> Vector2
Returns the window size.
#### window.resize(Vector2)
- `Vector2`: New window size.
Changes the window size.
#### window.resized() -> bool
Returns whether the window was resized in the current frame.
#### window.half() -> Vector2
Returns the half size of the window.
#### window.set_fullscreen(bool)
- `bool`: Whether to set the window to fullscreen.
Toggles fullscreen or windowed mode.

### renderer
#### renderer.clear_color() -> color
Returns the screen clear color.
#### renderer.set_clear_color(color)
- `Color`: New clear color.
Sets the screen clear color.

### scene
#### scene.main_Camera() -> Camera
Returns the main camera of the scene.
#### scene.size() -> Vector2
Returns the size of the scene.
#### scene.resize(Vector2)
- `Vector2`: New scene size.
Changes the scene size.
#### scene.half() -> Vector2
Returns the half size of the scene.

### collision
#### collision.aabb_aabb(AABB, AABB) -> bool
- `AABB`: First AABB.
- `AABB`: Second AABB.
Checks for a collision between two AABBs.

### keyboard
#### keyboard.is_pressed(code) -> bool
- `code`: Key code.
Checks if a key was pressed in the current frame.
#### keyboard.is_released(code) -> bool
- `code`: Key code.
Checks if a key was released in the current frame.
#### keyboard.is_down(code) -> bool
- `code`: Key code.
Checks if a key is currently being held down.
#### keyboard.A = code
#### keyboard.B = code
#### keyboard.C = code
#### keyboard.D = code
#### keyboard.E = code
#### keyboard.F = code
#### keyboard.G = code
#### keyboard.H = code
#### keyboard.I = code
#### keyboard.J = code
#### keyboard.K = code
#### keyboard.L = code
#### keyboard.M = code
#### keyboard.N = code
#### keyboard.O = code
#### keyboard.P = code
#### keyboard.Q = code
#### keyboard.R = code
#### keyboard.S = code
#### keyboard.T = code
#### keyboard.U = code
#### keyboard.V = code
#### keyboard.W = code
#### keyboard.X = code
#### keyboard.Y = code
#### keyboard.Z = code
#### keyboard.key0 = code
#### keyboard.key1 = code
#### keyboard.key2 = code
#### keyboard.key3 = code
#### keyboard.key4 = code
#### keyboard.key5 = code
#### keyboard.key6 = code
#### keyboard.key7 = code
#### keyboard.key8 = code
#### keyboard.key9 = code
#### keyboard.F1 = code
#### keyboard.F2 = code
#### keyboard.F3 = code
#### keyboard.F4 = code
#### keyboard.F5 = code
#### keyboard.F6 = code
#### keyboard.F7 = code
#### keyboard.F8 = code
#### keyboard.F9 = code
#### keyboard.F10 = code
#### keyboard.F11 = code
#### keyboard.F12 = code
#### keyboard.UP = code
#### keyboard.DOWN = code
#### keyboard.LEFT = code
#### keyboard.RIGHT = code
#### keyboard.ESCAPE = code
#### keyboard.SPACE = code
#### keyboard.ENTER = code
#### keyboard.BACKSPACE = code
#### keyboard.TAB = code
#### keyboard.LSHIFT = code
#### keyboard.RSHIFT = code
#### keyboard.LCTRL = code
#### keyboard.RCTRL = code
#### keyboard.ALT = code

### mouse
Center of the window/scene is the origin.
#### mouse.position() -> Vector2
Returns the mouse position.
#### mouse.position_on_scene() -> Vector2
Returns the mouse position in the scene.
#### mouse.set_position(Vector2)
- `Vector2`: New mouse position.
Sets the mouse position.
#### mouse.set_position_on_scene(Vector2)
- `Vector2`: New mouse position in the scene.
Sets the mouse position within the scene.
#### mouse.is_pressed(code) -> bool
- `code`: Mouse button code.
Checks if a mouse button was pressed in the current frame.
#### mouse.is_released(code) -> bool
- `code`: Mouse button code.
Checks if a mouse button was released in the current frame.
#### mouse.is_down(code) -> bool
- `code`: Mouse button code.
Checks if a mouse button is currently being held down.
#### mouse.scroll_wheel() -> Vector2
Returns the scroll wheel movement.
#### mouse.hide_cursor(bool)
- `bool`: Whether to hide the mouse cursor.
Toggles cursor visibility.
#### mouse.LEFT = code
#### mouse.RIGHT = code
#### mouse.MIDDLE = code
#### mouse.X1 = code
#### mouse.X2 = code

### gamepad
#### gamepad.is_connected() -> bool
Checks if a gamepad is connected.
#### gamepad.is_pressed(code) -> bool
- `code`: Gamepad button code.
Checks if a gamepad button was pressed in the current frame.
#### gamepad.is_released(code) -> bool
- `code`: Gamepad button code.
Checks if a gamepad button was released in the current frame.
#### gamepad.is_down(code) -> bool
- `code`: Gamepad button code.
Checks if a gamepad button is currently being held down.
#### gamepad.left_stick() -> Vector2
Gets the left stick delta movement.
#### gamepad.right_stick() -> Vector2
Gets the right stick delta movement.
#### gamepad.INVALID = code
#### gamepad.A = code
#### gamepad.B = code
#### gamepad.X = code
#### gamepad.Y = code
#### gamepad.BACK = code
#### gamepad.START = code
#### gamepad.GUIDE = code
#### gamepad.LEFTSTICK = code
#### gamepad.RIGHTSTICK = code
#### gamepad.LEFTSHOULDER = code
#### gamepad.RIGHTSHOULDER = code
#### gamepad.DPAD_UP = code
#### gamepad.DPAD_DOWN = code
#### gamepad.DPAD_LEFT = code
#### gamepad.DPAD_RIGHT = code
#### gamepad.MISC1 = code
#### gamepad.PADDLE1 = code
#### gamepad.PADDLE2 = code
#### gamepad.PADDLE3 = code
#### gamepad.PADDLE4 = code
#### gamepad.TOUCHPAD = code
