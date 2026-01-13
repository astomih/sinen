--TODO: depth texture as D24 or D32, PCF, etc...

local model = sn.Model.new()
local modelTransform = sn.Transform.new()
local floor = sn.Model.new()
local floorTransform = sn.Transform.new()
local shadowCamera = sn.Camera.new()
local depthVS = sn.Shader.new()
local depthFS = sn.Shader.new()
local depthPipeline = sn.GraphicsPipeline.new()
local depthRenderTexture = sn.RenderTexture.new()
local depthTexture = sn.Texture.new(sn.Window.size().x, sn.Window.size().y)
local vs = sn.Shader.new()
local fs = sn.Shader.new()
local pipeline = sn.GraphicsPipeline.new()
---@type sn.Buffer
local uniformBuffer
local floorTexture = sn.Texture.new()
local camera = sn.Camera.new()

function Setup()
    model:load("DamagedHelmet.glb")
    modelTransform.position = sn.Vec3.new(0, 0, 0)
    modelTransform.rotation = sn.Vec3.new(90, 0, 0)
    modelTransform.scale = sn.Vec3.new(1, 1, 1)
    floor:loadSprite()
    floorTransform.rotation.x = 90
    floorTransform.position.y = -1.0

    shadowCamera:orthographic(20, 20, -10, 10.0)
    shadowCamera:lookat(sn.Vec3.new(0.5, 2, 2), sn.Vec3.new(0), sn.Vec3.new(0, 1, 0))

    depthVS:compileAndLoad("depth_write.slang", sn.ShaderStage.Vertex)
    depthFS:compileAndLoad("depth_write.slang", sn.ShaderStage.Fragment)

    depthPipeline:setEnableDepthTest(true)
    depthPipeline:setVertexShader(depthVS)
    depthPipeline:setFragmentShader(depthFS)
    depthPipeline:build()

    depthRenderTexture:create(sn.Window.size().x, sn.Window.size().y)
    depthTexture:fill(sn.Color.new(1, 1, 1, 1))


    vs:compileAndLoad("shader.slang", sn.ShaderStage.Vertex)
    fs:compileAndLoad("shader.slang", sn.ShaderStage.Fragment)
    pipeline:setEnableDepthTest(true)
    pipeline:setVertexShader(vs)
    pipeline:setFragmentShader(fs)
    pipeline:build()

    uniformBuffer = sn.Buffer.new({ shadowCamera })

    floorTexture:fill(sn.Color.new(1))

    camera:perspective(70, 16.0 / 9.0, 0.1, 100)
    camera:lookat(sn.Vec3.new(0, 1, 5), sn.Vec3.new(0), sn.Vec3.new(0, 1, 0))
    floor:setTexture(sn.TextureKey.BaseColor, floorTexture)
end

function Update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function Draw()
    -- depth write pipeline
    sn.Graphics.setRenderTarget(depthRenderTexture)
    sn.Graphics.setGraphicsPipeline(depthPipeline)
    sn.Graphics.setCamera(shadowCamera)
    sn.Graphics.drawModel(model, modelTransform)
    sn.Graphics.drawModel(floor, floorTransform)
    sn.Graphics.flush()
    sn.Graphics.readbackTexture(depthRenderTexture, depthTexture)

    -- Lighting
    sn.Graphics.setGraphicsPipeline(pipeline)
    sn.Graphics.setUniformBuffer(1, uniformBuffer)
    sn.Graphics.setTexture(1, depthTexture)
    sn.Graphics.setCamera(camera)
    sn.Graphics.drawModel(model, modelTransform)
    sn.Graphics.drawModel(floor, floorTransform)
end
