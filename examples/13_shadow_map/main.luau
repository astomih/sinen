--TODO: depth texture as D24 or D32, PCF, etc...

local model = Model.new()
local modelTransform = Transform.new()
local floor = Model.new()
local floorTransform = Transform.new()
local shadowCamera = Camera3D.new()
local depthVS = Shader.new()
local depthFS = Shader.new()
local depthPipeline = GraphicsPipeline.new()
local depthRenderTexture = RenderTexture.new()
local depthTexture = Texture.new(Window.size().x, Window.size().y)
local vs = Shader.new()
local fs = Shader.new()
local pipeline = GraphicsPipeline.new()
local uniformBuffer: Buffer
local floorTexture = Texture.new()
local camera = Camera3D.new()

function setup()
	model:load("DamagedHelmet.glb")
	modelTransform.position = Vec3.new(0, 0, 0)
	modelTransform.rotation = Vec3.new(90, 0, 0)
	modelTransform.scale = Vec3.new(1, 1, 1)
	floor:loadSprite()
	floorTransform.rotation.x = 90
	floorTransform.position.y = -1.0

	shadowCamera:orthographic(20, 20, -10, 10.0)
	shadowCamera:lookat(Vec3.new(0.5, 2, 2), Vec3.new(0), Vec3.new(0, 1, 0))

	depthVS:compileAndLoad("depth_write.slang", ShaderStage.Vertex)
	depthFS:compileAndLoad("depth_write.slang", ShaderStage.Fragment)

	depthPipeline:setEnableDepthTest(true)
	depthPipeline:setVertexShader(depthVS)
	depthPipeline:setFragmentShader(depthFS)
	depthPipeline:build()

	depthRenderTexture:create(Window.size().x, Window.size().y)
	depthTexture:fill(Color.new(1, 1, 1, 1))

	vs:compileAndLoad("shader.slang", ShaderStage.Vertex)
	fs:compileAndLoad("shader.slang", ShaderStage.Fragment)
	pipeline:setEnableDepthTest(true)
	pipeline:setVertexShader(vs)
	pipeline:setFragmentShader(fs)
	pipeline:build()

	uniformBuffer = Buffer.new({ shadowCamera })

	floorTexture:fill(Color.new(1))

	camera:perspective(70, 16.0 / 9.0, 0.1, 100)
	camera:lookat(Vec3.new(0, 1, 5), Vec3.new(0), Vec3.new(0, 1, 0))
	floor:setTexture(TextureKey.BaseColor, floorTexture)
end

function update()
	if Keyboard.isPressed(Keyboard.ESCAPE) then
		Script.load("main", ".")
	end
end

function draw()
	-- depth write
	Graphics.setGraphicsPipeline(depthPipeline)
	Graphics.setCamera3D(shadowCamera)

	Graphics.beginRenderTarget(depthRenderTexture)
	Graphics.drawModel(model, modelTransform)
	Graphics.drawModel(floor, floorTransform)
	Graphics.endRenderTarget()
	Graphics.readbackTexture(depthRenderTexture, depthTexture)

	-- Lighting
	Graphics.setGraphicsPipeline(pipeline)
	Graphics.setCamera3D(camera)

	Graphics.setUniformBuffer(1, uniformBuffer)
	Graphics.setTexture(1, depthTexture)
	Graphics.drawModel(model, modelTransform)
	Graphics.drawModel(floor, floorTransform)
end
