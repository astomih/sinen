from sinen import *
texture = Texture()
texture.fill_color(Color(1, 1, 1, 1))
model = Model()
model.load("Suzanne.gltf")
draw3d = Draw3D(texture)
draw3d.position = Vec3(0, 0, 0)
draw3d.model = model


pos = Vec3(1.0, 1.0, 3.0)
at = Vec3(0, 0, 0)
up = Vec3(0, 1, 0)
speed = 0.1

def update():
  # if Keyboard.is_down(Keyboard.W): 
  #   pos.z -= speed

  # if Keyboard.is_down(Keyboard.S):
  #   pos.z += speed

  # if Keyboard.is_down(Keyboard.A):
  #   pos.x -= speed
  
  # if Keyboard.is_down(Keyboard.D):
  #   pos.x += speed

  # if Keyboard.is_down(Keyboard.LSHIFT):
  #   pos.y -= speed

  # if Keyboard.is_down(Keyboard.SPACE):
  #   pos.y += speed

  Scene.camera().lookat(pos, at, up)

def draw():
  # Draw texture
  draw3d.draw()
