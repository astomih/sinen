local texture = Texture()
texture:fill_color(Color(1, 1, 1, 1))
local outTexture = Texture()
outTexture:load("logo.png")
local render_texture = RenderTexture()
local s = outTexture:size()
render_texture:create(s.x, s.y)

local model = Model()
model:load("Suzanne.gltf")
local draw3d = Draw3D()
draw3d.material:append(texture)
draw3d.position = Vec3(0, 0, 0)
draw3d.model = model

local draw2d = Draw2D()
draw2d.material:append(outTexture)
draw2d.scale = outTexture:size()


local pos = Vec3(1, 1, 3)
local speed = 0.1

function update()
    Scene.camera():lookat(pos, Vec3(0, 0, 0), Vec3(0, 1, 0))
end

function draw()
    Graphics.begin_target2d(render_texture)
    -- Draw texture
    draw3d:draw()
    Graphics.end_target(render_texture, outTexture)

    -- Draw render texture
    draw2d:draw()
end
