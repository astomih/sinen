local texture = Texture.new()

local vertex_shader = Shader.new()
vertex_shader:load("shader_custom.vert.spv", ShaderStage.Vertex, 1)
local fragment_shader = Shader.new()
fragment_shader:load("shaderAlpha.frag.spv", ShaderStage.Fragment, 0)

local pipeline2d = GraphicsPipeline.new()
pipeline2d:setVertexShader(vertex_shader)
pipeline2d:setFragmentShader(fragment_shader)
pipeline2d:setEnableDepthTest(false)
pipeline2d:build()

local uniformBuffer = Buffer.new({
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
	Graphics.setGraphicsPipeline(pipeline2d)
	Graphics.setUniformBuffer(1, uniformBuffer)
	-- Draw texture
	Graphics.drawImage(texture, Rect.new(Vec2.new(0), texture:size()))
end
