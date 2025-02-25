-- TODO: Shading
local texture = Texture()
texture:fill_color(Color(1, 1, 1, 1))
local model = Model()
model:load("Suzanne.gltf")
local draw3d = Draw3D(texture)
draw3d.position = Vector3(0, 0, 0)
draw3d.model = model


local pos = Vector3(1, 1, 3)
local speed = 0.1

function Update()
  if keyboard.is_down(keyboard.W) then
    pos.z = pos.z - speed
  end

  if keyboard.is_down(keyboard.S) then
    pos.z = pos.z + speed
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

  scene.main_camera():lookat(pos, Vector3(0, 0, 0), Vector3(0, 1, 0))
end

function Draw()
  -- Draw texture
  draw3d:draw()
end
