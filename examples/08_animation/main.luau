local model: Model = Model.new()
local pipeline: GraphicsPipeline = GraphicsPipeline.new()
function setup()
	model:load("BrainStem.glb")
	local texture: Texture = Texture.new()
	texture:fill(Color.new(1, 1, 1, 1))

	local vs: Shader = Shader.new()
	local fs: Shader = Shader.new()

	vs:compileAndLoad("skinning.slang", ShaderStage.Vertex)
	fs:compileAndLoad("skinning.slang", ShaderStage.Fragment)

	pipeline:setVertexShader(vs)
	pipeline:setFragmentShader(fs)
	pipeline:setEnableDepthTest(true)
	pipeline:setEnableAnimation(true)
	pipeline:build()

	Graphics.getCamera3D():lookat(Vec3.new(0, -3, 1), Vec3.new(0, 0, 1), Vec3.new(0, 0, 1))
end

function update()
	if Keyboard.isPressed(Keyboard.ESCAPE) then
		Script.load("main", ".")
	end
	model:update(Time.delta())
end

function draw()
	Graphics.setGraphicsPipeline(pipeline)
	Graphics.setUniformBuffer(1, model:getBoneUniformBuffer())
	-- Draw texture
	Graphics.drawModel(model, Transform.new())
end
