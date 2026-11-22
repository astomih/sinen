local sn = require("@sinen")

local scene = {}
local renderer = sn.PBRRenderer.new()
local gi = sn.GlobalIllumination.new()
local camera = sn.Camera3D.new()
local font = sn.Font.new(20)
local objects = {}
local enabled = true
local animated = true
local elapsed = 0
local movingObject
local capture = 0
local quitAfterCapture = false
local target = sn.RenderTexture.new()
local image = sn.Texture.new(960, 720)
local light = sn.PointLight.new(sn.Vec3.new(0, 2.65, 0.3), 2.8, sn.Color.new(1, 0.91, 0.78))

local function add(model: sn.Model, position: sn.Vec3, emission: sn.Color?)
	local transform = sn.Transform.new()
	transform.position = position
	table.insert(objects, { model = model, transform = transform, emission = emission, giModel = model })
end

local function box(size: sn.Vec3, position: sn.Vec3, color: sn.Color)
	local builder = sn.MeshBuilder.new()
	builder:addBox(size, color)
	add(builder:toModel(), position)
end

function scene.setup()
	sn.Graphics.setClearColor(sn.Color.new(0.018, 0.024, 0.038))
	local white = sn.Color.new(0.76, 0.76, 0.72)
	box(sn.Vec3.new(4.2, 0.12, 4.2), sn.Vec3.new(0, -0.06, 0), white)
	box(sn.Vec3.new(4.2, 0.12, 4.2), sn.Vec3.new(0, 3.06, 0), white)
	box(sn.Vec3.new(4.2, 3.0, 0.12), sn.Vec3.new(0, 1.5, -2.06), white)
	box(sn.Vec3.new(0.12, 3.0, 4.2), sn.Vec3.new(-2.06, 1.5, 0), sn.Color.new(0.78, 0.055, 0.035))
	box(sn.Vec3.new(0.12, 3.0, 4.2), sn.Vec3.new(2.06, 1.5, 0), sn.Color.new(0.035, 0.12, 0.78))
	box(sn.Vec3.new(0.95, 1.25, 0.95), sn.Vec3.new(-0.85, 0.625, -0.25), white)
	local sphere = sn.MeshBuilder.new()
	sphere:addSphere(0.58, 24, 48, sn.Color.new(0.85))
	add(sphere:toModel(), sn.Vec3.new(0.82, 0.58, 0.5))
	movingObject = objects[#objects]
	-- A lower resolution proxy keeps per-frame BVH construction inexpensive.
	local proxy = sn.MeshBuilder.new()
	proxy:addSphere(0.58, 8, 16, sn.Color.new(0.85))
	movingObject.giModel = proxy:toModel()
	-- The small ceiling panel contributes emitted radiance to the GPU rays.
	local panel = sn.MeshBuilder.new()
	panel:addBox(sn.Vec3.new(0.9, 0.02, 0.65))
	local panelModel = panel:toModel()
	local emission = sn.Texture.new(1, 1)
	emission:fill(sn.Color.new(1, 0.91, 0.78))
	panelModel:setTexture(sn.TextureKey.Emissive, emission)
	add(panelModel, sn.Vec3.new(0, 2.97, 0.3), sn.Color.new(1, 0.91, 0.78))

	gi:setLight(light)
	assert(
		gi:initializeRealtime(sn.Vec3.new(-1.9, 0.08, -1.9), sn.Vec3.new(1.9, 2.92, 1.9), 9, 7, 9, 3),
		"Real-time GI requires Direct3D 12, Vulkan, or WebGPU"
	)
	renderer:setLight(light)
	renderer:setExposure(1.25)
	renderer:setGlobalIllumination(gi)
	target:create(960, 720)
	camera:lookat(sn.Vec3.new(0, 1.6, 6.5), sn.Vec3.new(0, 1.4, 0), sn.Vec3.new(0, 1, 0))
end

function scene.saveComparison(quit: boolean?)
	-- Freeze motion and let temporal history converge at this exact scene state.
	capture = -32
	quitAfterCapture = quit == true
end

function scene.update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
	if sn.Keyboard.isPressed(sn.Scancode.SPACE) then
		enabled = not enabled
		renderer:setGlobalIllumination(gi, if enabled then 1 else 0)
	end
	if sn.Keyboard.isPressed(sn.Scancode.S) then
		scene.saveComparison()
	end
	if sn.Keyboard.isPressed(sn.Scancode.P) then
		animated = not animated
	end
	if animated and capture == 0 then
		elapsed += sn.Time.delta()
		light:setPosition(sn.Vec3.new(math.sin(elapsed * 0.7) * 1.1, 2.65, 0.3))
		movingObject.transform.position.x = 0.65 + math.sin(elapsed * 0.55) * 0.55
	end
end

local function drawRoom(renderTarget: sn.RenderTexture?)
	local size = sn.Window.size()
	camera:perspective(42, if renderTarget then 960 / 720 else size.x / size.y, 0.1, 50)
	local pass = sn.Graphics.begin3DPass(camera, renderTarget)
	for _, object in objects do
		renderer:drawModel(pass, object.model, object.transform)
	end
	sn.Graphics.endPass(pass)
end

function scene.draw()
	gi:beginFrame()
	for _, object in objects do
		gi:addModel(object.giModel, object.transform, sn.Color.new(1), object.emission)
	end
	gi:setLight(light)
	assert(gi:update(0.85), "Real-time GI update failed")
	renderer:setLight(light)
	if capture < 0 then
		capture += 1
		if capture == 0 then
			capture = 1
		end
	end
	if capture > 0 then
		local withGI = capture == 2
		renderer:setGlobalIllumination(gi, if withGI then 1 else 0)
		drawRoom(target)
		assert(sn.Graphics.readbackTexture(target, image), "GI screenshot readback failed")
		local name = if withGI then "gi.png" else "direct.png"
		assert(sn.Filesystem.write(name, image:toPngBuffer()), "GI screenshot write failed")
		capture += 1
		if capture == 3 then
			capture = 0
			renderer:setGlobalIllumination(gi, if enabled then 1 else 0)
			if quitAfterCapture then
				sn.Event.quit()
			end
		end
	end
	drawRoom(nil)
	sn.Graphics.drawText(
		if enabled then "REAL-TIME GI  /  64 GPU rays per probe per frame" else "DIRECT LIGHT ONLY",
		sn.TextStyle.new(font, sn.Color.new(1), 20),
		sn.TextTransform.new(sn.Vec2.new(24, 20))
	)
	sn.Graphics.drawText(
		"SPACE: toggle GI    P: pause motion    S: save PNGs    ESC: examples",
		sn.TextStyle.new(font, sn.Color.new(0.8), 16),
		sn.TextTransform.new(sn.Vec2.new(24, sn.Window.size().y - 32))
	)
end

return scene
