local texture = Texture()
texture:fill_color(Color(1, 1, 1, 1))
local model = Model()
model:load("Suzanne.gltf")
draw3d = Draw3D()
draw3d.material:append(texture)
draw3d.position = Vec3(0, 0, 0)
draw3d.model = model


local pos = Vec3(1.0, 1.0, 3.0)
local at = Vec3(0, 0, 0)
local up = Vec3(0, 1, 0)

function update()
    Scene.camera():lookat(pos, at, up)
end

function draw()
    -- Draw texture
    draw3d:draw()
end
