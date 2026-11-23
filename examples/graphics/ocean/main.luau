local sn = require("@sinen")

local ok, err = sn.Script.loadPlugin("ocean")
assert(ok, err)
local Ocean = require("@sinen/plugin/ocean")
local compiled, compileError = sn.Script.loadPlugin("shader_compiler")
assert(compiled, compileError)

local camera = sn.Camera3D.new()
local transform = sn.Transform.new()
local font = sn.Font.new(20)
local water = sn.Texture.new(1, 1)
local material
local ocean, vertices, indices
local time = 0
local paused = false
local phillips = false
local swell = true
local eye = sn.Vec3.new(128, 8, 190)

local function reset()
	ocean = Ocean.new({
		resolution = 128, patchSize = 256, seed = 42, choppiness = 1.2,
		wind = { spectrum = if phillips then "phillips" else "jonswap",
			significantWaveHeight = 2.4, peakPeriod = 6, direction = 0.2 },
		swell = { spectrum = "jonswap", significantWaveHeight = if swell then 1.6 else 0,
			peakPeriod = 11, direction = 1.1, peakEnhancement = 5, spread = 18 },
	})
	ocean:update(time)
	vertices, indices = ocean:createMeshBuffers()
end

function setup()
	sn.Graphics.setClearColor(sn.Color.new(0.804, 0.877, 0.919))
	water:fill(sn.Color.new(1))
	local pipeline = sn.GraphicsPipeline.new()
	pipeline:setVertexShader(sn.Shader.compileAndLoad("ocean.slang", sn.ShaderStage.Vertex))
	pipeline:setFragmentShader(sn.Shader.compileAndLoad("ocean.slang", sn.ShaderStage.Fragment))
	pipeline:setEnableDepthTest(true)
	pipeline:build()
	material = sn.Material.new(pipeline)
	material:setTexture(0, water)
	material:setUniformBuffer(1, sn.Buffer.new({ transform:getWorldMatrix() }))
	reset()
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then sn.Script.load("/graphics/main.luau") end
	if sn.Keyboard.isPressed(sn.Scancode.SPACE) then paused = not paused end
	if sn.Keyboard.isPressed(sn.Scancode.P) then phillips = not phillips; reset() end
	if sn.Keyboard.isPressed(sn.Scancode.S) then swell = not swell; reset() end
	if not paused then
		time += sn.Time.delta()
		ocean:update(time)
		ocean:writeVertices(vertices)
	end
	if sn.Keyboard.isDown(sn.Scancode.UP) then eye.y = math.min(40, eye.y + sn.Time.delta() * 5) end
	if sn.Keyboard.isDown(sn.Scancode.DOWN) then eye.y = math.max(3, eye.y - sn.Time.delta() * 5) end
end

function draw()
	local size = sn.Window.size()
	camera:perspective(60, size.x / math.max(1, size.y), 0.1, 1200)
	camera:lookat(eye, sn.Vec3.new(128, 1, 60), sn.Vec3.new(0, 1, 0))
	-- Separate slots for the world matrix and camera position.
	material:setUniformBuffer(0, sn.Buffer.new({ eye, 0 }))
	local pass = sn.Graphics.begin3DPass(camera)
	pass:drawMesh(vertices, indices, transform, material)
	sn.Graphics.endPass(pass)
	sn.Graphics.drawText(string.format("FFT Ocean | %s + %s\nP: spectrum  S: swell  Space: pause  Up/Down: height  Esc: back",
		if phillips then "Phillips" else "JONSWAP", if swell then "swell" else "no swell"),
		sn.TextStyle.new(font, sn.Color.new(1), 20),
		sn.TextTransform.new(sn.Vec2.new(20, 20), 0, sn.Pivot.TopLeft))
end
