local boxTex: Texture = Texture.new()
boxTex:fill(Color.new(1, 1, 1, 1))

local sphereTex: Texture = Texture.new()
sphereTex:fill(Color.new(1, 0, 0, 1))

local box: Model = Model.new()
box:loadBox()
box:setTexture(TextureKey.BaseColor, boxTex)

local sphere: Model = Model.new()
sphere:load("sphere.glb")
sphere:setTexture(TextureKey.BaseColor, sphereTex)

local boxTransform: Transform = Transform.new()
boxTransform.rotation = Vec3.new(0, 0, 10)
boxTransform.scale = Vec3.new(20, 1, 20)
local sphereTransform: Transform = Transform.new()
sphereTransform.position = Vec3.new(0, 2, 0)
sphereTransform.scale = Vec3.new(0.5)

local colliders: { Collider } = {}

local world: World = World.new()
local floorCollider = world:createBoxCollider(boxTransform, true)
local sphereCollider = world:createSphereCollider(sphereTransform.position, 0.5, false)
sphereCollider:setLinearVelocity(Vec3.new(0, -5.0, 0))
world:addCollider(floorCollider, false)
world:addCollider(sphereCollider, true)

Graphics.getCamera3D():lookat(Vec3.new(0, 10, 25), Vec3.new(0, 0, 0), Vec3.new(0, 1, 0))

function update()
	if Keyboard.isPressed(Keyboard.ESCAPE) then
		Script.load("main", ".")
	end
	boxTransform.position = floorCollider:getPosition()
	sphereTransform.position = sphereCollider:getPosition()

	if Mouse.isPressed(Mouse.LEFT) then
		local sc: Collider = world:createSphereCollider(Vec3.new(0, 2, 0), 0.5, false)
		sc:setLinearVelocity(Vec3.new(math.random(-5, 5), math.random(-5, 5), math.random(-5, 5)))
		world:addCollider(sc, true)
		table.insert(colliders, sc)
	end
end

function draw()
	Graphics.drawModel(box, boxTransform)

	Graphics.drawModel(sphere, sphereTransform)
	for i, c in ipairs(colliders) do
		local transform = Transform.new()
		transform.position = c:getPosition()
		if transform.position.y < -10.0 then
			table.remove(colliders, i)
		else
			transform.scale = Vec3.new(0.5)
			Graphics.drawModel(sphere, transform)
		end
	end
end
