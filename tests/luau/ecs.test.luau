--!strict

local sn = require("@sinen")
local ECS = sn.ECS
local test = require("./sinen_test")

local describe = test.describe
local it = test.it
local expect = test.expect

type Position = { x: number, y: number }
type Velocity = { x: number, y: number }

local Position: sn.Component<Position> = ECS.component("Position")
local Velocity: sn.Component<Velocity> = ECS.component("Velocity")
local Name: sn.Component<string> = ECS.component("Name")

describe("ECS", function()
	it("creates, updates, removes, and recycles entities", function()
		local world = ECS.world()
		local entity = world:spawn({
			[Position] = { x = 1, y = 2 },
			[Name] = "player",
		})

		expect(world:isAlive(entity)):toBeTruthy()
		expect(world:get(entity, Name)):toBe("player")
		world:insert(entity, Velocity, { x = 3, y = 4 })
		expect(world:has(entity, Velocity)):toBeTruthy()
		expect((world:remove(entity, Name) :: string)):toBe("player")
		expect(world:has(entity, Name)):toBeFalsy()

		expect(world:despawn(entity)):toBeTruthy()
		expect(world:isAlive(entity)):toBeFalsy()
		local replacement = world:spawn()
		expect(replacement.index):toBe(entity.index)
		expect(replacement.generation):toBe(entity.generation + 1)
		expect(world:isAlive(entity)):toBeFalsy()
	end)

	it("queries matching entities in stable index order", function()
		local world = ECS.world()
		local first = world:spawn({
			[Position] = { x = 1, y = 2 },
			[Velocity] = { x = 10, y = 20 },
		})
		world:spawn({ [Position] = { x = 3, y = 4 } })
		local third = world:spawn({
			[Position] = { x = 5, y = 6 },
			[Velocity] = { x = 30, y = 40 },
		})

		local found = {}
		for entity, position, velocity in world:query(Position, Velocity) do
			table.insert(found, entity)
			position.x += velocity.x
		end

		expect(found):toEqual({ first, third })
		expect((world:get(first, Position) :: Position).x):toBe(11)
		expect(world:count(Position)):toBe(3)
		expect(world:count(Position, Velocity)):toBe(2)
	end)

	it("allows structural changes while iterating", function()
		local world = ECS.world()
		for index = 1, 4 do
			world:spawn({ [Position] = { x = index, y = 0 } })
		end

		world:each({ Position }, function(entity)
			if entity.index % 2 == 0 then
				world:despawn(entity)
			end
		end)
		expect(world:count(Position)):toBe(2)
	end)

	it("runs systems in order", function()
		local world = ECS.world()
		local entity = world:spawn({ [Position] = { x = 0, y = 0 } })
		local order = {}

		world:addSystem(function(currentWorld, deltaTime)
			table.insert(order, "move")
			currentWorld:each({ Position }, function(_, position)
				position.x += deltaTime
			end)
		end)
		world:addSystem(function()
			table.insert(order, "finish")
		end)

		world:update(0.5)
		expect(order):toEqual({ "move", "finish" })
		expect((world:get(entity, Position) :: Position).x):toBeCloseTo(0.5)
	end)
end)

return true
