local sn = require("@sinen")
local model: sn.Model = sn.Model.new()
local pipeline: sn.GraphicsPipeline = sn.GraphicsPipeline.new()
local camera: sn.Camera3D = sn.Camera3D.new()
function setup()
	model:load("BrainStem.glb")
	local texture: sn.Texture = sn.Texture.new()
	texture:fill(sn.Color.new(1, 1, 1, 1))

	local vs: sn.Shader = sn.Shader.new()
	local fs: sn.Shader = sn.Shader.new()

	vs:compileAndLoad("skinning.slang", sn.ShaderStage.Vertex)
	fs:compileAndLoad("skinning.slang", sn.ShaderStage.Fragment)

	pipeline:setVertexShader(vs)
	pipeline:setFragmentShader(fs)
	pipeline:setEnableDepthTest(true)
	pipeline:setEnableAnimation(true)
	pipeline:build()

	camera:lookat(sn.Vec3.new(0, -3, 1), sn.Vec3.new(0, 0, 1), sn.Vec3.new(0, 0, 1))
end

function update()
	if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
		sn.Script.load("main", ".")
	end
	model:update(sn.Time.delta())
end

function draw()
	sn.Graphics.begin3D(camera)
	sn.Graphics.setGraphicsPipeline(pipeline)
	sn.Graphics.setUniformBuffer(1, model:getBoneUniformBuffer())
	-- Draw texture
	sn.Graphics.drawModel(model, sn.Transform.new())
	sn.Graphics.finish()
end
