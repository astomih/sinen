local texture = sn.Texture.new()
local draw2d = sn.Draw2D.new()

local vertex_shader = sn.Shader.new()
vertex_shader:loadVertexShader("shader_custom.vert.spv", 1)
local fragment_shader = sn.Shader.new()
fragment_shader:loadFragmentShader("shaderAlpha.frag.spv", 0)

local pipeline2d = sn.GraphicsPipeline.new()
pipeline2d:setVertexShader(vertex_shader)
pipeline2d:setFragmentShader(fragment_shader)
pipeline2d:setEnableDepthTest(false)
pipeline2d:build()
sn.Graphics.bindPipeline(pipeline2d)

local uniform_data = sn.UniformData.new()
uniform_data:add(2.0)
uniform_data:add(0.5)
uniform_data:add(0.5)
uniform_data:add(1.0)

texture:load("logo.png")
draw2d.material:appendTexture(texture)
draw2d.scale = texture:size()

function Update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function Draw()
    sn.Graphics.setUniformData(1, uniform_data)
    -- Draw texture
    sn.Graphics.draw2D(draw2d)
end
