local model: Model = Model.new()
model:load("DamagedHelmet.glb")
local lightSphere: Model = Model.new()
lightSphere:load("sphere.glb")

local lut: Texture = Texture.new()
lut:load("citrus_orchard_road_puresky_2k/brdfLUT.png")

local cubemap: Texture = Texture.new()
cubemap:loadCubemap("citrus_orchard_road_puresky_2k/citrus_orchard_road_puresky_2k.exr")

local irradiance: Texture = Texture.new()
irradiance:loadCubemap("citrus_orchard_road_puresky_2k/irradiance.exr")

local prefiltered: Texture = Texture.new()
prefiltered:loadCubemap("citrus_orchard_road_puresky_2k/m1.exr")

local transform: Transform = Transform.new()
transform.position = Vec3.new(0, 0, 0)
transform.rotation = Vec3.new(90, 0, 0)
transform.scale = Vec3.new(1, 1, 1)

local light_transform: Transform = Transform.new()
light_transform.position = Vec3.new(0)
light_transform.rotation = Vec3.new(0, 0, 0)
light_transform.scale = Vec3.new(0.5)

local pos: Vec3 = Vec3.new(0.7, 0.7, 2.1)
local at: Vec3 = Vec3.new(0)
local up: Vec3 = Vec3.new(0, 1, 0)
Graphics.getCamera3D():lookat(pos, at, up)

local vertex_shader = Shader.new()
vertex_shader:compileAndLoad("shader_custom.slang", ShaderStage.Vertex)
local fragment_shader = Shader.new()
fragment_shader:compileAndLoad("shader_custom.slang", ShaderStage.Fragment)

local pipeline3d = GraphicsPipeline.new()
pipeline3d:setVertexShader(vertex_shader)
pipeline3d:setFragmentShader(fragment_shader)
pipeline3d:setEnableTangent(true)
pipeline3d:setEnableDepthTest(true)
pipeline3d:build()

local lightPos: Vec3 = Vec3.new(2, 0, 0)
local lightIntensity: number = 2.5

local uniformData: { any } = {
	pos,
	lightPos,
	lightIntensity,
}

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
	uniformData[2] = lightPos
	light_transform.position = lightPos
end

function draw()
	Graphics.setTexture(1, model:getTexture(TextureKey.Normal))
	Graphics.setTexture(2, model:getTexture(TextureKey.DiffuseRoughness))
	Graphics.setTexture(3, model:getTexture(TextureKey.Metalness))
	Graphics.setTexture(4, model:getTexture(TextureKey.Emissive))
	Graphics.setTexture(5, model:getTexture(TextureKey.LightMap))
	Graphics.setTexture(6, lut)
	Graphics.setTexture(7, cubemap)
	Graphics.setTexture(8, irradiance)
	Graphics.setTexture(9, prefiltered)

	Graphics.resetGraphicsPipeline()
	Graphics.drawCubemap(cubemap)

	Graphics.setGraphicsPipeline(pipeline3d)
	Graphics.setUniformBuffer(1, Buffer.new(uniformData))
	Graphics.drawModel(model, transform)
	Graphics.drawModel(lightSphere, light_transform)
end
