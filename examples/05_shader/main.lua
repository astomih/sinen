local texture = sn.Texture.new()
local draw2d = sn.Draw2D.new()

local vertex_shader = sn.Shader.new()
vertex_shader:load("shader_custom.vert.spv", sn.ShaderStage.Vertex, 1)
local fragment_shader = sn.Shader.new()
fragment_shader:load("shaderAlpha.frag.spv", sn.ShaderStage.Fragment, 0)

local pipeline2d = sn.GraphicsPipeline.new()
pipeline2d:setVertexShader(vertex_shader)
pipeline2d:setFragmentShader(fragment_shader)
pipeline2d:setEnableDepthTest(false)
pipeline2d:build()
sn.Graphics.bindPipeline(pipeline2d)

local uniformBuffer = sn.Buffer.new({
    2.0,
    0.5,
    0.5,
    1.0
})

texture:load("logo.png")
draw2d.material:appendTexture(texture)
draw2d.scale = texture:size()

function Update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function Draw()
    sn.Graphics.setUniformBuffer(1, uniformBuffer)
    -- Draw texture
    sn.Graphics.draw2D(draw2d)
end
