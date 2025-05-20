# Python API
## About call
## Classes

### Vector2
#### Vector2(x, y) -> Vector2
- `x`: X component of the vector.
- `y`: Y component of the vector.
Creates a Vector2 object.

#### Vector2.x = float
X component of the Vector2.
#### Vector2.y = float
Y component of the Vector2.
#### Vector2.length() -> float
Returns the length of the vector.
#### Vector2.normalize() -> Vector2
Normalizes the vector.
#### Vector2.copy() -> Vector2
Copies the vector.
#### Vector2.dot(Vector2, Vector2) -> float
- `Vector2`: Vector 1
- `Vector2`: Vector 2 
Gets the dot product of two vectors.
#### Vector2.lerp(Vector2, Vector2, float) -> Vector2
- `Vector2`: Vector 1
- `Vector2`: Vector 2
- `float`: Interpolation factor (0.0~1.0)
Gets the linear interpolation between two vectors.
#### Vector2.reflect(Vector2, Vector2) -> Vector2
- `Vector2`: Incident vector
- `Vector2`: Normal vector
Gets the reflection vector of the incident vector based on the normal vector.
### Vector3
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
#### Vector3.length() -> float
Returns the length of the vector.
#### Vector3.normalize() -> Vector3
Normalizes the vector.
#### Vector3.copy() -> Vector3
Copies the vector. Since Lua primarily passes references, use this method if a copy is needed.
#### Vector3.forward(Vector3) -> Vector3
- `Vector3`: Axis vector  
Gets the forward vector of the specified axis vector.
#### Vector3.dot(Vector3, Vector3) -> float
- `Vector3`: Vector 1
- `Vector3`: Vector 2
Gets the dot product of two vectors.
#### Vector3.cross(Vector3, Vector3) -> Vector3
- `Vector3`: Vector 1
- `Vector3`: Vector 2
Gets the cross product of two vectors.
#### Vector3.lerp(Vector3, Vector3, float) -> Vector3
- `Vector3`: Vector 1
- `Vector3`: Vector 2
- `float`: Interpolation factor (0.0~1.0)
Gets the linear interpolation between two vectors.
#### Vector3.reflect(Vector3, Vector3) -> Vector3
- `Vector3`: Incident vector
- `Vector3`: Normal vector
Gets the reflection vector of the incident vector based on the normal vector.

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
#### Texture.load(string)
- `string`: Path to the texture.
Loads a texture from a file.
#### Texture.fill_color(Color)
- `Color`: Color to fill the texture with.
Fills the texture with a color.
#### Texture.blend_color(Color)
- `Color`: Color to blend the texture with.
Blends the texture with a color.
#### Texture.copy() -> Texture
Copies the texture.
#### Texture.size() -> Vector2
Gets the size of the texture.

### RenderTexture
#### RenderTexture()
Creates a RenderTexture object.
#### RenderTexture:create(int, int)
- `int`: Width.
- `int`: Height.  
Creates a render texture.

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
#### Draw2D.add(Vector2, float, Vector2)
- `Vector2`: Position.
- `float`: Clockwise rotation angle.
- `Vector2`: Scale.  
Adds multiple objects with the same texture and model for rendering. One object already exists at the time of instance creation.
#### Draw2D.clear()
Clears added objects. However, the initially created object is not cleared.

