--!strict

-- A small, dependency-free ECS for Sinen projects.

export type Entity = {
	index: number,
	generation: number,
}

export type Component<T> = {
	name: string,
	__component: T?,
}

export type ComponentMap = { [Component<any>]: any }
export type System = (world: World, deltaTime: number) -> ()
export type QueryIterator = () -> (Entity?, ...any)

export type World = {
	spawn: (self: World, components: ComponentMap?) -> Entity,
	despawn: (self: World, entity: Entity) -> boolean,
	isAlive: (self: World, entity: Entity) -> boolean,
	insert: <T>(self: World, entity: Entity, component: Component<T>, value: T) -> T,
	remove: <T>(self: World, entity: Entity, component: Component<T>) -> T?,
	has: <T>(self: World, entity: Entity, component: Component<T>) -> boolean,
	get: <T>(self: World, entity: Entity, component: Component<T>) -> T?,
	query: (self: World, ...Component<any>) -> QueryIterator,
	each: (self: World, components: { Component<any> }, callback: (Entity, ...any) -> ()) -> (),
	count: (self: World, ...Component<any>) -> number,
	addSystem: (self: World, system: System) -> System,
	removeSystem: (self: World, system: System) -> boolean,
	update: (self: World, deltaTime: number) -> (),
	clear: (self: World) -> (),
}

type Storage = {
	data: { [number]: any },
	size: number,
}

type WorldImpl = World & {
	_generations: { [number]: number },
	_alive: { [number]: boolean },
	_entities: { [number]: Entity },
	_free: { number },
	_nextIndex: number,
	_storages: { [Component<any>]: Storage },
	_systems: { System },
}

local ECS = {}
local WorldMethods = {}
WorldMethods.__index = WorldMethods

local nextComponentId = 0

local function assertComponent(component: Component<any>)
	if type(component) ~= "table" or (component :: any).__ecsComponent ~= true then
		error("expected an ECS component", 3)
	end
end

local function assertAlive(world: WorldImpl, entity: Entity)
	if not world:isAlive(entity) then
		error("entity is not alive", 3)
	end
end

local function getStorage(world: WorldImpl, component: Component<any>, create: boolean): Storage?
	local storage = world._storages[component]
	if storage == nil and create then
		storage = {
			data = {},
			size = 0,
		}
		world._storages[component] = storage
	end
	return storage
end

function ECS.component<T>(name: string?): Component<T>
	nextComponentId += 1
	local componentName = name or ("Component" .. tostring(nextComponentId))
	local component = {
		name = componentName,
		__ecsComponent = true,
		__component = nil,
	}
	return table.freeze(component) :: any
end

function ECS.world(): World
	local world: WorldImpl = setmetatable({
		_generations = {},
		_alive = {},
		_entities = {},
		_free = {},
		_nextIndex = 1,
		_storages = {},
		_systems = {},
	}, WorldMethods) :: any
	return world
end

function WorldMethods.spawn(self: WorldImpl, components: ComponentMap?): Entity
	local freeCount = #self._free
	local index: number
	if freeCount > 0 then
		index = self._free[freeCount]
		table.remove(self._free, freeCount)
	else
		index = self._nextIndex
		self._nextIndex += 1
	end

	local generation = self._generations[index] or 1
	local entity = table.freeze({
		index = index,
		generation = generation,
	}) :: Entity
	self._generations[index] = generation
	self._alive[index] = true
	self._entities[index] = entity

	if components then
		for component, value in pairs(components) do
			self:insert(entity, component, value)
		end
	end
	return entity
end

function WorldMethods.isAlive(self: WorldImpl, entity: Entity): boolean
	if type(entity) ~= "table" then
		return false
	end
	local index = entity.index
	return type(index) == "number" and self._alive[index] == true and self._generations[index] == entity.generation
end

function WorldMethods.despawn(self: WorldImpl, entity: Entity): boolean
	if not self:isAlive(entity) then
		return false
	end

	local index = entity.index
	for _, storage in pairs(self._storages) do
		if storage.data[index] ~= nil then
			storage.data[index] = nil
			storage.size -= 1
		end
	end

	self._alive[index] = nil
	self._entities[index] = nil
	self._generations[index] = entity.generation + 1
	table.insert(self._free, index)
	return true
