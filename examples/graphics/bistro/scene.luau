local sn = require("@sinen")

local scene = {}
local renderer = sn.PBRRenderer.new()
local camera = sn.Camera3D.new()
local font = sn.Font.new(20)
local transform = sn.Transform.new()
local gi = sn.GlobalIllumination.new()
local model = nil
local skyMaterial = nil
local sky = nil
local initialized = false
local giAvailable = false
local giEnabled = true
local giCenter = sn.Vec3.new(0)
local exposure = 1.0
local position = sn.Vec3.new(0)
local yaw, pitch = 0, 0
local sunAngle = 0
local animateSun = false
local fps = 60
local captureFrames = 0
local captureQuit = false
local target = sn.RenderTexture.new()
local image = nil
local sun = sn.DirectionalLight.new(sn.Vec3.new(-0.5, -0.8, -0.3), 3.0, sn.Color.new(1, 0.92, 0.79))

local function resetCamera()
	position = sn.Vec3.new(24.822851, 3.160547, -61.648144)
	local direction = sn.Vec3.new(-0.31841794, -0.05823208, 0.9461602)
	yaw = math.atan2(direction.x, -direction.z)
	pitch = math.asin(math.clamp(direction.y, -1, 1))
end

local function updateCamera(aspect: number)
	local cp = math.cos(pitch)
	local direction = sn.Vec3.new(math.sin(yaw) * cp, math.sin(pitch), -math.cos(yaw) * cp)
	camera:lookat(position, position + direction, sn.Vec3.new(0, 1, 0))
	camera:perspective(60, aspect, 0.1, 500)
end

local function moveVolume()
	-- A local volume gives street-level probe spacing instead of spreading a few
	-- probes over the entire city block. Static geometry remains registered.
	giCenter = sn.Vec3.new(math.floor(position.x / 16) * 16, 0, math.floor(position.z / 16) * 16)
	giAvailable = gi:initializeRealtime(
		giCenter + sn.Vec3.new(-32, -1, -32),
		giCenter + sn.Vec3.new(32, 23, 32),
		12, 6, 12, 2
	)
	if giAvailable then renderer:setGlobalIllumination(gi, if giEnabled then 1 else 0) end
end

function scene.setup()
	sn.Graphics.setClearColor(sn.Color.new(0.47, 0.64, 0.8))
	local root = "assets/Bistro_v5_2/"
	if not sn.Filesystem.exists(root .. "BistroExterior.fbx") then return end
	model = sn.Model.new(root .. "BistroExterior.fbx", {
		maxTextureSize = 1024, packedSpecular = true, flipNormalY = true, staticScene = true,
	})
	local environment = root .. "san_giuseppe_bridge_4k.hdr"
	sky = sn.Texture.newCubemap(environment)
	local pipeline = sn.GraphicsPipeline.new()
	pipeline:setVertexShader(sn.BuiltinShader.getCubemapVS())
	pipeline:setFragmentShader(sn.Shader.new("sky.frag.snb", sn.ShaderStage.Fragment))
	pipeline:setEnableDepthTest(true)
	pipeline:setEnableDepthWrite(false)
	pipeline:build()
	skyMaterial = sn.Material.new(pipeline)
	renderer:setEnvironment(
		sn.Texture.newBRDFLUT(128, 128),
		sn.Texture.newIrradianceCubemap(environment, 16, 32),
		sn.Texture.newPrefilteredCubemap(environment, 64, 5, 32)
	)
	resetCamera()
	renderer:setLight(sun)
	renderer:setFillLight(sn.DirectionalLight.new(sn.Vec3.new(0.6, -0.5, 0.2), 0.65, sn.Color.new(0.65, 0.78, 1)))
	renderer:setExposure(exposure)
end

function scene.ready()
	if not model then return end
	assert(model:isReady(), "Bistro model or external textures failed to load")
	-- Build a simplified ray-tracing proxy in memory once after loading.
	gi:addModel(model, transform, nil, nil, 40000)
	gi:setLight(sun)
	gi:setSkyColor(sn.Color.new(0.35, 0.45, 0.65))
	moveVolume()
	initialized = true
	sn.Log.info(string.format("Bistro FBX ready: %d parts, %d materials, %d triangles",
		model:getPartCount(), model:getMaterialCount(), model:getTriangleCount()))
end

function scene.capture(quit: boolean?)
	if not initialized then return end
	captureFrames = 40
	captureQuit = quit == true
	if not image then
		target:create(1280, 720)
		image = sn.Texture.new(1280, 720)
	end
end

