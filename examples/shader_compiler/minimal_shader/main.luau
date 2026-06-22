local sn = require("@sinen")
local texture: sn.Texture = sn.Texture.new("logo.png")

local vertexShader: sn.Shader = sn.Shader.compileAndLoad("shader_custom.slang", sn.ShaderStage.Vertex)
local fragmentShader: sn.Shader = sn.Shader.compileAndLoad("shader_custom.slang", sn.ShaderStage.Fragment)

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
