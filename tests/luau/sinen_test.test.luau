--!strict

local test = require("./sinen_test")
local describe = test.describe
local it = test.it
local expect = test.expect

describe("sinen_test", function()
	it("checks primitive equality", function()
		expect(1 + 1):toBe(2)
		expect("sinen"):toContain("in")
		expect(false):toBeFalsy()
		expect(true):toBeTruthy()
	end)

	it("checks deep table equality", function()
		expect({
			name = "runner",
			tags = { "luau", "unit" },
			config = {
				strict = true,
			},
		}):toEqual({
			name = "runner",
			tags = { "luau", "unit" },
			config = {
				strict = true,
			},
		})
	end)

	it("checks inverse assertions", function()
		expect(10).never:toBe(11)
		expect({ "a", "b", "c" }).never:toContain("z")
	end)

	it("checks numeric tolerance", function()
		expect(0.1 + 0.2):toBeCloseTo(0.3, 0.000001)
	end)

	it("checks function errors", function()
		expect(function()
			error("expected failure")
		end):toThrow()
	end)
end)

return true
