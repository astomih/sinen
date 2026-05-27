local sn = require("@sinen")
local model: sn.Model = sn.Model.new()
model:load("DamagedHelmet.glb")

local transform: sn.Transform = sn.Transform.new()
transform.position = sn.Vec3.new(0, 0, 0)
transform.rotation = sn.Vec3.new(90, 0, 0)
transform.scale = sn.Vec3.new(1, 1, 1)

local pos: sn.Vec3 = sn.Vec3.new(1, 1, 3)
local at: sn.Vec3 = sn.Vec3.new(0)
local up: sn.Vec3 = sn.Vec3.new(0, 1, 0)
local camera: sn.Camera3D = sn.Camera3D.new()
function setup()
	camera:lookat(pos, at, up)
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
end

function draw()
	sn.Graphics.begin3D(camera)
	sn.Graphics.drawModel(model, transform)
	sn.Graphics.finish()
end
