local sn = require("@sinen")

local camera: sn.Camera2D = sn.Camera2D.new()
camera:resize(sn.Vec2.new(32, 18))

local world: sn.World2D = sn.World2D.new()
world:setGravity(sn.Vec3.new(0, 18, 0))

type Body = {
	collider: sn.Collider,
	size: sn.Vec2,
	color: sn.Color,
}

local bodies: { Body } = {}
local spawnTimer = 0.0
local spawnIndex = 0

local function newTransform(x: number, y: number, w: number, h: number, angle: number): sn.Transform
	local transform = sn.Transform.new()
	transform.position = sn.Vec3.new(x, y, 0)
	transform.rotation = sn.Vec3.new(0, 0, angle)
	transform.scale = sn.Vec3.new(w * 0.5, h * 0.5, 1)
	return transform
end

local function addBox(x: number, y: number, w: number, h: number, angle: number, isStatic: boolean, color: sn.Color)
	local collider = world:newBoxCollider(newTransform(x, y, w, h, angle), isStatic)
	world:addCollider(collider, not isStatic)
	table.insert(bodies, {
		collider = collider,
		size = sn.Vec2.new(w, h),
		color = color,
	})
end

local function addCircle(x: number, y: number, radius: number, color: sn.Color)
	local collider = world:newCircleCollider(sn.Vec3.new(x, y, 0), radius, false)
	world:addCollider(collider, true)
	table.insert(bodies, {
		collider = collider,
		size = sn.Vec2.new(radius * 2, radius * 2),
		color = color,
	})
end

local function addCapsule(x: number, y: number, halfHeight: number, radius: number, angle: number, color: sn.Color)
	local collider = world:newCapsuleCollider(sn.Vec3.new(x, y, 0), sn.Vec3.new(0, 0, angle), halfHeight, radius, false)
	world:addCollider(collider, true)
	table.insert(bodies, {
		collider = collider,
		size = sn.Vec2.new(radius * 2, halfHeight * 2 + radius * 2),
		color = color,
	})
end

local function spawnBody()
	local x = 7.0 + math.random() * 18.0
	local angle = math.random(-35, 35)
	spawnIndex += 1

	if spawnIndex % 3 == 1 then
		addBox(x, 1.0, 1.2, 1.2, angle, false, sn.Color.new(0.20, 0.70, 1.00, 1.0))
	elseif spawnIndex % 3 == 2 then
		addCircle(x, 1.0, 0.65, sn.Color.new(1.00, 0.38, 0.28, 1.0))
	else
		addCapsule(x, 1.0, 0.75, 0.28, angle, sn.Color.new(0.42, 0.92, 0.56, 1.0))
	end
end

addBox(16.0, 16.8, 24.0, 0.7, -4.0, true, sn.Color.new(0.85, 0.86, 0.78, 1.0))
addBox(4.0, 10.5, 6.5, 0.45, 16.0, true, sn.Color.new(0.76, 0.78, 0.88, 1.0))
addBox(25.5, 12.4, 6.5, 0.45, -16.0, true, sn.Color.new(0.76, 0.78, 0.88, 1.0))
spawnBody()
spawnBody()

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/physics/main.luau")
	end

	spawnTimer += sn.Time.delta()
	if spawnTimer > 0.65 or sn.Mouse.isPressed(sn.Mouse.LEFT) then
		spawnTimer = 0.0
		spawnBody()
	end

	world:update(sn.Time.delta(), 4)

	for i = #bodies, 1, -1 do
		local body = bodies[i]
		local position = body.collider:getPosition()
		if position.y > 24.0 then
			body.collider:destroy()
			table.remove(bodies, i)
		end
	end
end

function draw()
	sn.Graphics.begin2D(camera)
	sn.Graphics.drawRect(sn.Rect.new(0, 0, 32, 18), sn.Color.new(0.05, 0.06, 0.08, 1.0))

	for _, body in ipairs(bodies) do
		local position = body.collider:getPosition()
		local rotation = body.collider:getRotation()
		local rect = sn.Rect.new(sn.Pivot.Center, position.x, position.y, body.size.x, body.size.y)
		sn.Graphics.drawRect(rect, body.color, rotation.z)
	end

	sn.Graphics.finish()
end
