local sn = require("@sinen")
local model: sn.Model = sn.Model.new()
model:load("DamagedHelmet.glb")
local transform: sn.Transform = sn.Transform.new()
local lightTransform: sn.Transform = sn.Transform.new()
local vertexShader: sn.Shader = sn.Shader.new()
local fragmentShader: sn.Shader = sn.Shader.new()
local pipeline3D: sn.GraphicsPipeline = sn.GraphicsPipeline.new()
local lightPos: sn.Vec3 = sn.Vec3.new(2, 0, 0)
local lightIntensity: number = 2.5
local uniformBuffer: { any } = {}

function setup()
	transform.position = sn.Vec3.new(0, 0, 0)
	transform.rotation = sn.Vec3.new(90, 0, 0)
	transform.scale = sn.Vec3.new(1, 1, 1)

	lightTransform.position = sn.Vec3.new(0)
	lightTransform.rotation = sn.Vec3.new(90, 0, 0)
	lightTransform.scale = sn.Vec3.new(1)

	local pos = sn.Vec3.new(0.7, 0.7, 2.1)
	local at = sn.Vec3.new(0)
	local up = sn.Vec3.new(0, 1, 0)
	sn.Graphics.getCamera3D():lookat(pos, at, up)

	vertexShader:compileAndLoad("shader_custom.slang", sn.ShaderStage.Vertex)
	fragmentShader:compileAndLoad("shader_custom.slang", sn.ShaderStage.Fragment)
	pipeline3D:setVertexShader(vertexShader)
	pipeline3D:setFragmentShader(fragmentShader)
	pipeline3D:setEnableTangent(true)
	pipeline3D:setEnableDepthTest(true)
	pipeline3D:build()

	uniformBuffer[1] = pos
	uniformBuffer[2] = lightPos
	uniformBuffer[3] = lightIntensity
end

function update()
	if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
		sn.Script.load("main", ".")
	end
	transform.rotation.z = transform.rotation.z + sn.Time.delta() * 10
	if sn.Keyboard.isDown(sn.Keyboard.LEFT) then
		lightPos.x = lightPos.x - sn.Time.delta() * 5
	end
	if sn.Keyboard.isDown(sn.Keyboard.RIGHT) then
		lightPos.x = lightPos.x + sn.Time.delta() * 5
	end
	if sn.Keyboard.isDown(sn.Keyboard.UP) then
		if sn.Keyboard.isDown(sn.Keyboard.LSHIFT) then
			lightPos.z = lightPos.z + sn.Time.delta() * 5
		else
			lightPos.y = lightPos.y + sn.Time.delta() * 5
		end
	end
	if sn.Keyboard.isDown(sn.Keyboard.DOWN) then
		if sn.Keyboard.isDown(sn.Keyboard.LSHIFT) then
			lightPos.z = lightPos.z - sn.Time.delta() * 5
		else
			lightPos.y = lightPos.y - sn.Time.delta() * 5
		end
	end
	uniformBuffer[2] = lightPos
	lightTransform.position = lightPos
end

function draw()
	sn.Graphics.setGraphicsPipeline(pipeline3D)

	sn.Graphics.setTexture(1, model:getTexture(sn.TextureKey.Normal))
	sn.Graphics.setTexture(2, model:getTexture(sn.TextureKey.DiffuseRoughness))
	sn.Graphics.setTexture(3, model:getTexture(sn.TextureKey.Metalness))
	sn.Graphics.setTexture(4, model:getTexture(sn.TextureKey.Emissive))
	sn.Graphics.setTexture(5, model:getTexture(sn.TextureKey.LightMap))

	sn.Graphics.setUniformBuffer(1, sn.Buffer.new(uniformBuffer))
	sn.Graphics.drawModel(model, transform)
	sn.Graphics.drawModel(model, lightTransform)
end
