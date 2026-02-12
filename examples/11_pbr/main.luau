--!nolint FunctionUnused
local model = Model.new()
model:load("DamagedHelmet.glb")
local transform = Transform.new()
local light_transform = Transform.new()
local vertex_shader = Shader.new()
local fragment_shader = Shader.new()
local pipeline3d = GraphicsPipeline.new()
local light_pos = Vec3.new(2, 0, 0)
local light_intensity = 2.5
local uniform_data = {}

function setup()
	transform.position = Vec3.new(0, 0, 0)
	transform.rotation = Vec3.new(90, 0, 0)
	transform.scale = Vec3.new(1, 1, 1)

	light_transform.position = Vec3.new(0)
	light_transform.rotation = Vec3.new(90, 0, 0)
	light_transform.scale = Vec3.new(1)

	local pos = Vec3.new(0.7, 0.7, 2.1)
	local at = Vec3.new(0)
	local up = Vec3.new(0, 1, 0)
	Graphics.getCamera3D():lookat(pos, at, up)

	vertex_shader:compileAndLoad("shader_custom.slang", ShaderStage.Vertex)
	fragment_shader:compileAndLoad("shader_custom.slang", ShaderStage.Fragment)
	pipeline3d:setVertexShader(vertex_shader)
	pipeline3d:setFragmentShader(fragment_shader)
	pipeline3d:setEnableTangent(true)
	pipeline3d:setEnableDepthTest(true)
	pipeline3d:build()

	uniform_data[1] = pos
	uniform_data[2] = light_pos
	uniform_data[3] = light_intensity
end

function update()
	if Keyboard.isPressed(Keyboard.ESCAPE) then
		Script.load("main", ".")
	end
	transform.rotation.z = transform.rotation.z + Time.delta() * 10
	if Keyboard.isDown(Keyboard.LEFT) then
		light_pos.x = light_pos.x - Time.delta() * 5
	end
	if Keyboard.isDown(Keyboard.RIGHT) then
		light_pos.x = light_pos.x + Time.delta() * 5
	end
	if Keyboard.isDown(Keyboard.UP) then
		if Keyboard.isDown(Keyboard.LSHIFT) then
			light_pos.z = light_pos.z + Time.delta() * 5
		else
			light_pos.y = light_pos.y + Time.delta() * 5
		end
	end
	if Keyboard.isDown(Keyboard.DOWN) then
		if Keyboard.isDown(Keyboard.LSHIFT) then
			light_pos.z = light_pos.z - Time.delta() * 5
		else
			light_pos.y = light_pos.y - Time.delta() * 5
		end
	end
	uniform_data[2] = light_pos
	light_transform.position = light_pos
end

function draw()
	Graphics.setGraphicsPipeline(pipeline3d)

	Graphics.setTexture(1, model:getTexture(TextureKey.Normal))
	Graphics.setTexture(2, model:getTexture(TextureKey.DiffuseRoughness))
	Graphics.setTexture(3, model:getTexture(TextureKey.Metalness))
	Graphics.setTexture(4, model:getTexture(TextureKey.Emissive))
	Graphics.setTexture(5, model:getTexture(TextureKey.LightMap))

	Graphics.setUniformBuffer(1, Buffer.new(uniform_data))
	Graphics.drawModel(model, transform)
	Graphics.drawModel(model, light_transform)
end
