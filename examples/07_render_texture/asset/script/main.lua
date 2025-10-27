local renderTexture = sn.RenderTexture.new()
local out = sn.Texture.new()
out:load("logo.png")
local sx = out:size().x
local sy = out:size().y
renderTexture:create(math.floor(sx), math.floor(sy))

local model = sn.Model.new()
model:load("DamagedHelmet.glb")

local transform = sn.Transform.new()
transform.rotation = sn.Vec3.new(90, 0, 0)

local pos = sn.Vec3.new(1, 1, 3)

function Update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
    sn.Graphics.getCamera():lookat(pos, sn.Vec3.new(0), sn.Vec3.new(0, 1, 0))
end

function Draw()
    sn.Graphics.bindPipeline(sn.BuiltinPipelines.get3D())
    sn.Graphics.setRenderTarget(renderTexture)
    sn.Graphics.drawModel(model, transform, model:getMaterial())
    -- Draw texture
    sn.Graphics.flush()
    sn.Graphics.readbackTexture(renderTexture, out)
    sn.Graphics.bindPipeline(sn.BuiltinPipelines.get2D())
    sn.Graphics.drawImage(out, sn.Rect.new(0, 0, sx, sy))
end