### Draw3D
#### Draw3D.draw(Texture)
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
#### Draw3D.add(Vector3, Vector3, Vector3)
- `Vector3`: Position.
- `Vector3`: Rotation.
- `Vector3`: Scale.  
Adds multiple objects with the same texture and model for rendering. One object already exists at the time of instance creation.
#### Draw3D.clear()
Clears added objects. However, the initially created object is not cleared.
### UniformData
#### UniformData() -> UniformData
Creates a uniform data object.
#### UniformData.add(float)
- float: Data  
Adds uniform data.
#### UniformData.at(float, int)
- `float`: Data
- `int`: Index  
Adds uniform data at the specified index.
### Shader
#### Shader() -> Shader
Creates a shader object.
#### Shader.load_vertex_shader(string, int)
- `string`: Name of the vertex shader file located in data/shader/.
- `int`: Number of UniformData.
Loads a vertex shader.
#### Shader.load_fragment_shader(string, int)
- `string`: Name of the fragment shader file located in data/shader/.
- `int`: Number of UniformData.
Loads a fragment shader.
#### Shader.compile_and_load_vertex_shader(string,int)
- `string`: Vertex shader source code
- `int`: Number of UniformData
Compile and load vertex shader  
`slangc` must be installed
#### Shader.compile_and_load_fragment_shader(string,int)
- `string`: Fragment shader source code
- `int`: Number of UniformData
Compile and load fragment shader  
`slangc` must be installed
### RenderPipeline2D
#### RenderPipeline2D() -> RenderPipeline2D
Creates a 2D render pipeline object.
#### RenderPipeline2D.set_vertex_shader(Shader)
- `Shader`: Vertex shader
Sets the vertex shader.
#### RenderPipeline2D.set_fragment_shader(Shader)
- `Shader`: Fragment shader
Sets the fragment shader.
#### RenderPipeline2D.build()
Builds the render pipeline.
### RenderPipeline3D
#### RenderPipeline3D() -> RenderPipeline3D
Creates a 3D render pipeline object.
#### RenderPipeline3D.set_vertex_shader(Shader)
- `Shader`: Vertex shader
Sets the vertex shader.
#### RenderPipeline3D.set_fragment_shader(Shader)
- `Shader`: Fragment shader  
Sets the fragment shader.
#### RenderPipeline3D.build()
Builds the render pipeline.
### Camera
#### Camera.lookat(Vector3, Vector3, Vector3)
- `Vector3`: Position.
- `Vector3`: Target.
- `Vector3`: Up direction.
Sets the camera's position, target, and up direction.
#### Camera.perspective(float, float, float, float)
- `float`: Field of view.
- `float`: Aspect ratio.
- `float`: Near plane.
- `float`: Far plane.
Sets the camera's perspective projection.
#### Camera.orthographic(float, float, float, float, float, float)
- `float`: Left.
- `float`: Right.
- `float`: Bottom.
- `float`: Top.
- `float`: Near plane.
- `float`: Far plane.
Sets the camera's orthographic projection.
#### Camera.is_aabb_in_frustum(AABB) -> bool
- `AABB`: AABB  
Checks if an AABB is within the camera's frustum.
#### Camera.position = Vector3
Camera position.
#### Camera.target = Vector3
Camera target.
#### Camera.up = Vector3
Camera up direction.
### Music
#### Music() -> Music
Creates a Music object.
#### Music.load(string)
- `string`: Name of the music file located in `data/music/`.
Loads a music file. Supports .ogg, .wav, and .mp3 formats.
#### Music.play()
Plays the music.
#### Music.set_volume(float)
- `float`: Volume level.
Sets the music volume.
### Sound
#### Sound() -> Sound
Creates a Sound object.
#### Sound.load(string)
- `string`: Name of the sound file located in `data/sound/`.
Loads a sound file. Supports .wav format.
#### Sound.play()
Plays the sound.
#### Sound.set_volume(float)
- `float`: Volume level.
Sets the sound volume.
#### Sound.set_pitch(float)
- `float`: Pitch level.
Sets the sound pitch.
#### Sound.set_listener(Vector3)
- `Vector3`: Listener position.
Sets the listener position.
#### Sound.set_position(Vector3)
- `Vector3`: Sound source position.
Sets the sound source position.
### AABB
#### AABB.min = Vector3
Minimum point of the AABB.
#### AABB.max = Vector3
Maximum point of the AABB.
### Model
#### Model.load(string)
- `string`: Name of the model file located in `data/model/`.
Supports .gltf and .glb formats.
#### Model.AABB = AABB
AABB of the model.

## Static classes
### Random
#### Random.get_int_range(integer, integer) -> integer
- `integer`: Minimum value.
- `integer`: Maximum value.
Returns a random integer within the specified range.
#### Random.get_float_range(float, float) -> float
- `float`: Minimum value.
- `float`: Maximum value.
Returns a random floating-point number within the specified range.

### Window
#### Window.name() -> string
Returns the window name.
#### Window.rename(string)
- `string`: New window name.
Changes the window name.
#### Window.size() -> Vector2
Returns the window size.
#### Window.resize(Vector2)
- `Vector2`: New window size.
Changes the window size.
#### Window.resized() -> bool
Returns whether the window was resized in the current frame.
#### Window.half() -> Vector2
Returns the half size of the window.
#### Window.set_fullscreen(bool)
- `bool`: Whether to set the window to fullscreen.
Toggles fullscreen or windowed mode.

