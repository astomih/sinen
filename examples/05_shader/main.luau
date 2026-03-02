local sn = require("@sinen")
local texture: sn.Texture = sn.Texture.new()

local vertexShader: sn.Shader = sn.Shader.new()
vertexShader:load("shader_custom.vert.spv", sn.ShaderStage.Vertex, 1)
local fragmentShader: sn.Shader = sn.Shader.new()
fragmentShader:load("shaderAlpha.frag.spv", sn.ShaderStage.Fragment, 0)

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

texture:load("logo.png")

function update()
	if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
		sn.Script.load("main", ".")
	end
end

function draw()
	sn.Graphics.setGraphicsPipeline(pipeline2D)
	sn.Graphics.setUniformBuffer(1, uniformBuffer)
	-- Draw texture
	sn.Graphics.drawImage(texture, sn.Rect.new(sn.Vec2.new(0), texture:size()))
end
