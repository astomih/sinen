local texture = sn.Texture.new()
texture:fill(sn.Color.new(1, 1, 1, 1))
local model = sn.Model.new()
model:load("BrainStem.glb")
local draw3d = sn.Draw3D.new(texture)
draw3d.position = sn.Vec3.new(0, 0, 0)
draw3d.model = model
draw3d.material = model:getMaterial()

local vertex_shader = sn.Shader.new()
vertex_shader:compileLoadVertexShader("skinning.slang")
local fragment_shader = sn.Shader.new()
fragment_shader:compileLoadFragmentShader("skinning.slang")

local pipeline = sn.GraphicsPipeline.new()
pipeline:setVertexShader(vertex_shader)
pipeline:setFragmentShader(fragment_shader)
pipeline:setEnableDepthTest(true)
pipeline:setEnableAnimation(true)
pipeline:build()
sn.Graphics.bindPipeline(pipeline)


sn.Graphics.getCamera():lookat(sn.Vec3.new(0, -3, 1), sn.Vec3.new(0, 0, 1), sn.Vec3.new(0, 0, 1))

model:play(0)
function update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
    model:update(sn.Time.delta())
end

function draw()
    sn.Graphics.setUniformData(1, model:getBoneUniformData())
    -- Draw texture
    sn.Graphics.drawModel(model, sn.Transform.new(), draw3d.material)
end
