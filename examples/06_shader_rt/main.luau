local texture: Texture = Texture.new()
texture:load("logo.png")

local vertexShader: Shader = Shader.new()
vertexShader:compileAndLoad("shader_custom.slang", ShaderStage.Vertex)
local fragmentShader: Shader = Shader.new()
fragmentShader:compileAndLoad("shader_custom.slang", ShaderStage.Fragment)

local pipeline2D: GraphicsPipeline = GraphicsPipeline.new()
pipeline2D:setVertexShader(vertexShader)
pipeline2D:setFragmentShader(fragmentShader)
pipeline2D:setEnableDepthTest(false)
pipeline2D:build()

local uniformBuffer: Buffer = Buffer.new({
	2.0,
	0.5,
	0.5,
	1.0,
})

function update()
	if Keyboard.isPressed(Keyboard.ESCAPE) then
		Script.load("main", ".")
	end
end

function draw()
	Graphics.setGraphicsPipeline(pipeline2D)
	Graphics.setUniformBuffer(1, uniformBuffer)
	-- Draw texture
	Graphics.drawImage(texture, Rect.new(Vec2.new(0), texture:size()))
end
