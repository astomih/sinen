--!strict

export type TestFn = () -> ()
export type LogFn = (message: string) -> ()
export type RunOptions = {
	logInfo: LogFn?,
	logError: LogFn?,
}
export type TestResult = {
	name: string,
	ok: boolean,
	message: string?,
}
export type RunResult = {
	total: number,
	passed: number,
	failed: number,
	ok: boolean,
	lines: { string },
	results: { TestResult },
}

type TestCase = {
	name: string,
	fn: TestFn,
}

local Framework: any = {}
local tests: { TestCase } = {}
local suiteStack: { string } = {}

local function traceback(message: any): string
	local debugLib = (debug :: any)
	if type(debugLib) == "table" and type(debugLib.traceback) == "function" then
		return debugLib.traceback(tostring(message), 2)
	end
	return tostring(message)
end

local function fullName(name: string): string
	if #suiteStack == 0 then
		return name
	end

	local parts: { string } = {}
	for _, part in ipairs(suiteStack) do
		table.insert(parts, part)
	end
	table.insert(parts, name)
	return table.concat(parts, " > ")
end

local function formatValue(value: any, depth: number?, seen: { [any]: boolean }?): string
	local valueType = type(value)
	if valueType == "string" then
		return string.format("%q", value)
	end
	if valueType ~= "table" then
		return tostring(value)
	end

	local currentDepth = depth or 0
	if currentDepth >= 3 then
		return "{...}"
	end

	local visited = seen or {}
	if visited[value] then
		return "{<cycle>}"
	end
	visited[value] = true

	local parts: { string } = {}
	for key, child in pairs(value) do
		table.insert(parts, formatValue(key, currentDepth + 1, visited) .. " = " .. formatValue(child, currentDepth + 1, visited))
	end
	table.sort(parts)

	visited[value] = nil
	return "{" .. table.concat(parts, ", ") .. "}"
end

local function deepEqual(a: any, b: any, visited: { [any]: any }?): boolean
	if a == b then
		return true
	end

	if type(a) ~= type(b) then
		return false
	end

	if type(a) ~= "table" then
		return false
	end

	local seen = visited or {}
	if seen[a] == b then
		return true
	end
	seen[a] = b

	for key, value in pairs(a) do
		if not deepEqual(value, b[key], seen) then
			return false
		end
	end

	for key in pairs(b) do
		if a[key] == nil then
			return false
		end
	end

	return true
end

local function fail(message: string): never
	error(message, 3)
end

local function check(pass: boolean, inverted: boolean, positiveMessage: string, negativeMessage: string)
	if inverted then
		if pass then
			fail(negativeMessage)
		end
	elseif not pass then
		fail(positiveMessage)
	end
end

