local sn = require("@sinen")
local Art = require("./art")
local Arena = require("./arena")
local Collision = require("./collision")
local Hud = require("./hud")

local v3 = sn.Vec3.new
local color = sn.Color.new
local keys = sn.Scancode
local up = v3(0, 1, 0)
local waveSizes = { 4, 6, 8 }
local capacity = 18
local reloadDuration = 1.05
local eyeHeight = 1.64
local droneRadius = 0.53

type Drone = {
	feet: sn.Vec3, center: sn.Vec3, transform: sn.Transform,
	hp: number, cooldown: number, charge: number, phase: number, side: number,
}
type Effect = { transform: sn.Transform, velocity: sn.Vec3, life: number }
type Pickup = { position: sn.Vec3, transform: sn.Transform }
type State = "title" | "playing" | "paused" | "lost" | "won"

local camera = sn.Camera3D.new()
local viewCamera = sn.Camera3D.new()
local renderer = sn.PBRRenderer.new()
local keyLight = sn.DirectionalLight.new(v3(0.32, -0.83, -0.46), 1.25, color(0.89, 0.96, 1, 1))
local bayLight = sn.PointLight.new(v3(0, 4.6, 6.5), 8, color(0.34, 0.78, 1, 1))
local flashLight = sn.PointLight.new(v3(0), 8, color(0.16, 0.78, 1, 1))
local viewLight = sn.PointLight.new(v3(-0.25, 0.35, -0.45), 0.21, color(0.9, 0.98, 1, 1))

local parts: { Art.Part } = {}
local solids: { Art.Box } = {}
local models: typeof(Art.actors(Art.materials()))
local hud: typeof(Hud.new())
local sounds: { [string]: sn.Sound } = {}
local drones: { Drone } = {}
local bolts: { Effect } = {}
local sparks: { Effect } = {}
local pickups: { Pickup } = {}
local emitterPose = Art.transform(v3(0))
local beamPose = Art.transform(v3(0))

local state: State = "title"
local wave, health, charge, reserve, score = 0, 100, capacity, 126, 0
local feet, yaw, pitch, verticalSpeed = Arena.spawn, 0, 0, 0
local grounded, aiming = true, false
local eye, forward = feet + v3(0, eyeHeight, 0), v3(0, 0, -1)
local velocityX, velocityZ = 0, 0
local elapsed, intermission, cooldown, reload = 0, 1.35, 0, 0
local flash, hit, damage, sway, recoil, aimBlend = 0, 0, 0, 0, 0, 0
local fov, stride = 72, 0
local readyToDraw = false
local fromLauncher = sn.Filesystem.exists("/fps/main.luau") and sn.Filesystem.exists("/main.luau")

local function changeState(nextState: State)
	state = nextState
	sn.Mouse.setRelative(nextState == "playing")
	sn.Mouse.hideCursor(nextState == "playing")
end

local function restart()
	table.clear(drones)
	table.clear(bolts)
	table.clear(sparks)
	table.clear(pickups)
	wave, health, charge, reserve, score = 0, 100, capacity, 126, 0
	feet, yaw, pitch, verticalSpeed = Arena.spawn, 0, 0, 0
	velocityX, velocityZ = 0, 0
	grounded, aiming = true, false
	fov = 72
	elapsed, intermission, cooldown, reload = 0, 1.35, 0, 0
	flash, hit, damage, sway, recoil, aimBlend, stride = 0, 0, 0, 0, 0, 0, 0
	changeState("playing")
end

local function playCue(name: string)
	local sound = sounds[name]
	sound:restart()
	sound:play()
end

local function held(key: sn.Scancode): boolean
	return sn.Keyboard.isDown(key) or sn.Keyboard.isPressed(key)
end

local function pressed(key: sn.Scancode): boolean
	return sn.Keyboard.isPressed(key)
end

local function updateCamera(dt: number)
	local cosPitch = math.cos(pitch)
	forward = v3(math.sin(yaw) * cosPitch, math.sin(pitch), -math.cos(yaw) * cosPitch)
	local bob = math.sin(stride * 2) * sway
	eye = feet + v3(0, eyeHeight + bob * 0.035, 0)
	local size = sn.Window.size()
	fov += ((if aiming then 53 else 72) - fov) * (1 - math.exp(-14 * dt))
	camera:perspective(fov, math.max(size.x, 1) / math.max(size.y, 1), 0.055, 75)
	camera:lookat(eye, eye + forward, up)
	viewCamera:perspective(58, math.max(size.x, 1) / math.max(size.y, 1), 0.025, 5)
	viewCamera:lookat(v3(0), v3(0, 0, -1), up)

	local side = math.sin(stride) * sway * 0.021
	local lower = if reload > 0 then math.sin(math.pi * reload / reloadDuration) * 0.19 else 0
	emitterPose.position = v3(0.32 - 0.32 * aimBlend + side, -0.26 + 0.092 * aimBlend + bob * 0.018 - lower, -0.64 + recoil * 0.13)
	emitterPose.rotation = v3(recoil * 14 - lower * 130, -5 + aimBlend * 5, -6 + aimBlend * 6 - side * 180)