function scene.update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Mouse.setRelative(false)
		sn.Mouse.hideCursor(false)
		sn.Script.load("/graphics/main.luau")
	end
	if not initialized then return end
	local frameDt = sn.Time.delta()
	local dt = math.min(frameDt, 0.05)
	if frameDt > 0 then fps += (1 / frameDt - fps) * math.min(frameDt * 3, 1) end
	if captureFrames > 0 then return end
	local looking = sn.Mouse.isDown(sn.Mouse.RIGHT)
	if sn.Mouse.isRelative() ~= looking then
		sn.Mouse.setRelative(looking)
		sn.Mouse.hideCursor(looking)
	end
	if looking then
		local delta = sn.Mouse.getDelta()
		yaw += delta.x * 0.0025
		pitch = math.clamp(pitch + delta.y * 0.0025, -1.5, 1.5)
	end
	local speed = dt * (if sn.Keyboard.isDown(sn.Scancode.LSHIFT) then 18 else 5)
	local forward = sn.Vec3.new(math.sin(yaw), 0, -math.cos(yaw))
	local right = sn.Vec3.new(math.cos(yaw), 0, math.sin(yaw))
	if sn.Keyboard.isDown(sn.Scancode.W) then position += forward * sn.Vec3.new(speed) end
	if sn.Keyboard.isDown(sn.Scancode.S) then position -= forward * sn.Vec3.new(speed) end
	if sn.Keyboard.isDown(sn.Scancode.D) then position += right * sn.Vec3.new(speed) end
	if sn.Keyboard.isDown(sn.Scancode.A) then position -= right * sn.Vec3.new(speed) end
	if sn.Keyboard.isDown(sn.Scancode.E) then position.y += speed end
	if sn.Keyboard.isDown(sn.Scancode.Q) then position.y -= speed end
	if sn.Keyboard.isDown(sn.Scancode.LEFT) then yaw -= dt end
	if sn.Keyboard.isDown(sn.Scancode.RIGHT) then yaw += dt end
	if sn.Keyboard.isDown(sn.Scancode.UP) then pitch = math.min(pitch + dt, 1.5) end
	if sn.Keyboard.isDown(sn.Scancode.DOWN) then pitch = math.max(pitch - dt, -1.5) end
	if sn.Keyboard.isPressed(sn.Scancode.R) then resetCamera() end
	if sn.Keyboard.isPressed(sn.Scancode.G) and giAvailable then
		giEnabled = not giEnabled
		renderer:setGlobalIllumination(gi, if giEnabled then 1 else 0)
	end
	if sn.Keyboard.isPressed(sn.Scancode.P) then animateSun = not animateSun end
	if sn.Keyboard.isPressed(sn.Scancode.Z) then exposure = math.max(0.1, exposure - 0.1) end
	if sn.Keyboard.isPressed(sn.Scancode.X) then exposure = math.min(4, exposure + 0.1) end
	if sn.Keyboard.isPressed(sn.Scancode.F10) then scene.capture() end
	if animateSun then sunAngle += dt * 0.2 end
	sun:setDirection(sn.Vec3.new(-math.cos(sunAngle) * 0.5, -0.8, -math.sin(sunAngle) * 0.5 - 0.3))
	renderer:setLight(sun)
	renderer:setExposure(exposure)
end

local function drawWorld(renderTarget: sn.RenderTexture?)
	local size = sn.Window.size()
	updateCamera(if renderTarget then 1280 / 720 else size.x / size.y)
	local pass = sn.Graphics.begin3DPass(camera, renderTarget)
	pass:drawCubemap(sky, skyMaterial)
	renderer:drawModel(pass, model, transform)
	sn.Graphics.endPass(pass)
end

local function text(value: string, y: number, size: number?)
	sn.Graphics.drawText(value, sn.TextStyle.new(font, sn.Color.new(1), size or 18),
		sn.TextTransform.new(sn.Vec2.new(22, y)))
end

function scene.draw()
	if not model then
		text("BISTRO / ADVANCED GRAPHICS", 24, 26)
		text("Download and prepare the ORCA assets first:", 88)
		text("powershell -File examples/graphics/bistro/prepare.ps1", 126)
		text("Loads original FBX and DDS directly; see README.md.", 164)
		text("Amazon Lumberyard / NVIDIA ORCA - CC BY 4.0", 220, 16)
		text("ESC: examples", 260)
		return
	end
	if not initialized then return end
	if giAvailable then
		if math.abs(position.x - giCenter.x) > 24 or math.abs(position.z - giCenter.z) > 24 then moveVolume() end
		gi:setLight(sun)
		assert(gi:update(0.88), "Bistro GI update failed")
	end
	if captureFrames > 0 then
		captureFrames -= 1
		if captureFrames == 0 then
			drawWorld(target)
			assert(sn.Graphics.readbackTexture(target, image), "Bistro screenshot readback failed")
			assert(sn.Filesystem.write("bistro.png", image:toPngBuffer()), "Bistro screenshot write failed")
			if captureQuit then sn.Event.quit() end
		end
	end
	drawWorld(nil)
	text(string.format("BISTRO / %.0f FPS / %s", fps, sn.Graphics.getBackendName()), 18, 22)
	text(string.format("Native FBX | %d materials | %.2f M triangles | GI: %s | Exposure: %.1f",
		model:getMaterialCount(), model:getTriangleCount() / 1000000,
		if giAvailable then (if giEnabled then "real-time" else "off") else "unavailable", exposure), 49, 16)
	text("RMB: look | WASD/QE: move | Shift: fast | R: reset | G: GI | P: sun | Z/X: exposure | F10: PNG", sn.Window.size().y - 51, 14)
	text("Amazon Lumberyard / NVIDIA ORCA - CC BY 4.0 | ESC: examples", sn.Window.size().y - 29, 14)
end

return scene
