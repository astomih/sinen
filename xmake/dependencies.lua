local android = is_plat("android")
local windows = is_plat("windows")
local wasm = is_plat("wasm")

local function dependency(name, shared)
	target(name)
	set_kind(android and shared and "shared" or "static")
	set_default(false)
	set_targetdir("$(builddir)/vendor/native")
	if windows then
		set_runtimes(is_mode("debug") and "MDd" or "MD")
		add_defines("_CRT_SECURE_NO_WARNINGS", "_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR")
	elseif android or wasm then
		add_cxflags("-fPIC", {force = true})
	end
	if wasm then
		add_cxxflags("-fexceptions", {force = true})
	end
end

for _, name in ipairs({"box2d", "box3d"}) do
	dependency(name, true)
	set_languages("c17")
	add_files("../libs/" .. name .. "/src/*.c")
	add_includedirs("../libs/" .. name .. "/include", "../libs/" .. name .. "/src")
	if not windows then add_defines("_POSIX_C_SOURCE=200809L") end
	if wasm then
		add_defines(name == "box2d" and "BOX2D_DISABLE_SIMD" or "BOX3D_DISABLE_SIMD")
	end
	if is_mode("debug") then
		add_defines(name == "box2d" and "B2_ENABLE_ASSERT" or "B3_ENABLE_ASSERT")
	end
	target_end()
end

dependency("msdfgen-core", true)
add_files("../libs/msdfgen/core/*.cpp")
add_includedirs("../libs/msdfgen")
add_defines("MSDFGEN_USE_CPP11", "MSDFGEN_PUBLIC=")
target_end()

dependency("meshoptimizer", false)
add_files("../libs/meshoptimizer/src/*.cpp")
target_end()

dependency("cppdap", true)
add_includedirs("../libs/cppdap/include", "../libs/cppdap/third_party/json/include")
add_files("../libs/cppdap/src/*.cpp")
remove_files("../libs/cppdap/src/*_test.cpp", "../libs/cppdap/src/*_fuzzer.cpp",
	"../libs/cppdap/src/jsoncpp_json_serializer.cpp", "../libs/cppdap/src/rapid_json_serializer.cpp",
	"../libs/cppdap/src/test.cpp")
add_defines("CPPDAP_JSON_NLOHMANN=1")
target_end()

dependency("sinen_vendor_luau", false)
local luau_modules = {"Ast", "Bytecode", "Common", "Compiler", "Config", "Require", "VM"}
if not wasm then
	table.insert(luau_modules, "CodeGen")
end
for _, name in ipairs(luau_modules) do
	add_files("../libs/luau/" .. name .. "/src/*.cpp")
	add_includedirs("../libs/luau/" .. name .. "/include")
end
add_includedirs("../libs/luau/VM/src", "../libs/luau-debugger/debugger/include",
	"../libs/luau-debugger/debugger/src", "../libs/cppdap/include", "../libs/cppdap/src",
	"../libs/cppdap/third_party/json/include")
add_files("../libs/luau-debugger/debugger/src/**.cpp")
if wasm then
	add_defines("LUA_API=__attribute__((used))")
end
add_deps("cppdap")
target_end()

if has_config("sinen_profiler") then
	dependency("TracyClient", true)
	add_files("../libs/tracy/public/TracyClient.cpp")
	add_defines("TRACY_ENABLE", "TRACY_ON_DEMAND", "TRACY_ONLY_LOCALHOST")
	target_end()
end

if has_config("sinen_cpp_tests") then
	dependency("sinen_vendor_gtest", false)
	local gtest = "../tests/cpp/googletest/googletest"
	add_files(gtest .. "/src/gtest-all.cc")
	add_includedirs(gtest, gtest .. "/include")
	target_end()
end
