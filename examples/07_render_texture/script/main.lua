-- TODO: Shading
local texture = Texture()
texture:fill_color(Color(1, 1, 1, 1))
local outTexture = Texture()
outTexture:load("logo.png")
local render_texture = RenderTexture()
render_texture:create(outTexture:size().x, outTexture:size().y)

local model = Model()
model:load("Suzanne.gltf")
local draw3d = Draw3D(texture)
draw3d.position = Vec3(0, 0, 0)
draw3d.model = model

local draw2d = Draw2D(outTexture)
draw2d.scale = outTexture:size()


local pos = Vec3(1, 1, 3)
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

  scene.camera():lookat(pos, Vec3(0, 0, 0), Vec3(0, 1, 0))
end

function Draw()
  renderer.begin_render_texture3d(render_texture)
  -- Draw texture
  draw3d:draw()
  renderer.end_render_texture(render_texture, outTexture)

  -- Draw render texture
  draw2d:draw()
end
