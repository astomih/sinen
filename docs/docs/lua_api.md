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
### draw2d_instancing
draw2d_instancing is a class that draws 2D images with instancing.
#### draw2d_instancing:draw()
Draws the image.
#### draw2d_instancing:add(vector2,float,vector2)
- vector2: The position of the image.
- float: The rotation of the image.
- vector2: The scale of the image.
#### draw2d_instancing:at(integer, vector2,float,vector2)
- integer: The index of the image.
- vector2: The position of the image.
- float: The rotation of the image.
- vector2: The scale of the image.
#### draw2d_instancing:clear()
Clears the images.
#### draw2d_instancing.texture = texture
The texture of the image.
#### draw2d_instancing.vertex_name = string
The name of the model name.
#### draw2d_instancing:user_data_at(integer, float)
- integer: The index of the user data.
- float: The value of the user data.
### draw3d_instancing
draw3d_instancing is a class that draws 3D images with instancing.
#### draw3d_instancing:draw()
Draws the image.
#### draw3d_instancing:add(vector3,vector3,vector3)
- vector3: The position of the image.
- vector3: The rotation of the image.
- vector3: The scale of the image.
#### draw3d_instancing:at(integer, vector3,vector3,vector3)
- integer: The index of the image.
- vector3: The position of the image.
- vector3: The rotation of the image.
- vector3: The scale of the image.
#### draw3d_instancing:clear()
Clears the images.
#### draw3d_instancing.texture = texture
The texture of the image.
#### draw3d_instancing.vertex_name = string
The name of the model name.
#### draw3d_instancing:user_data_at(integer, float)
- integer: The index of the user data.
- float: The value of the user data.
###



## Table
### window
### renderer
