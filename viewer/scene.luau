--!strict

local sn = require("@sinen")
local Components = require("./components")
if type(Components) ~= "table" then
	error("components load failed: " .. tostring(Components))
end

export type EntitySpec = {
	name: string,
	components: { [string]: any },
}

export type Definition = {
	entities: { EntitySpec },
}

export type SceneEntity = {
	entity: sn.Entity,
	name: string,
	components: { [string]: any },
}

local Scene = {}
local EntityBuilder = {}
EntityBuilder.__index = EntityBuilder
local DocumentMethods = {}
DocumentMethods.__index = DocumentMethods

local function deepCopy(value: any, seen: { [any]: any }?): any
	if type(value) ~= "table" then
		return value
	end
	local visited = seen or {}
	if visited[value] then
		return visited[value]
	end
	local copy = {}
	visited[value] = copy
	for key, child in pairs(value) do
		copy[deepCopy(key, visited)] = deepCopy(child, visited)
	end
	return copy
end

function EntityBuilder:with(componentName: string, value: any): any
	if Components.byName[componentName] == nil or componentName == "Name" then
		error("unknown scene component: " .. componentName, 2)
	end
	self._spec.components[componentName] = deepCopy(value)
	return self
end

function Scene.define(build: (any) -> ()): Definition
	local definition: Definition = { entities = {} }
	local builder = {}

	function builder:entity(name: string): any
		local spec: EntitySpec = {
			name = name,
			components = {},
		}
		table.insert(definition.entities, spec)
		return setmetatable({ _spec = spec }, EntityBuilder)
	end

	build(builder)
	return definition
end

local function spawn(document: any, name: string, values: { [string]: any }): SceneEntity
	local componentMap: any = {
		[Components.Name] = name,
	}
	local copiedValues: { [string]: any } = {}
	for componentName, value in pairs(values) do
		local component = Components.byName[componentName]
		if component == nil or componentName == "Name" then
			error("unknown scene component: " .. componentName, 3)
		end
		local copy = deepCopy(value)
		componentMap[component] = copy
		copiedValues[componentName] = copy
	end

	local entity = document.world:spawn(componentMap)
	local record: SceneEntity = {
		entity = entity,
		name = name,
		components = copiedValues,
	}
	table.insert(document.entities, record)
	document.byIndex[entity.index] = record
	return record
end

function Scene.instantiate(definition: Definition): any
	if type(definition) ~= "table" or type(definition.entities) ~= "table" then
		local entitiesType = if type(definition) == "table" then type((definition :: any).entities) else "n/a"
		error("invalid scene definition " .. type(definition) .. "/" .. entitiesType .. ": " .. tostring(definition))
	end
	local document = setmetatable({
		world = sn.ECS.world(),
		entities = {},
		byIndex = {},
	}, DocumentMethods)
	for _, spec in ipairs(definition.entities) do
		spawn(document, spec.name, spec.components)
	end
	return document
end

function DocumentMethods:add(name: string, values: { [string]: any }?): SceneEntity
	return spawn(self, name, values or {})
end

function DocumentMethods:remove(entity: sn.Entity): boolean
	local record = self.byIndex[entity.index]
	if record == nil or not self.world:despawn(entity) then
		return false
	end
	self.byIndex[entity.index] = nil
	local index = table.find(self.entities, record)
	if index then
		table.remove(self.entities, index)
	end
	return true
end

function DocumentMethods:addComponent(record: SceneEntity, componentName: string): any
	local component = Components.byName[componentName]
	local default = Components.defaults[componentName]
	if component == nil or default == nil then
		error("component cannot be added: " .. componentName, 2)
	end
	if record.components[componentName] ~= nil then
		return record.components[componentName]
	end
	local value = deepCopy(default)
	record.components[componentName] = value
	self.world:insert(record.entity, component, value)
	return value
end

function DocumentMethods:removeComponent(record: SceneEntity, componentName: string): boolean
	local component = Components.byName[componentName]
	if component == nil or record.components[componentName] == nil then
		return false
	end
	record.components[componentName] = nil
	self.world:remove(record.entity, component)
	return true
end

local function isIdentifier(value: string): boolean
	return string.match(value, "^[%a_][%w_]*$") ~= nil
end

local function isArray(value: { [any]: any }): (boolean, number)
	local count = 0
	local maximum = 0
	for key in pairs(value) do
		if type(key) ~= "number" or key < 1 or key % 1 ~= 0 then
			return false, 0
		end
		count += 1
		maximum = math.max(maximum, key)
	end
	return count == maximum, maximum
end

local function serializeValue(value: any, indent: string): string
	local valueType = type(value)
	if valueType == "string" then
		return string.format("%q", value)
	elseif valueType == "number" then
		if value ~= value or value == math.huge or value == -math.huge then
			error("scene numbers must be finite")
		end
		return string.format("%.9g", value)
	elseif valueType == "boolean" or valueType == "nil" then
		return tostring(value)
	elseif valueType ~= "table" then
		error("scene values must contain only serializable Luau data")
	end

	local array, length = isArray(value)
	if array then
		local parts = {}
		for index = 1, length do
			table.insert(parts, serializeValue(value[index], indent))
		end
		return "{ " .. table.concat(parts, ", ") .. " }"
	end

	local keys = {}
	for key in pairs(value) do
		if type(key) ~= "string" then
			error("scene object keys must be strings")
		end
		table.insert(keys, key)
	end
	table.sort(keys)
	if #keys == 0 then
		return "{}"
	end

	local childIndent = indent .. "\t"
	local lines = { "{" }
	for _, key in ipairs(keys) do
		local encodedKey = if isIdentifier(key) then key else "[" .. string.format("%q", key) .. "]"
		table.insert(lines, childIndent .. encodedKey .. " = " .. serializeValue(value[key], childIndent) .. ",")
	end
	table.insert(lines, indent .. "}")
	return table.concat(lines, "\n")
end

function Scene.serialize(document: any, sceneRequirePath: string?): string
	local lines = {
		"--!strict",
		"",
		"-- Generated by the Sinen ECS editor. This is ordinary Luau and may be edited by hand.",
		"local Scene = require(" .. string.format("%q", sceneRequirePath or "../scene") .. ")",
		"",
		"return Scene.define(function(scene)",
	}
	for _, record in ipairs(document.entities) do
		table.insert(lines, "\tscene:entity(" .. string.format("%q", record.name) .. ")")
		local componentNames = {}
		for componentName in pairs(record.components) do
			table.insert(componentNames, componentName)
		end
		table.sort(componentNames)
		for _, componentName in ipairs(componentNames) do
			table.insert(
				lines,
				"\t\t:with("
					.. string.format("%q", componentName)
					.. ", "
					.. serializeValue(record.components[componentName], "\t\t")
					.. ")"
			)
		end
		table.insert(lines, "")
	end
	table.insert(lines, "end)")
	table.insert(lines, "")
	return table.concat(lines, "\n")
end

function Scene.write(path: string, document: any, sceneRequirePath: string?): boolean
	return sn.Filesystem.writeText(path, Scene.serialize(document, sceneRequirePath))
end

function Scene.readSource(path: string): string?
	return sn.Filesystem.readText(path)
end

return Scene
