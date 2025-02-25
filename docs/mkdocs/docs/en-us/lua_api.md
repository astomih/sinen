# Lua API
## Class
### vector2
#### vector2(x, y) -> vector2
- x: The x component of the vector.
- y: The y component of the vector.
Create a vector2 object.
#### vector2.x = float
x component of vector2.
#### vector2.y = float
y component of vector2.
#### vector2:add(vector2) -> vector2
Add a vector2 to this vector2.
#### vector2:sub(vector2) -> vector2
Subtract a vector2 from this vector2.
#### vector2:mul(vector2) -> vector2
Multiply this vector2 by a vector2.
#### vector2:div(vector2) -> vector2
Divide this vector2 by a vector2.
#### vector2:length() -> float
Get the length of this vector2.
#### vector2:normalize() -> vector2
Normalize this vector2.
### vector3
#### vector3(x, y, z) -> vector3
- x: The x component of the vector.
- y: The y component of the vector.
- z: The z component of the vector.
Create a vector3 object.
#### vector3.x = float
x component of vector3.
#### vector3.y = float
y component of vector3.
#### vector3.z = float
z component of vector3.
#### vector3:add(vector3) -> vector3
Add a vector3 to this vector3.
#### vector3:sub(vector3) -> vector3
Subtract a vector3 from this vector3.
#### vector3:mul(vector3) -> vector3
Multiply this vector3 by a vector3.
#### vector3:div(vector3) -> vector3
Divide this vector3 by a vector3.
#### vector3:length() -> float
Get the length of this vector3.
#### vector3:normalize() -> vector3
Normalize this vector3.
#### vector3:copy() -> vector3
Copy this vector3.
#### vector3:forward() -> vector3
Get the forward vector3.
### point2i
#### point2i(x, y) -> point2i
Create a Point2i object.
#### point2i.x = int
x component of Point2i.
#### point2i.y = int
y component of Point2i.
### color
#### color(r, g, b, a) -> color
- r: The red component of the color.
- g: The green component of the color.
- b: The blue component of the color.
- a: The alpha component of the color.
Create a color object.
#### color.r = float
red component of color.
#### color.g = float
green component of color.
#### color.b = float
blue component of color.
#### color.a = float
alpha component of color.
### texture
#### texture()
Create a texture object.
#### texture:load(string)
string: The path to the texture.
Load a texture from a file.
#### texture:fill_color(color)
color: The color to fill the texture with.
Fill the texture with a color.
#### texture:blend_color(color)
color: The color to blend the texture with.
Blend the texture with a color.
#### texture:copy() -> texture
Copy the texture.
#### texture:size() -> vector2
Get the size of the texture.
### font
#### font() -> font
Create a font object.
#### font:load(string)
string: The path to the font.
Load a font from a file.
#### font:render_text(texture,string, color)
- texture: The texture to render the text to.
- string: The text to render.
- color: The color to render the text with.
Render text to a texture.