local function makeExpect(actual: any, inverted: boolean, includeInverse: boolean?): any
	local expectation: any = {}

	function expectation:toBe(expected: any)
		check(
			actual == expected,
			inverted,
			"expected " .. formatValue(actual) .. " to be " .. formatValue(expected),
			"expected " .. formatValue(actual) .. " not to be " .. formatValue(expected)
		)
	end

	function expectation:toEqual(expected: any)
		check(
			deepEqual(actual, expected),
			inverted,
			"expected " .. formatValue(actual) .. " to equal " .. formatValue(expected),
			"expected " .. formatValue(actual) .. " not to equal " .. formatValue(expected)
		)
	end

	function expectation:toBeTruthy()
		check(
			actual ~= nil and actual ~= false,
			inverted,
			"expected " .. formatValue(actual) .. " to be truthy",
			"expected " .. formatValue(actual) .. " not to be truthy"
		)
	end

	function expectation:toBeFalsy()
		check(
			actual == nil or actual == false,
			inverted,
			"expected " .. formatValue(actual) .. " to be falsy",
			"expected " .. formatValue(actual) .. " not to be falsy"
		)
	end

	function expectation:toBeCloseTo(expected: number, tolerance: number?)
		local actualNumber = actual :: number
		local limit = tolerance or 0.000001
		check(
			type(actual) == "number" and math.abs(actualNumber - expected) <= limit,
			inverted,
			"expected " .. formatValue(actual) .. " to be within " .. tostring(limit) .. " of " .. tostring(expected),
			"expected " .. formatValue(actual) .. " not to be within " .. tostring(limit) .. " of " .. tostring(expected)
		)
	end

	function expectation:toContain(expected: any)
		local pass = false
		if type(actual) == "string" and type(expected) == "string" then
			pass = string.find(actual, expected, 1, true) ~= nil
		elseif type(actual) == "table" then
			for _, value in pairs(actual) do
				if deepEqual(value, expected) then
					pass = true
					break
				end
			end
		end

		check(
			pass,
			inverted,
			"expected " .. formatValue(actual) .. " to contain " .. formatValue(expected),
			"expected " .. formatValue(actual) .. " not to contain " .. formatValue(expected)
		)
	end

	function expectation:toHaveLength(expected: number)
		local actualType = type(actual)
		local length = if actualType == "string" or actualType == "table" then #actual else nil
		check(
			length == expected,
			inverted,
			"expected length " .. formatValue(length) .. " to be " .. tostring(expected),
			"expected length " .. formatValue(length) .. " not to be " .. tostring(expected)
		)
	end

	function expectation:toThrow()
		if type(actual) ~= "function" then
			fail("expected " .. formatValue(actual) .. " to be a function")
		end

		local ok = pcall(actual)
		check(
			not ok,
			inverted,
			"expected function to throw",
			"expected function not to throw"
		)
	end

	if includeInverse ~= false then
		expectation.never = makeExpect(actual, not inverted, false)
	end
	return expectation
end

function Framework.describe(name: string, fn: TestFn)
	table.insert(suiteStack, name)
	local registeredName = fullName("registration")
	local ok, err = xpcall(fn, traceback)
	table.remove(suiteStack)

	if not ok then
		table.insert(tests, {
			name = registeredName,
			fn = function()
				error(err, 0)
			end,
		})
	end
end

function Framework.it(name: string, fn: TestFn)
	table.insert(tests, {
		name = fullName(name),
		fn = fn,
	})
end

function Framework.expect(actual: any): any
	return makeExpect(actual, false, true)
end

function Framework.clear()
	while #tests > 0 do
		table.remove(tests)
	end
	while #suiteStack > 0 do
		table.remove(suiteStack)
	end
end

function Framework.installGlobals()
	local globals: any = _G
	globals.describe = Framework.describe
	globals.it = Framework.it
	globals.expect = Framework.expect
end

function Framework.run(options: RunOptions?): RunResult
	local lines: { string } = {}
	local results: { TestResult } = {}
	local passed = 0
	local failed = 0

	for _, test in ipairs(tests) do
		local ok, err = xpcall(test.fn, traceback)

		if ok then
			passed += 1
			table.insert(lines, "[PASS] " .. test.name)
			table.insert(results, {
				name = test.name,
				ok = true,
			})
		else
			failed += 1
			local message = tostring(err)
			table.insert(lines, "[FAIL] " .. test.name .. ": " .. message)
			table.insert(results, {
				name = test.name,
				ok = false,
				message = message,
			})
		end
	end

	local total = passed + failed
	local summary = tostring(passed) .. "/" .. tostring(total) .. " tests passed"
	if failed > 0 then
		summary ..= ", " .. tostring(failed) .. " failed"
	end
	table.insert(lines, 1, summary)

	local logInfo = options and options.logInfo
	local logError = options and options.logError
	for index, line in ipairs(lines) do
		if index == 1 or string.sub(line, 1, 6) == "[PASS]" then
			if logInfo then
				logInfo(line)
			end
		elseif logError then
			logError(line)
		end
	end

	return {
		total = total,
		passed = passed,
		failed = failed,
		ok = failed == 0,
		lines = lines,
		results = results,
	}
end

return Framework
