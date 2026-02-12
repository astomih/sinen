local texture = Texture.new()
texture:load("logo.png")

local vertex_shader = Shader.new()
vertex_shader:compileAndLoad("shader_custom.slang", ShaderStage.Vertex)
local fragment_shader = Shader.new()
fragment_shader:compileAndLoad("shader_custom.slang", ShaderStage.Fragment)

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
