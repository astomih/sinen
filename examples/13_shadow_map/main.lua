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
local modelMaterial = sn.Material.new()
local floorMaterial = sn.Material.new()
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

    depthVS:compileLoadVertexShader("depth_write.slang")
    depthFS:compileLoadFragmentShader("depth_write.slang")

    depthPipeline:setEnableDepthTest(true)
    depthPipeline:setVertexShader(depthVS)
    depthPipeline:setFragmentShader(depthFS)
    depthPipeline:build()

    depthRenderTexture:create(sn.Window.size().x, sn.Window.size().y)
    depthTexture:fill(sn.Color.new(1, 1, 1, 1))


    vs:compileLoadVertexShader("shader.slang")
    fs:compileLoadFragmentShader("shader.slang")
    pipeline:setEnableDepthTest(true)
    pipeline:setVertexShader(vs)
    pipeline:setFragmentShader(fs)
    pipeline:build()

    uniformBuffer = sn.Buffer.new({ shadowCamera })

    modelMaterial:appendTexture(model:getMaterial():getTexture(0))
    modelMaterial:appendTexture(depthTexture)

    floorTexture:fill(sn.Color.new(1))
    floorMaterial:appendTexture(floorTexture)
    floorMaterial:appendTexture(depthTexture)

    camera:perspective(70, 16.0 / 9.0, 0.1, 100)
    camera:lookat(sn.Vec3.new(0, 1, 5), sn.Vec3.new(0), sn.Vec3.new(0, 1, 0))
end

function Update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function Draw()
    -- depth write pipeline
    sn.Graphics.bindPipeline(depthPipeline)
    sn.Graphics.setRenderTarget(depthRenderTexture)
    sn.Graphics.setCamera(shadowCamera)
    sn.Graphics.drawModel(model, modelTransform, modelMaterial)
    sn.Graphics.drawModel(floor, floorTransform, floorMaterial)
    sn.Graphics.flush()
    sn.Graphics.readbackTexture(depthRenderTexture, depthTexture)

    -- Lighting
    sn.Graphics.bindPipeline(pipeline)
    sn.Graphics.setCamera(camera)
    sn.Graphics.setUniformBuffer(1, uniformBuffer)
    sn.Graphics.drawModel(model, modelTransform, modelMaterial)
    sn.Graphics.drawModel(floor, floorTransform, floorMaterial)
end
