import sinen as sn
texture = sn.Texture()
texture.fill_color(sn.Color(1, 1, 1, 1))
model = sn.Model()
model.load("Suzanne.gltf")
draw3d = sn.Draw3D(texture)
draw3d.position = sn.Vec3(0, 0, 0)
draw3d.model = model


pos = sn.Vec3(1.0, 1.0, 3.0)
at = sn.Vec3(0, 0, 0)
up = sn.Vec3(0, 1, 0)
speed = 0.1

def update():
  if sn.Keyboard.is_down(sn.Keyboard.W): 
    sn.logger.info(str(pos.z))
    pos.z -= speed

  if sn.Keyboard.is_down(sn.Keyboard.S):
    pos.z = pos.z + speed

  if sn.Keyboard.is_down(sn.Keyboard.A):
    pos.x = pos.x - speed
  
  if sn.Keyboard.is_down(sn.Keyboard.D):
    pos.x = pos.x + speed

  if sn.Keyboard.is_down(sn.Keyboard.LSHIFT):
    pos.y = pos.y - speed

  if sn.Keyboard.is_down(sn.Keyboard.SPACE):
    pos.y = pos.y + speed

  sn.Scene.camera().lookat(pos, at, up)

def draw():
  # Draw texture
  draw3d.draw()