end

local function movePlayer(dt: number)
	local motion = sn.Mouse.getDelta()
	yaw = (yaw + motion.x * 0.00225) % (2 * math.pi)
	pitch = math.clamp(pitch + motion.y * 0.00225, -1.45, 1.45)
	aiming = sn.Mouse.isDown(sn.Mouse.RIGHT) or sn.Mouse.isPressed(sn.Mouse.RIGHT)
	aimBlend += ((if aiming then 1 else 0) - aimBlend) * (1 - math.exp(-15 * dt))

	local moveX = (if held(keys.D) then 1 else 0) - (if held(keys.A) then 1 else 0)
	local moveZ = (if held(keys.W) then 1 else 0) - (if held(keys.S) then 1 else 0)
	local speed = if aiming then 3.2 elseif held(keys.LSHIFT) then 8.0 else 5.3
	local magnitude = math.max(1, math.sqrt(moveX ^ 2 + moveZ ^ 2))
	local sinYaw, cosYaw = math.sin(yaw), math.cos(yaw)
	local targetX = (moveX * cosYaw + moveZ * sinYaw) * speed / magnitude
	local targetZ = (moveX * sinYaw - moveZ * cosYaw) * speed / magnitude
	local response = 1 - math.exp(-(if grounded then 20 else 6) * dt)
	velocityX += (targetX - velocityX) * response
	velocityZ += (targetZ - velocityZ) * response
	if grounded and pressed(keys.SPACE) then
		verticalSpeed = 6.6
	end
	verticalSpeed -= 17 * dt
	local before = feet
	local hitVertical: boolean
	feet, grounded, hitVertical = Collision.move(solids, feet, v3(velocityX * dt, verticalSpeed * dt, velocityZ * dt), 0.32, 1.76)
	if hitVertical then
		verticalSpeed = 0
	end
	local distance = v3(feet.x - before.x, 0, feet.z - before.z):length()
	stride += distance * 2.25
	sway += ((if grounded and distance > 0.003 then 1 - 0.65 * aimBlend else 0) - sway) * (1 - math.exp(-12 * dt))
end

