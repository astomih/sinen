local sn = require("@sinen")

local titleFont: sn.Font = sn.Font.new()
local infoFont: sn.Font = sn.Font.new()
local blankTexture: sn.Texture = sn.Texture.new(1, 1)

local vertexShader: sn.Shader = sn.Shader.new()
local fragmentShader: sn.Shader = sn.Shader.new()
local pipeline: sn.GraphicsPipeline = sn.GraphicsPipeline.new()

local mode = 1
local pulse = 0.0
local startTime = 0.0

local function clamp(v: number, lo: number, hi: number): number
	if v < lo then
		return lo
	end
	if v > hi then
		return hi
	end
	return v
end

local function fullscreenRect(): sn.Rect
	local size = sn.Window.size()
	sn.Graphics.getCamera2D():resize(size)
	return sn.Rect.new(sn.Vec2.new(0.0, 0.0), size)
end

function setup()
	sn.Graphics.resetGraphicsPipeline()
	sn.Graphics.setClearColor(sn.Color.new(0.0, 0.0, 0.0, 1.0))

	titleFont:load(34)
	infoFont:load(18)

	blankTexture:fill(sn.Color.new(1.0, 1.0, 1.0, 1.0))

	vertexShader:compileAndLoad("shader.slang", sn.ShaderStage.Vertex)
	fragmentShader:compileAndLoad("shader.slang", sn.ShaderStage.Fragment)
	pipeline:setVertexShader(vertexShader)
	pipeline:setFragmentShader(fragmentShader)
	pipeline:setEnableDepthTest(false)
	pipeline:build()

	startTime = sn.Time.seconds()
end

function update()
	if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
		sn.Graphics.resetGraphicsPipeline()
		sn.Script.load("main", ".")
		return
	end

	if sn.Keyboard.isPressed(sn.Keyboard.SPACE) or sn.Mouse.isPressed(sn.Mouse.LEFT) then
		mode = mode + 1
		if mode > 3 then
			mode = 1
		end
		pulse = 1.0
	end

	pulse = clamp(pulse - 0.018, 0.0, 1.0)
end

function draw()
	local size = sn.Window.size()
	local mouse = sn.Mouse.getPosition()
	local time = sn.Time.seconds() - startTime
	local mouseDown = 0.0
	if sn.Mouse.isDown(sn.Mouse.LEFT) then
		mouseDown = 1.0
	end

	local shaderData: sn.Buffer = sn.Buffer.new({
		size.x,
		size.y,
		time,
		mode,
		mouse.x,
		size.y - mouse.y,
		mouseDown,
		pulse,
	})

	sn.Graphics.setGraphicsPipeline(pipeline)
	sn.Graphics.setUniformBuffer(1, shaderData)
	sn.Graphics.drawImage(blankTexture, fullscreenRect())

	sn.Graphics.resetGraphicsPipeline()
	sn.Graphics.drawText(
		"SPACE / CLICK: CHANGE SHADER  ESC: BACK",
		infoFont,
		infoFont
			:region(
				"SPACE / CLICK: CHANGE SHADER  ESC: BACK",
				18,
				sn.Pivot.Center,
				sn.Graphics.getCamera2D():center() + sn.Vec2.new(0, size.y * 0.5 - 34)
			)
			:topLeft(),
		sn.Color.new(1, 1, 1, 0.76),
		18
	)
end
