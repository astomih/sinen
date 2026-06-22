local sn = require("@sinen")
local renderTexture: sn.RenderTexture = sn.RenderTexture.new()
local width: number = 500
local height: number = 500
local out: sn.Texture = sn.Texture.new(width, height)
renderTexture:create(width, height)

local model: sn.Model = sn.Model.new("DamagedHelmet.glb")

local transform: sn.Transform = sn.Transform.new()
transform.rotation = sn.Vec3.new(90, 0, 0)

local pos: sn.Vec3 = sn.Vec3.new(1, 1, 3)
local camera: sn.Camera3D = sn.Camera3D.new()

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
	camera:lookat(pos, sn.Vec3.new(0), sn.Vec3.new(0, 1, 0))
end

function draw()
	sn.Graphics.resetGraphicsPipeline()

	sn.Graphics.begin3D(camera)
	sn.Graphics.beginRenderTarget(renderTexture)
	sn.Graphics.drawModel(model, transform)
	sn.Graphics.endRenderTarget()
	sn.Graphics.finish()
	sn.Graphics.readbackTexture(renderTexture, out)

	sn.Graphics.drawImage(out, sn.Rect.new(sn.Pivot.Center, sn.Window.center(), sn.Vec2.new(width, height)))
end
