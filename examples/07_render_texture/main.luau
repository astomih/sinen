local renderTexture: RenderTexture = RenderTexture.new()
local width: number = 500
local height: number = 500
local out: Texture = Texture.new(width, height)
renderTexture:create(width, height)

local model: Model = Model.new()
model:load("DamagedHelmet.glb")

local transform: Transform = Transform.new()
transform.rotation = Vec3.new(90, 0, 0)

local pos: Vec3 = Vec3.new(1, 1, 3)

function update()
	if Keyboard.isPressed(Keyboard.ESCAPE) then
		Script.load("main", ".")
	end
	Graphics.getCamera3D():lookat(pos, Vec3.new(0), Vec3.new(0, 1, 0))
end

function draw()
	Graphics.resetGraphicsPipeline()

	Graphics.beginRenderTarget(renderTexture)
	Graphics.drawModel(model, transform)
	Graphics.endRenderTarget()
	Graphics.readbackTexture(renderTexture, out)

	Graphics.drawImage(out, Rect.new(0, 0, width, height))
end
