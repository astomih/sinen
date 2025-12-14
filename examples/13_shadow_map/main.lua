--TODO: depth texture as D24 or D32, PCF, etc...

local model = sn.Model.new()
model:load("DamagedHelmet.glb")
local transform1 = sn.Transform.new()
transform1.position = sn.Vec3.new(0, 0, 0)
transform1.rotation = sn.Vec3.new(90, 0, 0)
transform1.scale = sn.Vec3.new(1, 1, 1)
local floor = sn.Model.new()
floor:loadSprite()
local transform2 = sn.Transform.new()
transform2.rotation.x = 90
transform2.position.y = -1.0

local shadowCamera = sn.Camera.new()
shadowCamera:orthographic(20, 20, -10, 10.0)
shadowCamera:lookat(sn.Vec3.new(0.5, 2, 2), sn.Vec3.new(0), sn.Vec3.new(0, 1, 0))

local depthVS = sn.Shader.new()
depthVS:compileLoadVertexShader("depth_write.slang")
local depthFS = sn.Shader.new()
depthFS:compileLoadFragmentShader("depth_write.slang")

local depthPipeline = sn.GraphicsPipeline.new()
depthPipeline:setEnableDepthTest(true)
depthPipeline:setVertexShader(depthVS)
depthPipeline:setFragmentShader(depthFS)
depthPipeline:build()

local depthRenderTexture = sn.RenderTexture.new()
depthRenderTexture:create(sn.Window.size().x, sn.Window.size().y)
local depthTexture = sn.Texture.new(sn.Window.size().x, sn.Window.size().y)
depthTexture:fill(sn.Color.new(1, 1, 1, 1))


local vs = sn.Shader.new()
vs:compileLoadVertexShader("shader.slang")
local fs = sn.Shader.new()
fs:compileLoadFragmentShader("shader.slang")
local pipeline = sn.GraphicsPipeline.new()
pipeline:setEnableDepthTest(true)
pipeline:setVertexShader(vs)
pipeline:setFragmentShader(fs)
pipeline:build()

local uniformData = sn.UniformData.new()
uniformData:addCamera(shadowCamera)

local modelMaterial = sn.Material.new()
modelMaterial:appendTexture(model:getMaterial():getTexture(0))
modelMaterial:appendTexture(depthTexture)

local floorMaterial = sn.Material.new()
local floorTexture = sn.Texture.new()
floorTexture:fill(sn.Color.new(1))
floorMaterial:appendTexture(floorTexture)
floorMaterial:appendTexture(depthTexture)

local camera = sn.Camera.new()
camera:perspective(70, 16.0 / 9.0, 0.1, 100)
camera:lookat(sn.Vec3.new(0, 1, 5), sn.Vec3.new(0), sn.Vec3.new(0, 1, 0))

function Update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function Draw()
    sn.Graphics.bindPipeline(depthPipeline)
    sn.Graphics.setRenderTarget(depthRenderTexture)
    sn.Graphics.setCamera(shadowCamera)
    sn.Graphics.drawModel(model, transform1, modelMaterial)
    sn.Graphics.drawModel(floor, transform2, floorMaterial)
    sn.Graphics.flush()
    sn.Graphics.readbackTexture(depthRenderTexture, depthTexture)
    sn.Graphics.bindPipeline(pipeline)
    sn.Graphics.setCamera(camera)
    sn.Graphics.setUniformData(1, uniformData)
    sn.Graphics.drawModel(model, transform1, modelMaterial)
    sn.Graphics.drawModel(floor, transform2, floorMaterial)
end