end

function WorldMethods.insert<T>(self: WorldImpl, entity: Entity, component: Component<T>, value: T): T
	assertAlive(self, entity)
	assertComponent(component)
	if value == nil then
		error("component value cannot be nil", 2)
	end

	local storage = getStorage(self, component, true) :: Storage
	if storage.data[entity.index] == nil then
		storage.size += 1
	end
	storage.data[entity.index] = value
	return value
end

function WorldMethods.remove<T>(self: WorldImpl, entity: Entity, component: Component<T>): T?
	assertAlive(self, entity)
	assertComponent(component)
	local storage = getStorage(self, component, false)
	if storage == nil then
		return nil
	end

	local value = storage.data[entity.index]
	if value ~= nil then
		storage.data[entity.index] = nil
		storage.size -= 1
	end
	return value
end

function WorldMethods.has<T>(self: WorldImpl, entity: Entity, component: Component<T>): boolean
	if not self:isAlive(entity) then
		return false
	end
	assertComponent(component)
	local storage = getStorage(self, component, false)
	return storage ~= nil and storage.data[entity.index] ~= nil
end

function WorldMethods.get<T>(self: WorldImpl, entity: Entity, component: Component<T>): T?
	if not self:isAlive(entity) then
		return nil
	end
	assertComponent(component)
	local storage = getStorage(self, component, false)
	return if storage then storage.data[entity.index] else nil
end

function WorldMethods.query(self: WorldImpl, ...: Component<any>): QueryIterator
	local components = { ... }
	local storages: { Storage } = {}
	for _, component in ipairs(components) do
		assertComponent(component)
		local storage = getStorage(self, component, false)
		if storage == nil then
			return function(): (Entity?, ...any)
				return nil
			end
		end
		table.insert(storages, storage)
	end

	local indices: { number } = {}
	if #storages == 0 then
		for index in pairs(self._alive) do
			table.insert(indices, index)
		end
	else
		local smallest = storages[1]
		for index = 2, #storages do
			if storages[index].size < smallest.size then
				smallest = storages[index]
			end
		end
		for index in pairs(smallest.data) do
			table.insert(indices, index)
		end
	end
	table.sort(indices)

	local cursor = 0
	return function(): (Entity?, ...any)
		while true do
			cursor += 1
			local index = indices[cursor]
			if index == nil then
				return nil
			end

			local entity = self._entities[index]
			if entity ~= nil then
				local values = table.create(#storages)
				local matches = true
				for storageIndex, storage in ipairs(storages) do
					local value = storage.data[index]
					if value == nil then
						matches = false
						break
					end
					values[storageIndex] = value
				end
				if matches then
					return entity, table.unpack(values, 1, #storages)
				end
			end
		end
	end
end

function WorldMethods.each(self: WorldImpl, components: { Component<any> }, callback: (Entity, ...any) -> ())
	local iterator = self:query(table.unpack(components))
	while true do
		local values = { iterator() }
		local entity = values[1]
		if entity == nil then
			break
		end
		callback(table.unpack(values))
	end
end

function WorldMethods.count(self: WorldImpl, ...: Component<any>): number
	local count = 0
	for _ in self:query(...) do
		count += 1
	end
	return count
end

function WorldMethods.addSystem(self: WorldImpl, system: System): System
	if type(system) ~= "function" then
		error("system must be a function", 2)
	end
	table.insert(self._systems, system)
	return system
end

function WorldMethods.removeSystem(self: WorldImpl, system: System): boolean
	for index, current in ipairs(self._systems) do
		if current == system then
			table.remove(self._systems, index)
			return true
		end
	end
	return false
end

function WorldMethods.update(self: WorldImpl, deltaTime: number)
	local systems = table.clone(self._systems)
	for _, system in ipairs(systems) do
		-- A removed system is skipped, while systems added during an update start
		-- on the next update.
		if table.find(self._systems, system) ~= nil then
			system(self, deltaTime)
		end
	end
end

function WorldMethods.clear(self: WorldImpl)
	local entities = table.clone(self._entities)
	for _, entity in pairs(entities) do
		self:despawn(entity)
	end
end

ECS.World = {
	new = ECS.world,
}

return table.freeze(ECS)
