local sn = require("@sinen")
--TODO: depth texture as D24 or D32, PCF, etc...

local model: sn.Model = sn.Model.new("DamagedHelmet.glb")
local modelTransform: sn.Transform = sn.Transform.new()
local floor: sn.Model = sn.Model.newSprite()
local floorTransform: sn.Transform = sn.Transform.new()
local shadowCamera: sn.Camera3D = sn.Camera3D.new()
local depthVS: sn.Shader = sn.Shader.new("depth_write.vert.snb", sn.ShaderStage.Vertex)
local depthFS: sn.Shader = sn.Shader.new("depth_write.frag.snb", sn.ShaderStage.Fragment)
local depthPipeline: sn.GraphicsPipeline = sn.GraphicsPipeline.new()
local depthRenderTexture: sn.RenderTexture = sn.RenderTexture.new()
local depthTexture: sn.Texture = sn.Texture.new(sn.Window.size().x, sn.Window.size().y)
local vs: sn.Shader = sn.Shader.new("shader.vert.snb", sn.ShaderStage.Vertex)
local fs: sn.Shader = sn.Shader.new("shader.frag.snb", sn.ShaderStage.Fragment)
local pipeline: sn.GraphicsPipeline = sn.GraphicsPipeline.new()
local floorTexture: sn.Texture = sn.Texture.new(1, 1)
local camera: sn.Camera3D = sn.Camera3D.new()

local function setShadowUniform(pass: sn.Render3DPass, t: sn.Transform)
	pass:setUniformBuffer(
		1,
		sn.Buffer.new({
			t:getWorldMatrix(),
			shadowCamera:getViewProjection(),
		})
	)
end

function setup()
	modelTransform.position = sn.Vec3.new(0, 0, 0)
	modelTransform.rotation = sn.Vec3.new(90, 0, 0)
	modelTransform.scale = sn.Vec3.new(1, 1, 1)
	floorTransform.rotation.x = 90
	floorTransform.position.y = -1.0

	shadowCamera:orthographic(20, 20, -10, 10.0)
	shadowCamera:lookat(sn.Vec3.new(0.5, 2, 2), sn.Vec3.new(0), sn.Vec3.new(0, 1, 0))


	depthPipeline:setEnableDepthTest(true)
	depthPipeline:setVertexShader(depthVS)
	depthPipeline:setFragmentShader(depthFS)
	depthPipeline:build()

	depthRenderTexture:create(sn.Window.size().x, sn.Window.size().y)
	depthTexture:fill(sn.Color.new(1, 1, 1, 1))

	pipeline:setEnableDepthTest(true)
	pipeline:setVertexShader(vs)
	pipeline:setFragmentShader(fs)
	pipeline:build()

	floorTexture:fill(sn.Color.new(1))

	camera:perspective(70, 16.0 / 9.0, 0.1, 100)
	camera:lookat(sn.Vec3.new(0, 1, 5), sn.Vec3.new(0), sn.Vec3.new(0, 1, 0))
	floor:setTexture(sn.TextureKey.BaseColor, floorTexture)
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
end

function draw()
	-- depth write
	local depthPass = sn.Graphics.begin3DPass(shadowCamera, depthRenderTexture)
	depthPass:setGraphicsPipeline(depthPipeline)
	depthPass:drawModel(model, modelTransform)
	depthPass:drawModel(floor, floorTransform)
	sn.Graphics.endPass(depthPass)
	sn.Graphics.readbackTexture(depthRenderTexture, depthTexture)

	-- Lighting
	local pass = sn.Graphics.begin3DPass(camera)
	pass:setGraphicsPipeline(pipeline)

	setShadowUniform(pass, modelTransform)
	pass:setTexture(1, depthTexture)
	pass:drawModel(model, modelTransform)
	setShadowUniform(pass, floorTransform)
	pass:setTexture(1, depthTexture)
	pass:drawModel(floor, floorTransform)
	sn.Graphics.endPass(pass)
end
