from sinen import *

texture = Texture()
texture.fill_color(Color(1, 1, 1, 1))
outTexture = Texture()
outTexture.load("logo.png")
render_texture = RenderTexture()
s = outTexture.size()
render_texture.create(int(s.x), int(s.y))

model = Model()
model.load("Suzanne.gltf")
draw3d = Draw3D()
draw3d.material.append(texture)
draw3d.position = Vec3(0, 0, 0)
draw3d.model = model

draw2d = Draw2D()
draw2d.material.append(outTexture)
draw2d.scale = outTexture.size()


pos = Vec3(1, 1, 3)
speed = 0.1

def update():
  if Keyboard.is_down(Keyboard.W): 
    pos.z -= speed

  if Keyboard.is_down(Keyboard.S):
    pos.z += speed

  if Keyboard.is_down(Keyboard.A):
    pos.x -= speed
  
  if Keyboard.is_down(Keyboard.D):
    pos.x += speed

  if Keyboard.is_down(Keyboard.LSHIFT):
    pos.y -= speed

  if Keyboard.is_down(Keyboard.SPACE):
    pos.y += speed

  Scene.camera().lookat(pos, Vec3(0, 0, 0), Vec3(0, 1, 0))

def draw():
  Graphics.begin_target2d(render_texture)
  # Draw texture
  draw3d.draw()
  Graphics.end_target(render_texture, outTexture)

  # Draw render texture
  draw2d.draw()