local function spawnWave()
	wave += 1
	for index = 1, waveSizes[wave] do
		local dock = Arena.docks[(index + wave - 2) % #Arena.docks + 1]
		local x = dock.x + (if index > #Arena.docks then 1.45 else 0)
		local y = if index % 3 == 0 then 2.0 else 1.27
		local position = v3(x, y, dock.z)
		-- Never open a new wave directly on top of the player.
		if (position - eye):length() < 4.5 then
			position = v3(x, y, if eye.z > 0 then -13.5 else 14.4)
		end
		local center = position + v3(0, 0.44, 0)
		table.insert(drones, {
			feet = position, center = center, transform = Art.transform(center),
			hp = if wave == 3 and index % 2 == 0 then 3 else 2,
			cooldown = 0.65 + index * 0.21, charge = 0, phase = index * 1.94,
			side = if index % 2 == 0 then 1 else -1,
		})
	end
	playCue("wave")
end

local function emitSparks(position: sn.Vec3, count: number)
	for _ = 1, count do
		local velocity = v3(sn.Random.getRange(-3.6, 3.6), sn.Random.getRange(0.9, 4.5), sn.Random.getRange(-3.6, 3.6))
		local life = sn.Random.getRange(0.14, 0.40)
		table.insert(sparks, { transform = Art.transform(position, nil, v3(0.045)), velocity = velocity, life = life })
	end
end

local function startReload()
	if reload == 0 and charge < capacity and reserve > 0 then
		reload = reloadDuration
		playCue("reload")
	end
end

local function fire()
	if cooldown > 0 or reload > 0 then return end
	if charge == 0 then
		startReload()
		return
	end

	charge -= 1
	cooldown, flash, recoil = 0.165, 0.075, 1
	playCue("pulse")

	local distance = Collision.wallDistance(solids, eye, forward, 46)
	local target: number? = nil
	for index, drone in drones do
		local contact = Collision.sphereDistance(eye, forward, drone.center, droneRadius)
		if contact and contact < distance then
			target, distance = index, contact
		end
	end

	local impact = eye + forward * v3(distance)
	local right = forward:cross(up):normalize()
	local muzzle = eye + forward * v3(0.85) + right * v3(0.24 * (1 - aimBlend)) - up * v3(0.18)
	local beam = impact - muzzle
	local length = beam:length()
	if length > 0.005 then
		beamPose.position = muzzle + beam * v3(0.5)
		beamPose.scale = v3(0.019, 0.019, length)
		beamPose.rotation = v3(math.deg(math.atan2(-beam.y, beam.z)), math.deg(math.asin(math.clamp(beam.x / length, -1, 1))), 0)
	else
		beamPose.scale = v3(0)
	end
	if target then
		local drone = drones[target]
		drone.hp -= 1
		hit = 0.16
		emitSparks(impact, 4)
		if drone.hp == 0 then
			score += 100
			emitSparks(drone.center, 10)
			playCue("breakup")
			if score % 300 == 0 then
				local p = v3(drone.center.x, 0.54, drone.center.z)
				table.insert(pickups, { position = p, transform = Art.transform(p, nil, v3(0.13)) })
			end
			table.remove(drones, target)
		else
			playCue("hit")
		end
	elseif distance < 46 then
		emitSparks(impact, 2)
	end
end

local function updateDrones(dt: number)
	for _, drone in drones do
		local toEye = eye - drone.center
		local distance = toEye:length()
		local direction = if distance > 0.001 then toEye / v3(distance) else forward
		local sight = Collision.wallDistance(solids, drone.center, direction, distance) >= distance
		local flat = v3(direction.x, 0, direction.z)
		local side = v3(flat.z, 0, -flat.x) * v3(drone.side)
		local advance = if not sight or distance > 8.4 then 1 elseif distance < 3 then -0.7 else 0.12
		local steer = flat * v3(advance) + side * v3(if sight then 0.72 else 0.48)
		local speed = if drone.charge > 0 then 1.25 else 2.1 + wave * 0.36
		drone.feet = Collision.move(solids, drone.feet, steer * v3(dt * speed), 0.61, 0.88)
		drone.center = drone.feet + v3(0, 0.44 + 0.12 * math.sin(elapsed * 3.4 + drone.phase), 0)
		drone.transform.position = drone.center
		drone.transform.rotation = v3(0, math.deg(math.atan2(direction.x, direction.z)), math.sin(elapsed * 3.4 + drone.phase) * 7)

		drone.cooldown = math.max(0, drone.cooldown - dt)
		if drone.charge > 0 then
			drone.charge -= dt
			if drone.charge <= 0 and sight then
				local offset = eye - drone.center
				if offset:length() > 0.001 then
					local shot = offset:normalize()
					local start = drone.center + shot * v3(0.48)
					table.insert(bolts, { transform = Art.transform(start, nil, v3(0.093)), velocity = shot * v3(8.5 + wave), life = 3 })
				end
			end
		elseif drone.cooldown == 0 and sight and distance < 21 then
			drone.charge = 0.52
			drone.cooldown = 1.7 + (drone.phase % 1) * 1.15 - wave * 0.12
		end
	end
end

local function updateEffects(dt: number)
	for index = #bolts, 1, -1 do
		local bolt = bolts[index]
		local origin = bolt.transform.position
		local direction = bolt.velocity:normalize()
		local distance = bolt.velocity:length() * dt
		local wall = Collision.wallDistance(solids, origin, direction, distance)
		local body = Collision.sphereDistance(origin, direction, feet + v3(0, 1.03, 0), 0.62)
		local head = Collision.sphereDistance(origin, direction, eye, 0.29)
		local contact = math.min(body or math.huge, head or math.huge)
		bolt.life -= dt
		if contact <= wall then
			health = math.max(0, health - 13)
			damage = 0.44
			playCue("damage")
			table.remove(bolts, index)
		elseif wall < distance or bolt.life <= 0 then
			table.remove(bolts, index)
		else
			bolt.transform.position = origin + direction * v3(distance)
		end
	end
	for index = #sparks, 1, -1 do
		local spark = sparks[index]
		spark.life -= dt
		if spark.life <= 0 then
			table.remove(sparks, index)
		else
			spark.velocity -= up * v3(9 * dt)
			spark.transform.position += spark.velocity * v3(dt)
			spark.transform.scale = v3(math.min(0.05, spark.life * 0.24))
		end
	end
	for index = #pickups, 1, -1 do
		local pickup = pickups[index]
		pickup.transform.position = pickup.position + up * v3(0.12 * math.sin(elapsed * 4))
		if (feet + up * v3(0.55) - pickup.position):length() < 0.82 then
			health = math.min(100, health + 24)
			reserve = math.min(180, reserve + 24)
			playCue("pickup")
			table.remove(pickups, index)
		end
	end
end

function setup()
	sn.Window.rename("Sinen | FOUNDRY")
	sn.Graphics.setClearColor(color(0.035, 0.049, 0.053, 1))
	changeState("title")
	renderer:setExposure(1.2)
	local environment = "assets/small_workshop_1k.exr"
	renderer:setEnvironment(
		sn.Texture.newBRDFLUT(128, 128),
		sn.Texture.newIrradianceCubemap(environment, 24, 128),
		sn.Texture.newPrefilteredCubemap(environment, 64, 6, 96)
	)
	local materials = Art.materials()
	parts, solids = Arena.build(materials)
	models = Art.actors(materials)
	hud = Hud.new()
	for _, name in { "wave", "pulse", "reload", "hit", "breakup", "damage", "pickup" } do
		sounds[name] = sn.Sound.new("assets/" .. name .. ".wav")
		sounds[name]:setVolume(0.42)
	end
end

function ready()
	updateCamera(1)
	readyToDraw = true
end

function update()
	if not readyToDraw then return end
	local dt = math.clamp(sn.Time.delta(), 0, 1 / 20)
	if state ~= "playing" then
		if pressed(keys.Q) then
			if fromLauncher then sn.Script.load("/main.luau") else sn.Event.quit() end
			return
		end
		if pressed(keys.ENTER) or sn.Mouse.isPressed(sn.Mouse.LEFT) then
			if state == "paused" then changeState("playing") else restart() end
		elseif state == "paused" and pressed(keys.ESCAPE) then
			changeState("playing")
		end
		updateCamera(dt)
		return
	end
	if pressed(keys.ESCAPE) or not sn.Window.hasFocus() or not sn.Mouse.isRelative() then
		changeState("paused")
		return
	end

	elapsed += dt
	cooldown = math.max(0, cooldown - dt)
	flash = math.max(0, flash - dt)
	hit = math.max(0, hit - dt)
	damage = math.max(0, damage - dt)
	recoil *= math.exp(-15 * dt)
	if reload > 0 then
		reload = math.max(0, reload - dt)
		if reload == 0 then
			local amount = math.min(capacity - charge, reserve)
			charge += amount
			reserve -= amount
		end
	end
	movePlayer(dt)
	updateCamera(dt)
	if pressed(keys.R) then startReload() end
	if sn.Mouse.isPressed(sn.Mouse.LEFT) or sn.Mouse.isDown(sn.Mouse.LEFT) then fire() end

	updateDrones(dt)
	updateEffects(dt)
	if health == 0 then
		changeState("lost")
	elseif #drones == 0 then
		if wave == #waveSizes then
			changeState("won")
		else
			intermission -= dt
			if intermission <= 0 then
				if wave > 0 then
					health = math.min(100, health + 14)
					reserve = math.min(180, reserve + 18)
				end
				table.clear(bolts)
				spawnWave()
				intermission = 2.5
			end
		end
	end
end

function draw()
	if not readyToDraw then return end
	local world = sn.Graphics.begin3DPass(camera)
	renderer:setLight(keyLight)
	if flash > 0 then
		flashLight:setPosition(eye + forward * v3(0.8))
		flashLight:setIntensity(flash * 100)
		renderer:setFillLight(flashLight)
	else
		renderer:setFillLight(bayLight)
	end
	for _, part in parts do renderer:drawModel(world, part.model, part.transform) end
	for _, drone in drones do
		renderer:drawModel(world, models.drone, drone.transform)
		renderer:drawModel(world, if drone.charge > 0 then models.alertVisor else models.visor, drone.transform)
	end
	for _, bolt in bolts do renderer:drawModel(world, models.bolt, bolt.transform) end
	for _, spark in sparks do renderer:drawModel(world, models.sparks, spark.transform) end
	for _, pickup in pickups do renderer:drawModel(world, models.cell, pickup.transform) end
	if flash > 0 then renderer:drawModel(world, models.beam, beamPose) end
	sn.Graphics.endPass(world)

	if state == "playing" or state == "paused" then
		local view = sn.Graphics.begin3DPass(viewCamera)
		renderer:setLight(viewLight)
		renderer:clearFillLight()
		for _, model in models.emitter do renderer:drawModel(view, model, emitterPose) end
		sn.Graphics.endPass(view)
	end

	local reloadProgress = if reload > 0 then 1 - reload / reloadDuration else nil
	hud:draw(state, wave, #waveSizes, #drones, health, charge, reserve, capacity, score, intermission,
		reloadProgress, hit, damage, aiming, fromLauncher)
end
