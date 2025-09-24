local texture = sn.Texture()
texture:fill(sn.Color(1, 1, 1, 1))
local material = sn.Material()
material:append_texture(texture)
local model = sn.Model()
model:load("Suzanne.gltf")

local transform = sn.Transform()
transform.position = sn.Vec3(0, 0, 0)
transform.scale = sn.Vec3(1, 1, 1)


local pos = sn.Vec3(1, 1, 3)
local at = sn.Vec3(0)
local up = sn.Vec3(0, 1, 0)
sn.Graphics.get_camera():lookat(pos, at, up)

function update()
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function draw()
    sn.Graphics.draw_model(model, transform, material)
end
