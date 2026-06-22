local sn = require("@sinen")

local function createLightModel(): sn.Model
	local builder = sn.MeshBuilder.new()
	builder:addSphere(1.0, 24, 48, sn.Color.new(1.0))
	return builder:toModel()
end

local model: sn.Model = sn.Model.new("DamagedHelmet.glb")
local lightModel: sn.Model = createLightModel()

local envPath = "citrus_orchard_road_puresky_2k/citrus_orchard_road_puresky_2k.exr"

function saveImageExr(name: string, texture: sn.Texture)
	local buffer: sn.Buffer = texture:toExrBuffer(true)
	sn.Filesystem.write(name, buffer)
end

function saveImage(name: string, texture: sn.Texture)
	local buffer: sn.Buffer = texture:toPngBuffer()
	sn.Filesystem.write(name, buffer)
end

local isExistsEXRs = false
if
	sn.Filesystem.exists("brdf_lut.png")
	and sn.Filesystem.exists("irradiance_cubemap.exr")
	and sn.Filesystem.exists("prefiltered_cubemap.exr")
then
	isExistsEXRs = true
end

local lut: sn.Texture
local cubemap: sn.Texture
local irradiance: sn.Texture
local prefiltered: sn.Texture

if isExistsEXRs then
	lut = sn.Texture.new("brdf_lut.png")
	cubemap = sn.Texture.newCubemap(envPath)
	irradiance = sn.Texture.newCubemap("irradiance_cubemap.exr")
	prefiltered = sn.Texture.newCubemap("prefiltered_cubemap.exr")
else
	lut = sn.Texture.newBRDFLUT(256, 512)
	cubemap = sn.Texture.newCubemap(envPath)
	irradiance = sn.Texture.newIrradianceCubemap(envPath, 64, 256)
	prefiltered = sn.Texture.newPrefilteredCubemap(envPath, 128, 5, 128)
end

local transform: sn.Transform = sn.Transform.new()
transform.position = sn.Vec3.new(0, 0, 0)
transform.rotation = sn.Vec3.new(90, 0, 0)
transform.scale = sn.Vec3.new(1, 1, 1)

local lightTransform: sn.Transform = sn.Transform.new()
lightTransform.rotation = sn.Vec3.new(0, 0, 0)
lightTransform.scale = sn.Vec3.new(0.12)

local pos: sn.Vec3 = sn.Vec3.new(0.7, 0.7, 2.1)
local at: sn.Vec3 = sn.Vec3.new(0)
local up: sn.Vec3 = sn.Vec3.new(0, 1, 0)
local camera: sn.Camera3D = sn.Camera3D.new()
camera:lookat(pos, at, up)

local vertex_shader = sn.Shader.new("shader_custom.vert.snb", sn.ShaderStage.Vertex)
local fragment_shader = sn.Shader.new("shader_custom.frag.snb", sn.ShaderStage.Fragment)

local pipeline3d = sn.GraphicsPipeline.new()
pipeline3d:setVertexShader(vertex_shader)
pipeline3d:setFragmentShader(fragment_shader)
pipeline3d:setEnableTangent(true)
pipeline3d:setEnableDepthTest(true)
pipeline3d:build()

local lightPos: sn.Vec3 = sn.Vec3.new(1.05, 0.8, 0.75)
local lightIntensity: number = 4.0
lightTransform.position = lightPos

local uniformData: { any } = {
	pos,
	lightPos,
	lightIntensity,
}

local function setWorldUniform(t: sn.Transform)
	sn.Graphics.setUniformBuffer(1, sn.Buffer.new({ t:getWorldMatrix() }))
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.P) then
		saveImage("brdf_lut.png", lut)
		saveImageExr("irradiance_cubemap.exr", irradiance)
		saveImageExr("prefiltered_cubemap.exr", prefiltered)
	end
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
	uniformData[2] = lightPos
	lightTransform.position = lightPos
end

function draw()
	sn.Graphics.begin3D(camera)
	sn.Graphics.resetAllTexture()
	sn.Graphics.resetGraphicsPipeline()
	sn.Graphics.drawCubemap(cubemap)

	sn.Graphics.resetAllTexture()
	sn.Graphics.setTexture(1, model:getTexture(sn.TextureKey.Normal))
	sn.Graphics.setTexture(2, model:getTexture(sn.TextureKey.DiffuseRoughness))
	sn.Graphics.setTexture(3, model:getTexture(sn.TextureKey.Emissive))
	sn.Graphics.setTexture(4, model:getTexture(sn.TextureKey.LightMap))
	sn.Graphics.setTexture(5, lut)
	sn.Graphics.setTexture(6, cubemap)
	sn.Graphics.setTexture(7, irradiance)
	sn.Graphics.setTexture(8, prefiltered)

	sn.Graphics.setGraphicsPipeline(pipeline3d)
	sn.Graphics.setUniformBuffer(0, sn.Buffer.new(uniformData))
	setWorldUniform(transform)
	sn.Graphics.drawModel(model, transform)

	sn.Graphics.resetAllTexture()
	sn.Graphics.resetGraphicsPipeline()
	sn.Graphics.drawModel(lightModel, lightTransform)
	sn.Graphics.finish()
end
