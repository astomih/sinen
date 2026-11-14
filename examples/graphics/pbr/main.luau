local sn = require("@sinen")

local function createLightModel(): sn.Model
	local builder = sn.MeshBuilder.new()
	builder:addSphere(1.0, 24, 48, sn.Color.new(1.0))
	return builder:toModel()
end

local model: sn.Model = sn.Model.new("DamagedHelmet.glb")
local lightModel: sn.Model = createLightModel()
local transform: sn.Transform = sn.Transform.new()
local lightTransform: sn.Transform = sn.Transform.new()
local renderer = sn.PBRRenderer.new()
local light = sn.PointLight.new(sn.Vec3.new(1.05, 0.8, 0.75), 4.0)
local camera: sn.Camera3D = sn.Camera3D.new()

function setup()
	transform.position = sn.Vec3.new(0, 0, 0)
	transform.rotation = sn.Vec3.new(90, 0, 0)
	transform.scale = sn.Vec3.new(1, 1, 1)

	lightTransform.position = light:getPosition()
	lightTransform.rotation = sn.Vec3.new(0, 0, 0)
	lightTransform.scale = sn.Vec3.new(0.12)

	local pos = sn.Vec3.new(0.7, 0.7, 2.1)
	local at = sn.Vec3.new(0)
	local up = sn.Vec3.new(0, 1, 0)
	camera:lookat(pos, at, up)

	renderer:setLight(light)
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
	transform.rotation.z = transform.rotation.z + sn.Time.delta() * 10
	local lightPos = light:getPosition()
	if sn.Keyboard.isDown(sn.Scancode.LEFT) then
		lightPos.x = lightPos.x - sn.Time.delta() * 5
	end
	if sn.Keyboard.isDown(sn.Scancode.RIGHT) then
		lightPos.x = lightPos.x + sn.Time.delta() * 5
	end
	if sn.Keyboard.isDown(sn.Scancode.UP) then
		if sn.Keyboard.isDown(sn.Scancode.LSHIFT) then
			lightPos.z = lightPos.z + sn.Time.delta() * 5
		else
			lightPos.y = lightPos.y + sn.Time.delta() * 5
		end
	end
	if sn.Keyboard.isDown(sn.Scancode.DOWN) then
		if sn.Keyboard.isDown(sn.Scancode.LSHIFT) then
			lightPos.z = lightPos.z - sn.Time.delta() * 5
		else
			lightPos.y = lightPos.y - sn.Time.delta() * 5
		end
	end
	light:setPosition(lightPos)
	renderer:setLight(light)
	lightTransform.position = lightPos
end

function draw()
	local pass = sn.Graphics.begin3DPass(camera)
	renderer:drawModel(pass, model, transform)

	pass:drawModel(lightModel, lightTransform)
	sn.Graphics.endPass(pass)
end
