-- TODO: Shading
local hello_texture = texture()
hello_texture:fill_color(color(1, 1, 1, 1))
local hello_model = model()
hello_model:load("Suzanne.gltf")
local hello_drawer = draw3d(hello_texture)
hello_drawer.position = vector3(0, 0, 0)
hello_drawer.model = hello_model


local pos = vector3(1, 1, 1)
local speed = 0.1

function Update()
  if keyboard.is_down(keyboard.W) then
    pos.z = pos.z + speed
  end

  if keyboard.is_down(keyboard.S) then
    pos.z = pos.z - speed
  end

  if keyboard.is_down(keyboard.A) then
    pos.x = pos.x - speed
  end

  if keyboard.is_down(keyboard.D) then
    pos.x = pos.x + speed
  end

  if keyboard.is_down(keyboard.SPACE) then
    pos.y = pos.y + speed
  end

  if keyboard.is_down(keyboard.LSHIFT) then
    pos.y = pos.y - speed
  end

  print(pos.x, pos.y, pos.z)

  scene.main_camera():lookat(pos, vector3(0, 0, 0), vector3(0, 1, 0))
end

function Draw()
  -- Draw texture
  hello_drawer:draw()
end
