local sn = require("@sinen")
local texture: sn.Texture = sn.Texture.new()
texture:load("logo.png")

local shaderSource = "shader_custom.slang"
local vertexShaderBundle = "shader_custom.vert.snb"
local fragmentShaderBundle = "shader_custom.frag.snb"

local basePath = "graphics/06_shader_compiled/"

local function compileShader(source: string, stage: any, format: any): sn.Buffer
	local shader = sn.Shader.new()
	shader:compile(source, stage, format)
	return shader:getCode()
end

local function compileShaderBundle(path: string, stage: any, numSamplers: number, numUniformBuffers: number)
	local bundle = sn.ShaderBundle.pack({
		{
			stage = stage,
			format = sn.ShaderFormat.SPIRV,
			code = compileShader(shaderSource, stage, sn.ShaderFormat.SPIRV),
			numSamplers = numSamplers,
			numStorageBuffers = 0,
			numStorageTextures = 0,
			numUniformBuffers = numUniformBuffers,
		},
		{
			stage = stage,
			format = sn.ShaderFormat.SPIRV_1_3,
			code = compileShader(shaderSource, stage, sn.ShaderFormat.SPIRV_1_3),
			numSamplers = numSamplers,
			numStorageBuffers = 0,
			numStorageTextures = 0,
			numUniformBuffers = numUniformBuffers,
		},
		{
			stage = stage,
			format = sn.ShaderFormat.WGSL,
			code = compileShader(shaderSource, stage, sn.ShaderFormat.WGSL),
			numSamplers = numSamplers,
			numStorageBuffers = 0,
			numStorageTextures = 0,
			numUniformBuffers = numUniformBuffers,
		},
		{
			stage = stage,
			format = sn.ShaderFormat.DXIL,
			code = compileShader(shaderSource, stage, sn.ShaderFormat.DXIL),
			numSamplers = numSamplers,
			numStorageBuffers = 0,
			numStorageTextures = 0,
			numUniformBuffers = numUniformBuffers,
		},
	})

	if not sn.Filesystem.write(basePath .. path, bundle) then
		error("Failed to write shader bundle: " .. path)
	end
end

if not sn.Filesystem.exists(basePath .. vertexShaderBundle) then
	compileShaderBundle(vertexShaderBundle, sn.ShaderStage.Vertex, 0, 1)
end

if not sn.Filesystem.exists(basePath .. fragmentShaderBundle) then
	compileShaderBundle(fragmentShaderBundle, sn.ShaderStage.Fragment, 1, 1)
end

local vertexShader: sn.Shader = sn.Shader.new()
vertexShader:load(vertexShaderBundle, sn.ShaderStage.Vertex, 0)
local fragmentShader: sn.Shader = sn.Shader.new()
fragmentShader:load(fragmentShaderBundle, sn.ShaderStage.Fragment, 0)

local pipeline2D: sn.GraphicsPipeline = sn.GraphicsPipeline.new()
pipeline2D:setVertexShader(vertexShader)
pipeline2D:setFragmentShader(fragmentShader)
pipeline2D:setEnableDepthTest(false)
pipeline2D:build()

local uniformBuffer: sn.Buffer = sn.Buffer.new({
	2.0,
	0.5,
	0.5,
	1.0,
})

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
end

function draw()
	sn.Graphics.setGraphicsPipeline(pipeline2D)
	sn.Graphics.setUniformBuffer(1, uniformBuffer)
	-- Draw texture
	sn.Graphics.drawImage(texture, sn.Rect.new(sn.Pivot.Center, sn.Window.center(), texture:size()))
end
