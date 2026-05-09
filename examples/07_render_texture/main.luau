local sn = require("@sinen")
local renderTexture: sn.RenderTexture = sn.RenderTexture.new()
local width: number = 500
local height: number = 500
local out: sn.Texture = sn.Texture.new(width, height)
renderTexture:create(width, height)

local model: sn.Model = sn.Model.new()
model:load("DamagedHelmet.glb")

local transform: sn.Transform = sn.Transform.new()
transform.rotation = sn.Vec3.new(90, 0, 0)

local pos: sn.Vec3 = sn.Vec3.new(1, 1, 3)

local function rectFromCenter(pos2D: sn.Vec2, size: sn.Vec2): sn.Rect
	local half = sn.Graphics.getCamera2D():half()
	return sn.Rect.new(sn.Pivot.Center, sn.Vec2.new(half.x + pos2D.x, half.y - pos2D.y), size)
end

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

	sn.Graphics.drawImage(out, rectFromCenter(sn.Vec2.new(0), sn.Vec2.new(width, height)))
end
