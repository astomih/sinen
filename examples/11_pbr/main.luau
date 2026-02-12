--!nolint FunctionUnused
local model: Model = Model.new()
model:load("DamagedHelmet.glb")
local transform: Transform = Transform.new()
local lightTransform: Transform = Transform.new()
local vertexShader: Shader = Shader.new()
local fragmentShader: Shader = Shader.new()
local pipeline3D: GraphicsPipeline = GraphicsPipeline.new()
local lightPos: Vec3 = Vec3.new(2, 0, 0)
local lightIntensity: number = 2.5
local uniformBuffer: { any } = {}

function setup()
	transform.position = Vec3.new(0, 0, 0)
	transform.rotation = Vec3.new(90, 0, 0)
	transform.scale = Vec3.new(1, 1, 1)

	lightTransform.position = Vec3.new(0)
	lightTransform.rotation = Vec3.new(90, 0, 0)
	lightTransform.scale = Vec3.new(1)

	local pos = Vec3.new(0.7, 0.7, 2.1)
	local at = Vec3.new(0)
	local up = Vec3.new(0, 1, 0)
	Graphics.getCamera3D():lookat(pos, at, up)

	vertexShader:compileAndLoad("shader_custom.slang", ShaderStage.Vertex)
	fragmentShader:compileAndLoad("shader_custom.slang", ShaderStage.Fragment)
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
	if Keyboard.isPressed(Keyboard.ESCAPE) then
		Script.load("main", ".")
	end
	transform.rotation.z = transform.rotation.z + Time.delta() * 10
	if Keyboard.isDown(Keyboard.LEFT) then
		lightPos.x = lightPos.x - Time.delta() * 5
	end
	if Keyboard.isDown(Keyboard.RIGHT) then
		lightPos.x = lightPos.x + Time.delta() * 5
	end
	if Keyboard.isDown(Keyboard.UP) then
		if Keyboard.isDown(Keyboard.LSHIFT) then
			lightPos.z = lightPos.z + Time.delta() * 5
		else
			lightPos.y = lightPos.y + Time.delta() * 5
		end
	end
	if Keyboard.isDown(Keyboard.DOWN) then
		if Keyboard.isDown(Keyboard.LSHIFT) then
			lightPos.z = lightPos.z - Time.delta() * 5
		else
			lightPos.y = lightPos.y - Time.delta() * 5
		end
	end
	uniformBuffer[2] = lightPos
	lightTransform.position = lightPos
end

function draw()
	Graphics.setGraphicsPipeline(pipeline3D)

	Graphics.setTexture(1, model:getTexture(TextureKey.Normal))
	Graphics.setTexture(2, model:getTexture(TextureKey.DiffuseRoughness))
	Graphics.setTexture(3, model:getTexture(TextureKey.Metalness))
	Graphics.setTexture(4, model:getTexture(TextureKey.Emissive))
	Graphics.setTexture(5, model:getTexture(TextureKey.LightMap))

	Graphics.setUniformBuffer(1, Buffer.new(uniformBuffer))
	Graphics.drawModel(model, transform)
	Graphics.drawModel(model, lightTransform)
end
