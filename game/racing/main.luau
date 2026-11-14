local sn = require("@sinen")
local Track = require("./track")
local Car = require("./car")
local Grid = require("./grid")
local Art = require("./art")
local Hud = require("./hud")

local v3 = sn.Vec3.new
local key, button = sn.Scancode, sn.GamepadButton
local kb, pad = sn.Keyboard, sn.Gamepad
local totalLaps = 3
local fixedStep = 1 / 90

local camera = sn.Camera3D.new()
local renderer = sn.PBRRenderer.new()
local sun = sn.DirectionalLight.new(v3(0.43, -0.77, -0.45), 2.45, sn.Color.new(1, 0.92, 0.79, 1))
local identity = sn.Transform.new()
local pose, wheel, shadow = sn.Transform.new(), sn.Transform.new(), sn.Transform.new()
local up = v3(0, 1, 0)
local terrain: { sn.Model }
local cars: typeof(Art.cars())
local sky: sn.Texture
local engine: sn.Sound
local tone: sn.Sound
local green: sn.Sound
local hud: typeof(Hud.new())

type Phase = "title" | "countdown" | "racing" | "paused" | "finished"
local phase: Phase = "title"
local beforePause: Phase = "racing"
local field = Grid.new()
local elapsed, lapStart, lastLap, bestLap = 0, 0, 0, 0
local countdown, accumulator, goFlash = 3, 0, 0
local cameraMode = 1
local viewYaw = field[1].car.yaw
local loaded = false
local connected = false
local padName: string? = nil
local driveCommands: { Grid.Input } = {}
local fromLauncher = sn.Filesystem.exists("/racing/main.luau") and sn.Filesystem.exists("/main.luau")

local function pressed(k: sn.Scancode, b: sn.GamepadButton): boolean
	return kb.isPressed(k) or pad.isPressed(b)
end

local function held(k: sn.Scancode): boolean
	return kb.isPressed(k) or kb.isDown(k)
end

local function padHeld(b: sn.GamepadButton): boolean
	return pad.isPressed(b) or pad.isDown(b)
end

local function readInput(): Grid.Input
	local steering = (if held(key.D) or held(key.RIGHT) or padHeld(button.DPAD_RIGHT) then 1 else 0)
		- (if held(key.A) or held(key.LEFT) or padHeld(button.DPAD_LEFT) then 1 else 0)
	local stick = pad.getLeftStick().x
	return {
		steer = if steering ~= 0 then steering else math.sign(stick) * math.abs(stick) ^ 1.4,
		gas = if held(key.W) or held(key.UP) then 1 else pad.getRightTrigger(),
		brake = if held(key.S) or held(key.DOWN) then 1 else pad.getLeftTrigger(),
	}
end

local function begin()
	field = Grid.new()
	elapsed, lapStart, lastLap, bestLap = 0, 0, 0, 0
	countdown, accumulator, goFlash = 3, 0, 0
	viewYaw = field[1].car.yaw
	phase = "countdown"
	tone:restart()
	tone:play()
end

local function updateCamera(dt: number)
	local car = field[1].car
	local size = sn.Window.size()
	local aspect = math.max(1, size.x) / math.max(1, size.y)
	local yawDifference = (car.yaw - viewYaw + math.pi) % (2 * math.pi) - math.pi
	viewYaw += yawDifference * (1 - math.exp(-7 * dt))
	local y = Track.height(car.x, car.z)
	local x, z = math.sin(viewYaw), -math.cos(viewYaw)
	if phase == "title" then
		camera:perspective(57, aspect, 0.07, 1100)
		camera:lookat(v3(car.x + 5.5, y + 2.8, car.z + 6.0), v3(car.x - 0.65, y + 0.72, car.z - 1.7), up)
	elseif cameraMode == 2 then
		local fx, fz = math.sin(car.yaw), -math.cos(car.yaw)
		camera:perspective(72, aspect, 0.08, 1100)
		local px, pz = car.x + fx * 1.05, car.z + fz * 1.05
		camera:lookat(v3(px, y + 1.22, pz), v3(px + fx * 32, y + 0.87 + Track.height(px + fx * 9, pz + fz * 9) - y, pz + fz * 32), up)
	else
		local distance = if cameraMode == 3 then 10.1 else 6.8
		local height = if cameraMode == 3 then 4.4 else 2.78
		camera:perspective(64 + math.min(10, car.speed * 0.16), aspect, 0.07, 1100)
		local px, pz = car.x - x * distance, car.z - z * distance
		camera:lookat(v3(px, math.max(y, Track.height(px, pz)) + height, pz),
			v3(car.x + x * 8, y + 0.61, car.z + z * 8), up)
	end
