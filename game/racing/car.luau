local Track = require("./track")

local Car = {}
local gravity = 9.81
local wheelbase = 2.65

export type State = {
	x: number, z: number, vx: number, vz: number, yaw: number, yawRate: number,
	steer: number, speed: number, gear: number, rpm: number, spin: number,
	progress: number, lastS: number, nextGate: number, lap: number, finished: boolean,
	margin: number, longitudinal: number, lateral: number, brake: number,
}

function Car.new(s: number, lane: number): State
	local frame = Track.at(s, lane)
	return {
		x = frame.x, z = frame.z, vx = 0, vz = 0, yaw = math.atan2(frame.tx, -frame.tz), yawRate = 0,
		steer = 0, speed = 0, gear = 1, rpm = 1050, spin = 0,
		progress = s, lastS = frame.s, nextGate = 0, lap = 0, finished = false,
		margin = lane, longitudinal = 0, lateral = 0, brake = 0,
	}
end

local ratios = { 3.12, 2.13, 1.58, 1.22, 0.99, 0.82 }
local function engineRpm(speed: number, gear: number): number
	return math.max(1050, math.abs(speed) * 60 / (2 * math.pi * 0.33) * ratios[gear] * 3.58)
end

-- Planar bicycle model. Tire forces saturate together as longitudinal demand rises.
function Car.step(car: State, throttle: number, brake: number, steer: number, dt: number)
	local fx, fz = math.sin(car.yaw), -math.cos(car.yaw)
	local rx, rz = -fz, fx
	local forward = car.vx * fx + car.vz * fz
	local sideways = car.vx * rx + car.vz * rz
	local speed = math.sqrt(car.vx ^ 2 + car.vz ^ 2)
	local grip = if math.abs(car.margin) > Track.halfWidth + 0.55 then 0.58 else 1.12
	local maxSteer = 0.56 / (1 + (speed / 21) ^ 1.42)
	car.steer += (steer * maxSteer - car.steer) * (1 - math.exp(-10 * dt))
	local rpm = engineRpm(forward, car.gear)
	if rpm > 6850 and car.gear < #ratios then
		car.gear += 1
	elseif rpm < 3100 and car.gear > 1 then
		car.gear -= 1
	end
	car.rpm += (math.min(7500, engineRpm(forward, car.gear) + throttle * 450) - car.rpm) * (1 - math.exp(-8 * dt))

	local reverse = brake > 0 and forward < 0.5 and speed < 7 and throttle == 0
	local resistance = (if grip < 1 then 0.75 else 0.16) + 0.00175 * speed ^ 2
	local engine = throttle * (3.5 + ratios[car.gear] * 1.22) * (1 - 0.33 * (math.min(7500, rpm) / 7400) ^ 2)
	local rolling = math.sign(forward) * math.min(resistance, math.abs(forward) / dt)
	local stopping = if reverse then 2.2 * brake else math.sign(forward) * math.min(brake * 12, math.abs(forward) / dt)
	local drive = math.clamp(engine - stopping - rolling, -grip * gravity, grip * gravity)
	local tireBudget = math.sqrt(math.max(0.12, (grip * gravity) ^ 2 - (drive * 0.84) ^ 2))
	local steerRate = forward * math.tan(car.steer) / wheelbase
	local turnLimit = tireBudget / math.max(2, math.abs(forward))
	local targetYawRate = math.clamp(steerRate, -turnLimit, turnLimit)
	car.yawRate += (targetYawRate - car.yawRate) * (1 - math.exp(-8 * dt))
	local sideForce = math.clamp(car.yawRate * forward - sideways * 6, -tireBudget, tireBudget)
	car.vx += (fx * drive + rx * sideForce) * dt
	car.vz += (fz * drive + rz * sideForce) * dt
	car.x += car.vx * dt
	car.z += car.vz * dt
	car.yaw = (car.yaw + car.yawRate * dt) % (2 * math.pi)
	car.speed = math.sqrt(car.vx ^ 2 + car.vz ^ 2)
	car.spin = (car.spin + forward * dt / 0.33) % (2 * math.pi)
	car.longitudinal, car.lateral, car.brake = drive, sideForce, brake
end

function Car.updateProgress(car: State, totalLaps: number)
	local frame, lateral = Track.project(car.x, car.z)
	car.margin = lateral
	if math.abs(lateral) > Track.wall - 0.9 then
		local side = math.sign(lateral)
		local boundary = Track.at(frame.s, side * (Track.wall - 0.9))
		car.x, car.z = boundary.x, boundary.z
		local tangent = (car.vx * frame.tx + car.vz * frame.tz) * 0.72
		local normal = math.max(0, (car.vx * -frame.tz + car.vz * frame.tx) * side)
		car.vx = frame.tx * tangent + frame.tz * normal * side * 0.2
		car.vz = frame.tz * tangent - frame.tx * normal * side * 0.2
	end
	local delta = (frame.s - car.lastS + Track.length / 2) % Track.length - Track.length / 2
	local previous = car.progress
	car.progress += delta
	car.lastS = frame.s
	local gate = car.nextGate * Track.gateSpacing
	-- Runoff within the barriers still crosses a timing sector.
	if not car.finished and previous < gate and car.progress >= gate then
		car.nextGate += 1
		if gate > 0 and (car.nextGate - 1) % Track.gates == 0 then
			car.lap += 1
			car.finished = car.lap >= totalLaps
		end
	end
end

function Car.reset(car: State)
	-- Return to the last valid section if a checkpoint was missed.
	local nextGate = car.nextGate * Track.gateSpacing
	local distance = math.min(car.progress, nextGate - 5)
	local frame = Track.at(distance)
	car.x, car.z, car.yaw = frame.x, frame.z, math.atan2(frame.tx, -frame.tz)
	car.progress, car.lastS, car.margin = distance, frame.s, 0
	car.vx, car.vz, car.speed, car.steer, car.yawRate = 0, 0, 0, 0, 0
end

return Car
