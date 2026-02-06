local renderTexture = sn.RenderTexture.new()
local width = 500
local height = 500
local out = sn.Texture.new(width, height)
renderTexture:create(width, height)

local model = sn.Model.new()
model:load("DamagedHelmet.glb")

local transform = sn.Transform.new()
transform.rotation = sn.Vec3.new(90, 0, 0)

local pos = sn.Vec3.new(1, 1, 3)

function update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
    sn.Graphics.getCamera3D():lookat(pos, sn.Vec3.new(0), sn.Vec3.new(0, 1, 0))
end

function draw()
    sn.Graphics.resetGraphicsPipeline()

    sn.Graphics.beginRenderTarget(renderTexture)
    sn.Graphics.drawModel(model, transform)
    sn.Graphics.endRenderTarget()
    sn.Graphics.readbackTexture(renderTexture, out)

    sn.Graphics.drawImage(out, sn.Rect.new(0, 0, width, height))
end
