local Track = require("./track")
local Car = require("./car")
local Grid = {}

export type Driver = { car: Car.State, lane: number, pace: number, finish: number? }
export type Input = { gas: number, brake: number, steer: number }

local function angle(x: number): number
	return (x + math.pi) % (2 * math.pi) - math.pi
end

function Grid.new(): { Driver }
	local field = {}
	for i = 1, 6 do
		local slot = if i == 1 then 6 else i - 1
		local lane = if slot % 2 == 0 then 2.3 else -2.3
		field[i] = {
			car = Car.new(-14 - math.floor((slot - 1) / 2) * 14 + (if lane > 0 then -4 else 0), lane),
			lane = (i % 3 - 1) * 3.2,
			pace = 0.89 + (i % 4) * 0.037,
			finish = nil,
		}
	end
	return field
end

function Grid.drive(driver: Driver): Input
	local car = driver.car
	local lookahead = 7.5 + car.speed * 0.36
	local lane = driver.lane * 0.75 + math.sin(car.progress * 0.011 + driver.lane) * 0.9
	local target = Track.at(car.lastS + lookahead, lane)
	local desired = math.atan2(target.x - car.x, car.z - target.z)
	local steer = math.clamp(angle(desired - car.yaw) * (2.4 + car.speed * 0.036), -1, 1)

	local speed = 55 * driver.pace
	local heading = math.atan2(target.tx, -target.tz)
	for ahead = 10, 110, 10 do
		local a = Track.at(car.lastS + ahead - 4)
		local b = Track.at(car.lastS + ahead + 4)
		local curvature = math.abs(angle(math.atan2(b.tx, -b.tz) - math.atan2(a.tx, -a.tz))) / 8
		if curvature > 0.005 then
			local corner = math.sqrt(8.2 / curvature) * driver.pace
			speed = math.min(speed, math.sqrt(corner ^ 2 + 2 * 6.8 * math.max(0, ahead - 6)))
		end
	end
	speed = math.min(speed, 20 + 35 * math.max(0, 1 - math.abs(angle(heading - car.yaw))))
	local difference = speed - car.speed
	return {
		gas = math.clamp(difference * 0.38 + 0.21, 0, 1),
		brake = math.clamp(-difference * 0.26, 0, 0.94),
		steer = steer,
	}
end

function Grid.resolve(field: { Driver })
	for i = 1, #field - 1 do
		local a = field[i].car
		for j = i + 1, #field do
			local b = field[j].car
			local dx, dz = b.x - a.x, b.z - a.z
			if math.abs(dx) > 4.5 or math.abs(dz) > 4.5 then continue end
			-- Two circles approximate each car's footprint.
			local ax, az = math.sin(a.yaw), -math.cos(a.yaw)
			local bx, bz = math.sin(b.yaw), -math.cos(b.yaw)
			for _, u in { -1, 1 } do
				for _, v in { -1, 1 } do
					local offsetX = b.x + bx * v - a.x - ax * u
					local offsetZ = b.z + bz * v - a.z - az * u
					local distance = math.sqrt(offsetX ^ 2 + offsetZ ^ 2)
					if distance >= 1.72 then continue end
					local nx, nz = if distance > 1e-5 then offsetX / distance else 1, if distance > 1e-5 then offsetZ / distance else 0
					local separation = (1.72 - distance) * 0.5 + 0.001
					a.x, a.z = a.x - nx * separation, a.z - nz * separation
					b.x, b.z = b.x + nx * separation, b.z + nz * separation
					local approach = (a.vx - b.vx) * nx + (a.vz - b.vz) * nz
					local impulse = math.max(0, approach) * 0.56
					a.vx, a.vz = a.vx - nx * impulse, a.vz - nz * impulse
					b.vx, b.vz = b.vx + nx * impulse, b.vz + nz * impulse
				end
			end
		end
	end
end

local function raceDistance(car: Car.State): number
	return math.min(car.progress, car.nextGate * Track.gateSpacing)
end

function Grid.position(field: { Driver }): number
	local player = field[1]
	local position = 1
	for index = 2, #field do
		local rival = field[index]
		if (rival.finish and (not player.finish or rival.finish < player.finish))
			or (not player.finish and raceDistance(rival.car) > raceDistance(player.car)) then
			position += 1
		end
	end
	return position
end

return Grid
