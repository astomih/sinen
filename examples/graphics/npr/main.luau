local sn = require("@sinen")

local builder = sn.MeshBuilder.new()
builder:addSphere(1.0, 32, 64, sn.Color.new(0.35, 0.75, 0.95, 1.0))
local model = builder:toModel()
local transform = sn.Transform.new()

local camera = sn.Camera3D.new()
camera:lookat(sn.Vec3.new(0, 0.5, 3), sn.Vec3.new(0), sn.Vec3.new(0, 1, 0))

local renderer = sn.NPRRenderer.new()
renderer:setShadowColor(sn.Color.new(0.2, 0.25, 0.55, 1.0))
renderer:setBands(4)
local angle = 0.0

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
	angle += sn.Time.delta()
	renderer:setLightDirection(sn.Vec3.new(math.cos(angle), 0.6, math.sin(angle)))
end

function draw()
	local pass = sn.Graphics.begin3DPass(camera)
	renderer:drawModel(pass, model, transform)
	sn.Graphics.endPass(pass)
end