end

local function updateRace(dt: number, playerInput: Grid.Input)
	local player = field[1].car
	for i = 2, #field do
		driveCommands[i] = Grid.drive(field[i])
	end
	driveCommands[1] = playerInput
	accumulator += dt
	while accumulator >= fixedStep do
		accumulator -= fixedStep
		elapsed += fixedStep
		for i, driver in field do
			local input = driveCommands[i]
			Car.step(driver.car, input.gas, input.brake, input.steer, fixedStep)
		end
		Grid.resolve(field)
		local previousLap = player.lap
		for _, driver in field do
			Car.updateProgress(driver.car, totalLaps)
			if driver.car.finished and not driver.finish then driver.finish = elapsed end
		end
		if player.lap > previousLap then
			lastLap = elapsed - lapStart
			bestLap = if bestLap == 0 then lastLap else math.min(bestLap, lastLap)
			lapStart = elapsed
		elseif lapStart == 0 and player.nextGate > 0 then
			lapStart = elapsed
		end
		if player.finished then
			phase = "finished"
			break
		end
	end
end

function setup()
	sn.Window.rename("Sinen | RIDGELINE")
	sn.Graphics.setClearColor(sn.Color.new(0.53, 0.69, 0.83, 1))
	renderer:setExposure(1.25)
	renderer:setLight(sun)
	local source = "assets/kloofendal_48d_partly_cloudy_puresky_1k.exr"
	sky = sn.Texture.newCubemap(source)
	renderer:setEnvironment(sn.Texture.newBRDFLUT(128, 128),
		sn.Texture.newIrradianceCubemap(source, 24, 96),
		sn.Texture.newPrefilteredCubemap(source, 64, 6, 80))
	terrain, cars = Art.scenery(), Art.cars()
	engine = sn.Sound.new("assets/engine.wav")
	engine:setLooping(true)
	engine:setVolume(0)
	engine:play()
	tone = sn.Sound.new("assets/countdown.wav")
	tone:setVolume(0.42)
	green = sn.Sound.new("assets/start.wav")
	green:setVolume(0.48)
	hud = Hud.new()
end

function ready()
	updateCamera(1)
	loaded = true
end