### Renderer
#### Renderer.clear_color() -> color
Returns the screen clear color.
#### Renderer.set_clear_color(color)
- `Color`: New clear color.
Sets the screen clear color.
#### Renderer.begin_pipeline2D(RenderPipeline2D)
- `RenderPipeline2D`: Render pipeline.
Begins rendering with a 2D render pipeline.
#### Renderer.end_pipeline2D()
Ends rendering with a 2D render pipeline.
#### Renderer.begin_pipeline3D(RenderPipeline3D)
- `RenderPipeline3D`: Render pipeline.
Begins rendering with a 3D render pipeline.
#### Renderer.end_pipeline3D()
Ends rendering with a 3D render pipeline.
#### Renderer.begin_render_texture2d(RenderTexture)
- `RenderTexture`: Render texture.  
Begins rendering to a render texture 2d.
#### Renderer.begin_render_texture3d(RenderTexture)
- `RenderTexture`: Render texture.  
Begins rendering to a render texture 3d.
#### Renderer.end_render_texture(RenderTexture, Texture)
- `RenderTexture`: Render texture.
- `Texture`: Texture.
Ends rendering to a render texture.  
`Texture` is must be the same size as `RenderTexture`.

### Scene
#### Scene.camera() -> Camera
Returns the main camera of the scene.
#### Scene.size() -> Vector2
Returns the size of the scene.
#### Scene.resize(Vector2)
- `Vector2`: New scene size.
Changes the scene size.
#### Scene.half() -> Vector2
Returns the half size of the scene.

### Collision
#### Collison.aabb_aabb(AABB, AABB) -> bool
- `AABB`: First AABB.
- `AABB`: Second AABB.
Checks for a collision between two AABBs.

### Keyboard
#### Keyboard.is_pressed(code) -> bool
- `code`: Key code.
Checks if a key was pressed in the current frame.
#### Keyboard.is_released(code) -> bool
- `code`: Key code.
Checks if a key was released in the current frame.
#### Keyboard.is_down(code) -> bool
- `code`: Key code.
Checks if a key is currently being held down.
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
#### Keyboard.key0 = code
#### Keyboard.key1 = code
#### Keyboard.key2 = code
#### Keyboard.key3 = code
#### Keyboard.key4 = code
#### Keyboard.key5 = code
#### Keyboard.key6 = code
#### Keyboard.key7 = code
#### Keyboard.key8 = code
#### Keyboard.key9 = code
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
#### Keyboard.ENTER = code
#### Keyboard.BACKSPACE = code
#### Keyboard.TAB = code
#### Keyboard.LSHIFT = code
#### Keyboard.RSHIFT = code
#### Keyboard.LCTRL = code
#### Keyboard.RCTRL = code
#### Keyboard.ALT = code

### Mouse
Center of the window/scene is the origin.
#### Mouse.position() -> Vector2
Returns the mouse position.
#### Mouse.position_on_scene() -> Vector2
Returns the mouse position in the scene.
#### Mouse.set_position(Vector2)
- `Vector2`: New mouse position.
Sets the mouse position.
#### Mouse.set_position_on_scene(Vector2)
- `Vector2`: New mouse position in the scene.
Sets the mouse position within the scene.
#### Mouse.is_pressed(code) -> bool
- `code`: Mouse button code.
Checks if a mouse button was pressed in the current frame.
#### Mouse.is_released(code) -> bool
- `code`: Mouse button code.
Checks if a mouse button was released in the current frame.
#### Mouse.is_down(code) -> bool
- `code`: Mouse button code.
Checks if a mouse button is currently being held down.
#### Mouse.scroll_wheel() -> Vector2
Returns the scroll wheel movement.
#### Mouse.hide_cursor(bool)
- `bool`: Whether to hide the mouse cursor.
Toggles cursor visibility.
#### Mouse.set_relative(bool)
- `bool`: Whether to set relative coordinates.  
Sets the mouse position relative to the window size.
#### Mouse.is_relative() -> bool
Returns whether the mouse is in relative coordinates.
#### Mouse.LEFT = code
#### Mouse.RIGHT = code
#### Mouse.MIDDLE = code
#### Mouse.X1 = code
#### Mouse.X2 = code

### Gamepad
#### Gamepad.is_connected() -> bool
Checks if a gamepad is connected.
#### Gamepad.is_pressed(code) -> bool
- `code`: Gamepad button code.
Checks if a gamepad button was pressed in the current frame.
#### Gamepad.is_released(code) -> bool
- `code`: Gamepad button code.
Checks if a gamepad button was released in the current frame.
#### Gamepad.is_down(code) -> bool
- `code`: Gamepad button code.
Checks if a gamepad button is currently being held down.
#### Gamepad.left_stick() -> Vector2
Gets the left stick delta movement.
#### Gamepad.right_stick() -> Vector2
Gets the right stick delta movement.
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
