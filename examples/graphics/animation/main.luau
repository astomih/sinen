local sn = require("@sinen")
local model: sn.Model = sn.Model.new("BrainStem.glb")
local pipeline: sn.GraphicsPipeline = sn.GraphicsPipeline.new()
local material: sn.Material
local camera: sn.Camera3D = sn.Camera3D.new()
function setup()
	local texture: sn.Texture = sn.Texture.new(1, 1)
	texture:fill(sn.Color.new(1, 1, 1, 1))

	local vs: sn.Shader = sn.Shader.new("skinning.vert.snb", sn.ShaderStage.Vertex)
	local fs: sn.Shader = sn.Shader.new("skinning.frag.snb", sn.ShaderStage.Fragment)

	pipeline:setVertexShader(vs)
	pipeline:setFragmentShader(fs)
	pipeline:setEnableDepthTest(true)
	pipeline:setEnableAnimation(true)
	pipeline:build()
	material = sn.Material.new(pipeline)

	camera:lookat(sn.Vec3.new(0, -3, 1), sn.Vec3.new(0, 0, 1), sn.Vec3.new(0, 0, 1))
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
	model:update(sn.Time.delta())
end

function draw()
	local pass = sn.Graphics.begin3DPass(camera)
	material:setUniformBuffer(1, model:getBoneUniformBuffer())
	-- Draw texture
	pass:drawModel(model, sn.Transform.new(), material)
	sn.Graphics.endPass(pass)
end
