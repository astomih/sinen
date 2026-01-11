local model
local material
function Setup()
    model = sn.Model.new()
    model:load("BrainStem.glb")
    local texture = sn.Texture.new()
    texture:fill(sn.Color.new(1, 1, 1, 1))

    material = model:getMaterial()
    material:appendTexture(texture)

    local vs = sn.Shader.new()
    local fs = sn.Shader.new()

    local pipeline = sn.GraphicsPipeline.new()
    vs:compileAndLoad("skinning.slang", sn.ShaderStage.Vertex)
    fs:compileAndLoad("skinning.slang", sn.ShaderStage.Fragment)

    pipeline:setVertexShader(vs)
    pipeline:setFragmentShader(fs)
    pipeline:setEnableDepthTest(true)
    pipeline:setEnableAnimation(true)
    pipeline:build()
    material:setGraphicsPipeline(pipeline)

    sn.Graphics.getCamera():lookat(sn.Vec3.new(0, -3, 1), sn.Vec3.new(0, 0, 1), sn.Vec3.new(0, 0, 1))

    model:play(0)
end

function Update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
    model:update(sn.Time.delta())
    material:setUniformBuffer(1, model:getBoneUniformBuffer())
end

function Draw()
    -- Draw texture
    sn.Graphics.drawModel(model, sn.Transform.new(), material)
end