### draw2d
draw2d is a class that draws 2D images.
#### draw2d:draw()
Draws the image.
#### draw2d.position = vector2
The position of the image.
#### draw2d.scale = vector2
The scale of the image.
#### draw2d.rotation = float
The rotation of the image.
#### draw2d.texture = texture
The texture of the image.
#### draw2d.vertex_name = string
The name of the model name.
#### draw2d:user_data_at(integer, float)
- integer: The index of the user data.
- float: The value of the user data.
#### draw2d:add(vector2,float,vector2)
- vector2: The position of the image.
- float: The rotation of the image.
- vector2: The scale of the image.
#### draw2d:at(integer, vector2,float,vector2)
- integer: The index of the image.
- vector2: The position of the image.
- float: The rotation of the image.
- vector2: The scale of the image.
#### draw2d:clear()
Clears the images.
### drawui
drawui is a class that draws UI images.
#### drawui:draw()
Draws the image.
#### drawui.position = vector2
The position of the image.
#### drawui.scale = vector2
The scale of the image.
#### drawui.rotation = float
The rotation of the image.
#### drawui.texture = texture
The texture of the image.
#### drawui.vertex_name = string
The name of the model name.
#### drawui:user_data_at(integer, float)
- integer: The index of the user data.
- float: The value of the user data.
#### drawui:add(vector2,float,vector2)
- vector2: The position of the image.
- float: The rotation of the image.
- vector2: The scale of the image.
#### drawui:at(integer, vector2,float,vector2)
- integer: The index of the image.
- vector2: The position of the image.
- float: The rotation of the image.
- vector2: The scale of the image.
### draw3d
draw3d is a class that draws 3D images.
#### draw3d:draw()
Draws the image.
#### draw3d.position = vector3
The position of the image.
#### draw3d.scale = vector3
The scale of the image.
#### draw3d.rotation = vector3
The rotation of the image.
#### draw3d.texture = texture
The texture of the image.
#### draw3d.vertex_name = string
The name of the model name.
#### draw3d:user_data_at(integer, float)
- integer: The index of the user data.
- float: The value of the user data.
#### draw3d:add(vector3,vector3,vector3)
- vector3: The position of the image.
- vector3: The rotation of the image.
- vector3: The scale of the image.
#### draw3d:at(integer, vector3,vector3,vector3)
- integer: The index of the image.
- vector3: The position of the image.
- vector3: The rotation of the image.
- vector3: The scale of the image.
#### draw3d:clear()
Clears the images.
### camera
#### camera:look_at(vector3, vector3, vector3)
- vector3: The position of the camera.
- vector3: The position to look at.
- vector3: The up vector.
Look at a position.
#### camera:perspective(float, float, float, float)
- float: The field of view.
- float: The aspect ratio.
- float: The near plane.
- float: The far plane.
Set the perspective of the camera.
#### camera:orthographic(float, float, float, float, float, float)
- float: The left plane.
- float: The right plane.
- float: The bottom plane.
- float: The top plane.
- float: The near plane.
- float: The far plane.
Set the orthographic projection of the camera.
#### camera.position = vector3
The position of the camera.
#### camera.target = vector3
The target of the camera.
#### camera.up = vector3
The up vector of the camera.
### music
#### music:load(string)
- string: The path to the music.
Load music from a file.
#### music:play()
Play the music.
#### music:set_volume(float)
- float: The volume to set the music to.
Set the volume of the music.
### sound
#### sound:load(string)
- string: The path to the sound.
Load sound from a file.
#### sound:play()
Play the sound.
#### sound:set_volume(float)
- float: The volume to set the sound to.
Set the volume of the sound.
#### sound:set_pitch(float)
- float: The pitch to set the sound to.
Set the pitch of the sound.
#### sound:set_listener(vector3)
- vector3: The position of the listener.
Set the position of the listener.
#### sound:set_position(vector3)
- vector3: The position of the sound.
Set the position of the sound.
### aabb
#### aabb.min = vector3
The minimum point of the AABB.
#### aabb.max = vector3
The maximum point of the AABB.
### model
#### model:load(string)
- string: The path to the model.
Load a model from a file.
#### model.aabb = aabb
The AABB of the model.
### random
#### random:get_int_range(integer, integer) -> integer
- integer: The minimum value.
- integer: The maximum value.
Get a random integer in a range.
#### random:get_float_range(float, float) -> float
- float: The minimum value.
- float: The maximum value.
Get a random float in a range.

