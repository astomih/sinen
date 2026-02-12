local texture: Texture = Texture.new()

local vertexShader: Shader = Shader.new()
vertexShader:load("shader_custom.vert.spv", ShaderStage.Vertex, 1)
local fragmentShader: Shader = Shader.new()
fragmentShader:load("shaderAlpha.frag.spv", ShaderStage.Fragment, 0)

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

texture:load("logo.png")

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
