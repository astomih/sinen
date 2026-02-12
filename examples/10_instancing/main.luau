local model: Model = Model.new()
model:load("DamagedHelmet.glb")
local transforms: { Transform } = {}
function setup()
	for i = 0, 10 do
		for j = 0, 10 do
			local t = Transform.new()
			t.position = Vec3.new(i * 2, 0, j * 2)
			t.rotation = Vec3.new(90, 0, 180)
			table.insert(transforms, t)
		end
	end

	local pos: Vec3 = Vec3.new(-3, 5, -3)
	local at: Vec3 = Vec3.new(15, -5, 15)
	local up: Vec3 = Vec3.new(0, 1, 0)
	Graphics.getCamera3D():lookat(pos, at, up)
end

function update()
	if Keyboard.isPressed(Keyboard.ESCAPE) then
		Script.load("main", ".")
	end
end

function draw()
	Graphics.drawModelInstanced(model, transforms)
end