## Table
### window
#### window.name() -> string
Get the name of the window.
#### window.rename(string)
- string: The name to rename the window to.
Rename the window. 
#### window.size() -> vector2
Get the size of the window.
#### window.resize(vector2)
- vector2: The size to resize the window to.
Resize the window.
#### window.resized() -> bool
Get whether or not the window has been resized.
#### window.center() -> vector2
Get the center of the window.
#### window.set_fullscreen(bool)
- bool: Whether or not to set the window to fullscreen.
Set or unset the window to fullscreen.
### renderer
#### renderer.set_skybox(texture)
- texture: The texture to set the skybox to.
Set the skybox texture.
#### renderer.clear_color() -> color
Get the clear color of the renderer.
#### renderer.set_clear_color(color)
- color: The color to set the clear color to.
Set the clear color of the renderer.
#### renderer.at_render_texture_user_data(integer, float)
- integer: The index of the user data.
- float: The value of the user data.
Shader user data for the render texture.
### scene
#### scene.load(string)
- string: The name of the scene to load.
Load a scene from data/scene/.
#### scene.main_camera() -> camera
Get the main camera of the scene.
#### scene.get_actor(string) -> actor
- string: The name of the actor to get.
Get an actor from the scene.
#### scene.size() -> vector2
Get the size of the scene.
#### scene.resize(vector2)
- vector2: The size to resize the scene to.
Resize the scene.
#### scene.center() -> vector2
Get the center of the scene.
### collision
#### collision.aabb_aaabb(aabb, aabb) -> bool
- aabb: The first AABB.
- aabb: The second AABB.
Check if two AABBs are colliding.
### keyboard
#### keyboard.is_pressed(code) -> bool
- code: The key to check.
Check if a key is pressed.
#### keyboard.is_released(code) -> bool
- code: The key to check.
Check if a key is released.
#### keyboard.is_down(code) -> bool
- code: The key to check.
Check if a key is down.

