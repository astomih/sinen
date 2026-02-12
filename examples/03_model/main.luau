local model: Model = Model.new()
model:load("DamagedHelmet.glb")

local transform: Transform = Transform.new()
transform.position = Vec3.new(0, 0, 0)
transform.rotation = Vec3.new(90, 0, 0)
transform.scale = Vec3.new(1, 1, 1)

local pos: Vec3 = Vec3.new(1, 1, 3)
local at: Vec3 = Vec3.new(0)
local up: Vec3 = Vec3.new(0, 1, 0)
function setup()
	Graphics.getCamera3D():lookat(pos, at, up)
end

function update()
	if Keyboard.isPressed(Keyboard.ESCAPE) then
		Script.load("main", ".")
	end
end

function draw()
	Graphics.drawModel(model, transform)
end