function update()
	if not loaded then return end
	local dt = math.clamp(sn.Time.delta(), 0, 1 / 20)
	local nowConnected = pad.isConnected()
	if nowConnected ~= connected then
		local wasConnected = connected
		connected = nowConnected
		padName = if connected then pad.getName() or "Gamepad" else nil
		sn.Log.info(if padName then "RIDGELINE gamepad: " .. padName else "RIDGELINE gamepad disconnected")
		if wasConnected and (phase == "countdown" or phase == "racing") then
			beforePause, phase = phase, "paused"
		end
	end

	local focus = sn.Window.hasFocus()
	local input = readInput()
	if (phase == "racing" or phase == "countdown") and (not focus or pressed(key.ESCAPE, button.START)) then
		beforePause, phase = phase, "paused"
	elseif focus then
		if phase == "title" or phase == "finished" or phase == "paused" then
			if pressed(key.Q, button.B) then
				engine:stop()
				if fromLauncher then sn.Script.load("/main.luau") else sn.Event.quit() end
				return
			elseif pressed(key.ENTER, button.A) or sn.Mouse.isPressed(sn.Mouse.LEFT)
				or (phase == "paused" and pad.isPressed(button.START)) then
				if phase == "paused" then phase = beforePause else begin() end
			end
		elseif phase == "countdown" then
			local previous = math.ceil(countdown)
			countdown = math.max(0, countdown - dt)
			if countdown == 0 then
				phase, goFlash = "racing", 1.4
				green:restart()
				green:play()
			elseif math.ceil(countdown) < previous then
				tone:restart()
				tone:play()
			end
		elseif phase == "racing" then
			if pressed(key.R, button.Y) then Car.reset(field[1].car) end
			goFlash = math.max(0, goFlash - dt)
			updateRace(dt, input)
		end
		if pressed(key.C, button.X) then cameraMode = cameraMode % 3 + 1 end
	end

	local rpm = field[1].car.rpm
	engine:setPitch(math.clamp(rpm / 2900, 0.62, 2.7))
	engine:setVolume(if phase == "racing" or phase == "countdown" then 0.20 + input.gas * 0.22 else 0)
	updateCamera(dt)
end

local function drawCar(pass: sn.Render3DPass, car: Car.State, index: number)
	local x, z = car.x, car.z
	local y = Track.height(x, z) + 0.095
	local yaw = math.deg(-car.yaw)
	local fx, fz = math.sin(car.yaw), -math.cos(car.yaw)
	local rx, rz = -fz, fx
	local grade = (Track.height(x + fx, z + fz) - Track.height(x - fx, z - fz)) / 2
	pose.position = v3(x, y, z)
	pose.rotation = v3(math.deg(math.atan(grade)) - car.longitudinal * 0.17, yaw, car.lateral * 0.22)
	shadow.position = v3(x, y + 0.011, z)
	shadow.rotation = v3(0, yaw, 0)
	renderer:drawModel(pass, cars.shadow, shadow)
	renderer:drawModel(pass, cars.bodies[index], pose)
	renderer:drawModel(pass, cars.chassis, pose)
	renderer:drawModel(pass, cars.cabin, pose)
	renderer:drawModel(pass, cars.lamp, pose)
	if car.brake > 0.12 then renderer:drawModel(pass, cars.brake, pose) end
	for _, offset in { -1.325, 1.325 } do
		for _, side in { -1, 1 } do
			local wx, wz = x + fx * offset + rx * side * 0.88, z + fz * offset + rz * side * 0.88
			wheel.position = v3(wx, Track.height(wx, wz) + 0.418, wz)
			-- Convert yaw * axle rotation * spin to Sinen's X-Y-Z Euler sequence.
			local heading = -car.yaw - (if offset > 0 then car.steer else 0)
			local sh, ch = math.sin(heading), math.cos(heading)
			local ss, cs = math.sin(car.spin), math.cos(car.spin)
			local sinY = sh * cs
			if math.abs(sinY) > 0.999999 then
				wheel.rotation = v3(math.sign(sh) * 90, math.sign(sinY) * 90, 0)
			else
				wheel.rotation = v3(math.deg(math.atan2(-ss, ch * cs)),
					math.deg(math.asin(sinY)), math.deg(math.atan2(ch, -sh * ss)))
			end
			renderer:drawModel(pass, cars.wheel, wheel)
			renderer:drawModel(pass, cars.hub, wheel)
		end
	end
end

function draw()
	if not loaded then return end
	local pass = sn.Graphics.begin3DPass(camera)
	pass:drawCubemap(sky)
	for _, model in terrain do renderer:drawModel(pass, model, identity) end
	for i, driver in field do drawCar(pass, driver.car, i) end
	sn.Graphics.endPass(pass)
	hud:draw(phase, field, totalLaps, elapsed - lapStart, lastLap, bestLap, elapsed, countdown, goFlash, padName, fromLauncher)
end