#### keyboard.A = code
The key code for A.
#### keyboard.B = code
The key code for B.
#### keyboard.C = code
The key code for C.
#### keyboard.D = code
The key code for D.
#### keyboard.E = code
The key code for E.
#### keyboard.F = code
The key code for F.
#### keyboard.G = code
The key code for G.
#### keyboard.H = code
The key code for H.
#### keyboard.I = code
The key code for I.
#### keyboard.J = code
The key code for J.
#### keyboard.K = code
The key code for K.
#### keyboard.L = code
The key code for L.
#### keyboard.M = code
The key code for M.
#### keyboard.N = code
The key code for N.
#### keyboard.O = code
The key code for O.
#### keyboard.P = code
The key code for P.
#### keyboard.Q = code
The key code for Q.
#### keyboard.R = code
The key code for R.
#### keyboard.S = code
The key code for S.
#### keyboard.T = code
The key code for T.
#### keyboard.U = code
The key code for U.
#### keyboard.V = code
The key code for V.
#### keyboard.W = code
The key code for W.
#### keyboard.X = code
The key code for X.
#### keyboard.Y = code
The key code for Y.
#### keyboard.Z = code
The key code for Z.
#### keyboard.key0 = code
The key code for 0.
#### keyboard.key1 = code
The key code for 1.
#### keyboard.key2 = code
The key code for 2.
#### keyboard.key3 = code
The key code for 3.
#### keyboard.key4 = code
The key code for 4.
#### keyboard.key5 = code
The key code for 5.
#### keyboard.key6 = code
The key code for 6.
#### keyboard.key7 = code
The key code for 7.
#### keyboard.key8 = code
The key code for 8.
#### keyboard.key9 = code
The key code for 9.
#### keyboard.F1 = code
The key code for F1.
#### keyboard.F2 = code
The key code for F2.
#### keyboard.F3 = code
The key code for F3.
#### keyboard.F4 = code
The key code for F4.
#### keyboard.F5 = code
The key code for F5.
#### keyboard.F6 = code
The key code for F6.
#### keyboard.F7 = code
The key code for F7.
#### keyboard.F8 = code
The key code for F8.
#### keyboard.F9 = code
The key code for F9.
#### keyboard.F10 = code
The key code for F10.
#### keyboard.F11 = code
The key code for F11.
#### keyboard.F12 = code
The key code for F12.
#### keyboard.UP = code
The key code for the up arrow.
#### keyboard.DOWN = code
The key code for the down arrow.
#### keyboard.LEFT = code
The key code for the left arrow.
#### keyboard.RIGHT = code
The key code for the right arrow.
#### keyboard.ESCAPE = code
The key code for the escape key.
#### keyboard.SPACE = code
The key code for the space key.
#### keyboard.ENTER = code
The key code for the enter key.
#### keyboard.BACKSPACE = code
The key code for the backspace key.
#### keyboard.TAB = code
The key code for the tab key.
#### keyboard.LSHIFT = code
The key code for the left shift key.
#### keyboard.RSHIFT = code
The key code for the right shift key.
#### keyboard.LCTRL = code
The key code for the left control key.
#### keyboard.RCTRL = code
The key code for the right control key.
#### keyboard.ALT = code
The key code for the alt key.
### mouse
#### mouse.position() -> vector2
Get the position of the mouse.
#### mouse.position_on_scene() -> vector2
Get the position of the mouse on the scene.
#### mouse.set_position(vector2)
- vector2: The position to set the mouse to.
Set the position of the mouse.
#### mouse.set_position_on_scene(vector2)
- vector2: The position to set the mouse to.
Set the position of the mouse on the scene.
#### mouse.is_pressed(code) -> bool
- code: The button to check.
Check if a mouse button is pressed.
#### mouse.is_released(code) -> bool
- code: The button to check.
Check if a mouse button is released.
#### mouse.is_down(code) -> bool
- code: The button to check.
Check if a mouse button is down.
#### mouse.scroll_wheel() -> vector2
Get the scroll wheel of the mouse.
#### mouse.hide_cursor(bool)
- bool: Whether to hide the cursor or not.
Hide the cursor.
#### mouse.LEFT = code
The mouse button code for the left button.
#### mouse.RIGHT = code
The mouse button code for the right button.
#### mouse.MIDDLE = code
The mouse button code for the middle button.
#### mouse.X1 = code
The mouse button code for the first extra button.
#### mouse.X2 = code
The mouse button code for the second extra button.
### gamepad
#### gamepad.is_connected() -> bool
Check if a gamepad is connected.
#### gamepad.is_pressed(code) -> bool
- code: The button to check.
Check if a gamepad button is pressed.
#### gamepad.is_released(code) -> bool
- code: The button to check.
Check if a gamepad button is released.
#### gamepad.is_down(code) -> bool
- code: The button to check.
Check if a gamepad button is down.
#### gamepad.left_stick() -> vector2
Get the left stick of the gamepad.
#### gamepad.right_stick() -> vector2
Get the right stick of the gamepad.
#### gamepad.INVALID = code
The gamepad button code for an invalid button.
#### gamepad.A = code
The gamepad button code for A.
#### gamepad.B = code
The gamepad button code for B.
#### gamepad.X = code
The gamepad button code for X.
#### gamepad.Y = code
The gamepad button code for Y.
#### gamepad.BACK = code
The gamepad button code for the back button.
#### gamepad.START = code
The gamepad button code for the start button.
#### gamepad.GUIDE = code
The gamepad button code for the guide button.
#### gamepad.LEFTSTICK = code
The gamepad button code for the left stick button.
#### gamepad.RIGHTSTICK = code
The gamepad button code for the right stick button.
#### gamepad.LEFTSHOULDER = code
The gamepad button code for the left shoulder button.
#### gamepad.RIGHTSHOULDER = code
The gamepad button code for the right shoulder button.
#### gamepad.DPAD_UP = code
The gamepad button code for the dpad up button.
#### gamepad.DPAD_DOWN = code
The gamepad button code for the dpad down button.
#### gamepad.DPAD_LEFT = code
The gamepad button code for the dpad left button.
#### gamepad.DPAD_RIGHT = code
The gamepad button code for the dpad right button.
#### gamepad.MISC1 = code
The gamepad button code for the first misc button.
#### gamepad.PADDLE1 = code
The gamepad button code for the first paddle button.
#### gamepad.PADDLE2 = code
The gamepad button code for the second paddle button.
#### gamepad.PADDLE3 = code
The gamepad button code for the third paddle button.
#### gamepad.PADDLE4 = code
The gamepad button code for the fourth paddle button.
#### gamepad.TOUCHPAD = code
The gamepad button code for the touchpad button.
