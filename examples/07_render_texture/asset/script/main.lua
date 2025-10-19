local texture = sn.Texture()
texture:fill(sn.Color(1, 1, 1, 1))
local renderTexture = sn.RenderTexture()
local out = sn.Texture()
out:load("logo.png")
local sx = out:size().x
local sy = out:size().y
renderTexture:create(sx, sy)

local model = sn.Model()
model:load("Suzanne.gltf")
local draw3d = sn.Draw3D(texture)
draw3d.position = sn.Vec3(0)
draw3d.model = model

local pos = sn.Vec3(1, 1, 3)

function update()
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
    sn.Graphics.get_camera():lookat(pos, sn.Vec3(0), sn.Vec3(0, 1, 0))
end

function draw()
    sn.Graphics.set_render_target(renderTexture)
    sn.Graphics.bind_pipeline(sn.BuiltinPipelines.get_3d())
    sn.Graphics.draw3d(draw3d)
    -- Draw texture
    sn.Graphics.flush()
    sn.Graphics.readback_texture(renderTexture, out)
    sn.Graphics.bind_pipeline(sn.BuiltinPipelines.get_2d())
    sn.Graphics.draw_image(out, sn.Rect(0, 0, sx, sy))
end
