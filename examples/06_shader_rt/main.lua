local texture = sn.Texture.new()
texture:load("logo.png")

local vertex_shader = sn.Shader.new()
vertex_shader:compileAndLoad("shader_custom.slang", sn.ShaderStage.Vertex)
local fragment_shader = sn.Shader.new()
fragment_shader:compileAndLoad("shader_custom.slang", sn.ShaderStage.Fragment)

local pipeline2d = sn.GraphicsPipeline.new()
pipeline2d:setVertexShader(vertex_shader)
pipeline2d:setFragmentShader(fragment_shader)
pipeline2d:setEnableDepthTest(false)
pipeline2d:build()

local uniformBuffer = sn.Buffer.new({
    2.0, 0.5, 0.5, 1.0
})

function update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function draw()
    sn.Graphics.setGraphicsPipeline(pipeline2d)
    sn.Graphics.setUniformBuffer(1, uniformBuffer)
    -- Draw texture
    sn.Graphics.drawImage(texture, sn.Rect.new(sn.Vec2.new(0), texture:size()))
end
