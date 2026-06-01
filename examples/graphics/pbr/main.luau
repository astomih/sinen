local sn = require("@sinen")

local function createLightModel(): sn.Model
	local builder = sn.MeshBuilder.new()
	builder:addSphere(1.0, 24, 48, sn.Color.new(1.0, 0.98, 0.78, 1.0))
	return builder:toModel()
end

local function createLightGlowModel(): sn.Model
	local builder = sn.MeshBuilder.new()
	builder:addSphere(1.0, 24, 48, sn.Color.new(1.0, 0.78, 0.22, 0.42))
	return builder:toModel()
end

local function createGlowTexture(size: number): sn.Texture
	local pixels = {}
	local center = (size - 1) * 0.5
	local index = 1
	for y = 0, size - 1 do
		for x = 0, size - 1 do
			local dx = (x - center) / center
			local dy = (y - center) / center
			local distance = math.sqrt(dx * dx + dy * dy)
			local falloff = math.max(0, 1 - distance)
			local alpha = math.floor((falloff * falloff * falloff) * 180)
			pixels[index] = 255
			pixels[index + 1] = 220
			pixels[index + 2] = 92
			pixels[index + 3] = alpha
			index += 4
		end
	end

	local texture = sn.Texture.new()
	texture:loadPixels(sn.Buffer.fromBytes(pixels), size, size)
	return texture
end

local model: sn.Model = sn.Model.new()
model:load("DamagedHelmet.glb")
local lightModel: sn.Model = createLightModel()
local lightGlowModel: sn.Model = createLightGlowModel()
local glowTexture: sn.Texture = createGlowTexture(96)
local transform: sn.Transform = sn.Transform.new()
local lightTransform: sn.Transform = sn.Transform.new()
local lightGlowTransform: sn.Transform = sn.Transform.new()
local vertexShader: sn.Shader = sn.Shader.new()
local fragmentShader: sn.Shader = sn.Shader.new()
local pipeline3D: sn.GraphicsPipeline = sn.GraphicsPipeline.new()
local lightGlowPipeline: sn.GraphicsPipeline = sn.GraphicsPipeline.new()
local lightPos: sn.Vec3 = sn.Vec3.new(1.6, 0.85, 1.1)
local lightIntensity: number = 4.0
local uniformBuffer: { any } = {}
local camera: sn.Camera3D = sn.Camera3D.new()

local function setWorldUniform(t: sn.Transform)
	sn.Graphics.setUniformBuffer(1, sn.Buffer.new({ t:getWorldMatrix() }))
end

local function drawLightBloom()
	local screen = camera:worldToScreen(lightPos)

	sn.Graphics.begin2D()
	sn.Graphics.resetAllTexture()
	sn.Graphics.resetGraphicsPipeline()
	local glowSize = 170
	sn.Graphics.drawImage(
		glowTexture,
		sn.Rect.new(sn.Pivot.Center, sn.Vec2.new(screen.x, screen.y), sn.Vec2.new(glowSize, glowSize))
	)
end

function setup()
	transform.position = sn.Vec3.new(0, 0, 0)
	transform.rotation = sn.Vec3.new(90, 0, 0)
	transform.scale = sn.Vec3.new(1, 1, 1)

	lightTransform.position = lightPos
	lightTransform.rotation = sn.Vec3.new(0, 0, 0)
	lightTransform.scale = sn.Vec3.new(0.12)
	lightGlowTransform.position = lightPos
	lightGlowTransform.rotation = sn.Vec3.new(0, 0, 0)
	lightGlowTransform.scale = sn.Vec3.new(0.24)

	local pos = sn.Vec3.new(0.7, 0.7, 2.1)
	local at = sn.Vec3.new(0)
	local up = sn.Vec3.new(0, 1, 0)
	camera:lookat(pos, at, up)

	vertexShader:compileAndLoad("shader_custom.slang", sn.ShaderStage.Vertex)
	fragmentShader:compileAndLoad("shader_custom.slang", sn.ShaderStage.Fragment)
	pipeline3D:setVertexShader(vertexShader)
	pipeline3D:setFragmentShader(fragmentShader)
	pipeline3D:setEnableTangent(true)
	pipeline3D:setEnableDepthTest(true)
	pipeline3D:build()

	lightGlowPipeline:setVertexShader(sn.BuiltinShader.getDefaultVS())
	lightGlowPipeline:setFragmentShader(sn.BuiltinShader.getDefaultFS())
	lightGlowPipeline:setEnableDepthTest(true)
	lightGlowPipeline:setEnableDepthWrite(false)
	lightGlowPipeline:build()

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
	lightGlowTransform.position = lightPos
end

function draw()
	sn.Graphics.begin3D(camera)
	sn.Graphics.setGraphicsPipeline(pipeline3D)

	sn.Graphics.setTexture(1, model:getTexture(sn.TextureKey.Normal))
	sn.Graphics.setTexture(2, model:getTexture(sn.TextureKey.DiffuseRoughness))
	sn.Graphics.setTexture(3, model:getTexture(sn.TextureKey.Emissive))
	sn.Graphics.setTexture(4, model:getTexture(sn.TextureKey.LightMap))

	sn.Graphics.setUniformBuffer(0, sn.Buffer.new(uniformBuffer))
	setWorldUniform(transform)
	sn.Graphics.drawModel(model, transform)

	sn.Graphics.resetAllTexture()
	sn.Graphics.setGraphicsPipeline(lightGlowPipeline)
	sn.Graphics.drawModel(lightGlowModel, lightGlowTransform)
	sn.Graphics.resetGraphicsPipeline()
	sn.Graphics.drawModel(lightModel, lightTransform)
	drawLightBloom()
	sn.Graphics.finish()
end
