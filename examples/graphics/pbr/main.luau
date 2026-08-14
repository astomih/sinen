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
local vertexShader: sn.Shader = sn.Shader.new("shader_custom.vert.snb", sn.ShaderStage.Vertex)
local fragmentShader: sn.Shader = sn.Shader.new("shader_custom.frag.snb", sn.ShaderStage.Fragment)
local pipeline3D: sn.GraphicsPipeline = sn.GraphicsPipeline.new()
local material: sn.Material
local lightPos: sn.Vec3 = sn.Vec3.new(1.05, 0.8, 0.75)
local lightIntensity: number = 4.0
local uniformBuffer: { any } = {}
local camera: sn.Camera3D = sn.Camera3D.new()

local function setWorldUniform(t: sn.Transform)
	material:setUniformBuffer(1, sn.Buffer.new({ t:getWorldMatrix() }))
end

function setup()
	transform.position = sn.Vec3.new(0, 0, 0)
	transform.rotation = sn.Vec3.new(90, 0, 0)
	transform.scale = sn.Vec3.new(1, 1, 1)

	lightTransform.position = lightPos
	lightTransform.rotation = sn.Vec3.new(0, 0, 0)
	lightTransform.scale = sn.Vec3.new(0.12)

	local pos = sn.Vec3.new(0.7, 0.7, 2.1)
	local at = sn.Vec3.new(0)
	local up = sn.Vec3.new(0, 1, 0)
	camera:lookat(pos, at, up)

	pipeline3D:setVertexShader(vertexShader)
	pipeline3D:setFragmentShader(fragmentShader)
	pipeline3D:setEnableTangent(true)
	pipeline3D:setEnableDepthTest(true)
	pipeline3D:build()
	material = sn.Material.new(pipeline3D)
	material:setTexture(1, model:getTexture(sn.TextureKey.Normal))
	material:setTexture(2, model:getTexture(sn.TextureKey.DiffuseRoughness))
	material:setTexture(3, model:getTexture(sn.TextureKey.Emissive))
	material:setTexture(4, model:getTexture(sn.TextureKey.LightMap))

	uniformBuffer[1] = pos
	uniformBuffer[2] = lightPos
	uniformBuffer[3] = lightIntensity
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
	transform.rotation.z = transform.rotation.z + sn.Time.delta() * 10
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
	uniformBuffer[2] = lightPos
	lightTransform.position = lightPos
end

function draw()
	local pass = sn.Graphics.begin3DPass(camera)
	material:setUniformBuffer(0, sn.Buffer.new(uniformBuffer))
	setWorldUniform(transform)
	pass:drawModel(model, transform, material)

	pass:drawModel(lightModel, lightTransform)
	sn.Graphics.endPass(pass)
end
