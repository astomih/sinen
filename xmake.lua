set_project("sinen")
set_version("0.0.5")
add_rules("mode.debug", "mode.release")
set_languages("cxx23")

local root = os.projectdir()
local output = path.absolute(get_config("builddir") or "build/xmake", root)
local vendor = path.join(output, "vendor/upstream")
local generated = path.join(output, "generated")
local wasm = is_plat("wasm")
local android = is_plat("android")
local windows = is_plat("windows")
local mode = get_config("mode") or "release"
local debug = mode == "debug"

option("sinen_webgpu_dawn")
set_default(windows and debug)
option_end()
option("sinen_shader_compiler")
set_default(not wasm)
option_end()
option("sinen_profiler")
set_default(true)
option_end()
option("sinen_cpp_tests")
set_default(not android)
option_end()
option("sinen_nativefs")
set_default(false)
option_end()

local webgpu = wasm or (windows and has_config("sinen_webgpu_dawn"))
local slang = not wasm and has_config("sinen_shader_compiler")

option("sinen_preload")
set_default("")
set_showmenu(true)
set_description("Directory to preload at / in the Emscripten app")
option_end()
option("sinen_slang_generators")
set_default("")
set_showmenu(true)
set_description("Host Slang generator directory for Android")
option_end()

includes("xmake/dependencies.lua")

target("sinen_vendor")
set_kind("phony")
set_default(false)
set_targetdir(vendor)
set_policy("build.fence", true)
set_values("sinen.dawn", windows and webgpu)
set_values("sinen.slang", slang)
on_build(function(target)
	import("upstream", { rootdir = path.join(os.projectdir(), "xmake") })
	upstream(target)
end)
target_end()

target("sinen_licenses")
set_kind("phony")
set_default(false)
set_policy("build.fence", true)
set_targetdir(generated)
on_build(function(target)
	import("licenses", { rootdir = path.join(os.projectdir(), "xmake") })
	licenses(path.join(target:targetdir(), "licenses_data.hpp"))
end)
target_end()

local function vendor_includes()
	add_includedirs(
		"libs/SDL/include",
		"libs/rapidjson/include",
		"libs/tlsf",
		"libs/tinyexr",
		"libs/tinyexr/deps/miniz",
		"libs/miniaudio",
		"libs/stb",
		"libs/minimp4",
		"libs/volk",
		"libs/VulkanMemoryAllocator/include",
		"libs/renderdoc",
		"libs/assimp/include",
		"libs/box2d/include",
		"libs/box3d/include",
		"libs/meshoptimizer/src",
		"libs/openh264/codec/api",
		"libs/msdfgen",
		"libs/msdfgen/core",
		"libs/ktx-software/lib/include",
		"libs/ktx-software/external/dfdutils",
		"libs/cppdap/include",
		"libs/luau/VM/include",
		"libs/luau/Compiler/include",
		"libs/luau/Ast/include",
		"libs/luau/CodeGen/include",
		"libs/luau/Config/include",
		"libs/luau/Require/include",
		"libs/luau/Common/include",
		"libs/luau-debugger/debugger/include",
		"libs/tracy/public",
		"plugins/ocean/include",
		"plugins/shader_compiler/include",
		"plugins/nativefs/include",
		"tests/cpp/googletest/googletest/include",
		generated,
		path.join(vendor, "assimp/include"),
		path.join(vendor, "sdl/include-revision")
	)
	if slang then
		add_includedirs("plugins/shader_compiler/libs/slang/include")
	end
	if webgpu and windows then
		add_includedirs("libs/WebGPU-distribution/dawn/include")
		add_defines("DAWN_NATIVE_SHARED_LIBRARY", "WEBGPU_BACKEND_DAWN", "WGPU_SHARED_LIBRARY")
		add_includedirs(path.join(vendor, "dawn/include"))
	end
end

local function common()
	add_deps("sinen_vendor", { inherit = false })
	set_targetdir(output)
	set_objectdir(path.join(output, ".objs"))
	add_includedirs("include", "include/sinen", "src")
	vendor_includes()
	local vulkan_sdk = os.getenv("VULKAN_SDK")
	if vulkan_sdk then
		add_includedirs(path.join(vulkan_sdk, "Include"))
	end
	add_defines(
		"SINEN_MODULE_CORE",
		"SINEN_MODULE_MATH",
		"SINEN_MODULE_PLATFORM",
		"SINEN_MODULE_AUDIO",
		"SINEN_MODULE_GPU",
		"SINEN_MODULE_GRAPHICS",
		"SINEN_MODULE_RENDERER",
		"SINEN_MODULE_PHYSICS",
		"SINEN_MODULE_SCRIPT"
	)
	add_defines(
		"MSDFGEN_PUBLIC=",
		"MSDFGEN_USE_CPP11",
		"KHRONOS_STATIC",
		"KTX_FEATURE_KTX1",
		"KTX_FEATURE_KTX2",
		"BASISU_NO_ITERATOR_DEBUG_LEVEL"
	)
	if has_config("sinen_profiler") then
		add_defines("SINEN_MODULE_PROFILER")
	end
	if webgpu then
		add_defines("SINEN_ENABLE_WEBGPU")
	end
	if debug then
		add_defines("SINEN_DEBUG_WORKING_DIRECTORY")
	end
	if wasm then
		add_defines("EMSCRIPTEN", "WEBGPU_BACKEND_EMDAWNWEBGPU")
		add_cxflags("-fPIC", "-fexceptions", "--use-port=emdawnwebgpu", { force = true })
	elseif android then
		add_cxflags("-fPIC", { force = true })
	elseif windows then
		add_cxflags("/bigobj", { force = true })
		set_runtimes(debug and "MDd" or "MD")
	end
end

local modules = { "core", "math", "platform", "audio", "gpu", "graphics", "renderer", "physics", "script" }
target("sinen_openh264")
set_kind("static")
common()
add_files(
	"libs/openh264/codec/common/src/*.cpp",
	"libs/openh264/codec/encoder/core/src/*.cpp",
	"libs/openh264/codec/encoder/plus/src/*.cpp",
	"libs/openh264/codec/processing/src/**.cpp"
)
remove_files("libs/openh264/codec/encoder/plus/src/DllEntry.cpp")
add_includedirs(
	"libs/openh264/codec/api",
	"libs/openh264/codec/api/wels",
	"libs/openh264/codec/common/inc",
	"libs/openh264/codec/encoder/core/inc",
	"libs/openh264/codec/encoder/plus/inc",
	"libs/openh264/codec/processing/interface",
	"libs/openh264/codec/processing/src/common"
)
target_end()
for _, name in ipairs(modules) do
	target("sinen_" .. name)
	set_kind("static")
	common()
	add_files("src/" .. name .. "/**.cpp")
	if name == "core" then
		add_deps("sinen_licenses", { inherit = false })
		add_files("libs/tlsf/tlsf.c")
	elseif name == "platform" then
		add_files("libs/tinyexr/deps/miniz/miniz.c")
	elseif name == "gpu" then
		if not webgpu then
			remove_files("src/gpu/backend/webgpu/**.cpp")
		end
		if windows then
			add_syslinks("d3d12", "dxgi", "dxguid")
		end
	elseif name == "script" and not wasm then
		add_defines("SINEN_LUAU_CODEGEN")
	end
	if name == "graphics" then
		add_deps("sinen_openh264")
	end
	target_end()
end

local function link_vendor()
	local links = { "SDL3", "assimp", "ktx_read" }
	if not android then
		table.insert(links, "zlib")
	end
	if windows and webgpu then
		table.insert(links, "webgpu_dawn")
	end
	if slang then
		table.insert(links, "slang-compiler")
		if android then
			table.insert(links, "miniz")
		end
	end
	for _, lib in ipairs(links) do
		local filename = windows and (lib .. ".lib") or ("lib" .. lib .. (android and ".so" or ".a"))
		add_links(path.join(vendor, "link", filename))
	end
	if wasm then
		-- The pinned KTX read archive contains two copies of Basis Universal's miniz.
		add_ldflags("-Wl,--allow-multiple-definition", { force = true })
	end
	add_deps("box2d", "box3d", "msdfgen-core", "meshoptimizer", "sinen_vendor_luau")
	if has_config("sinen_profiler") then
		add_deps("TracyClient")
	end
	if windows then
		add_syslinks(
			"kernel32",
			"user32",
			"gdi32",
			"winmm",
			"imm32",
			"ole32",
			"oleaut32",
			"version",
			"uuid",
			"advapi32",
			"setupapi",
			"shell32",
			"dinput8",
			"ws2_32",
			"winspool",
			"comdlg32"
		)
	elseif android then
		add_syslinks("dl")
	elseif not wasm then
		add_syslinks("dl", "pthread")
	end
end

target("sinen")
set_kind(android and "shared" or "binary")
common()
add_files("src/main.cpp", "src/debug_overlay.cpp")
if windows then
	add_files("docs/docs/icon/icon.rc")
	add_ldflags("/SUBSYSTEM:WINDOWS", { force = true })
elseif wasm then
	set_extension(".html")
	add_ldflags(
		"--use-port=emdawnwebgpu",
		"-sASYNCIFY=1",
		"-sALLOW_MEMORY_GROWTH=1",
		"-sMAIN_MODULE=2",
		"-sINITIAL_MEMORY=268435456",
		"-sMAXIMUM_MEMORY=2147483648",
		"-fexceptions",
		"-sNO_DISABLE_EXCEPTION_CATCHING=1",
		"-sASSERTIONS=1",
		{ force = true }
	)
	local assets = get_config("sinen_preload")
	if assets and #assets > 0 then
		add_ldflags("--preload-file", path.absolute(assets, root) .. "@/", { force = true })
	end
elseif android then
	add_syslinks("android")
end
for _, name in ipairs(modules) do
	add_deps("sinen_" .. name)
end
link_vendor()
target_end()

local function lua_module(name, files)
	target(name)
	set_kind("shared")
	common()
	set_filename(
		(android and "lib" or "")
			.. name:gsub("^sinen_plugin_", "")
			.. (wasm and ".wasm" or windows and ".dll" or ".so")
	)
	add_files(table.unpack(files))
	if name == "sinen_plugin_ocean" then
		add_deps("sinen_ocean")
	end
	if wasm then
		add_shflags("-sSIDE_MODULE=1", { force = true })
	else
		link_vendor()
	end
	target_end()
end

target("sinen_ocean")
set_kind("static")
common()
add_files("plugins/ocean/source/ocean.cpp")
target_end()
lua_module("sinen_plugin_ocean", { "plugins/ocean/source/plugin.cpp" })
if not android and not wasm then
	lua_module(
		"sinen_plugin_luau_to_cpp",
		{ "plugins/luau_to_cpp/source/plugin.cpp", "plugins/luau_to_cpp/source/converter.cpp" }
	)
	target("sinen_luau_to_cpp")
	set_kind("binary")
	common()
	add_files("plugins/luau_to_cpp/source/cli.cpp", "plugins/luau_to_cpp/source/converter.cpp")
	link_vendor()
	target_end()
end

target("sinen_plugin_fps_collision")
set_kind("shared")
common()
set_filename(windows and "fps_collision.dll" or "fps_collision.so")
add_files("game/fps/plugin/plugin.cpp")
add_includedirs(path.join(output, "game/fps/plugin"))
add_deps("sinen_core")
link_vendor()
after_build(function(target)
	os.cp(target:targetfile(), path.join(root, "game/fps"))
end)
target_end()

if has_config("sinen_nativefs") then
	lua_module("sinen_plugin_nativefs", { "plugins/nativefs/source/nativefs.cpp" })
end
if slang then
	lua_module("sinen_plugin_shader_compiler", {
		"plugins/shader_compiler/source/plugin.cpp",
		"plugins/shader_compiler/source/shader_compiler.cpp",
	})
	target("sinen_plugin_shader_compiler")
	add_defines("SINEN_SHADER_COMPILER_PLUGIN_BUILD")
end

if has_config("sinen_cpp_tests") then
	if windows or wasm then
		lua_module("sinen_test_native_module", { "tests/cpp/native_module_fixture.cpp" })
	end
	if windows then
		lua_module("sinen_test_failing_native_module", { "tests/cpp/native_module_failure_fixture.cpp" })
		lua_module("sinen_test_shared_binding_consumer", { "tests/cpp/shared_binding_consumer_fixture.cpp" })
		target("sinen_test_renderdoc")
		set_kind("shared")
		common()
		add_files("tests/cpp/renderdoc_fixture.cpp")
		target_end()
	end
	local function test_target(name, files, deps)
		target(name)
		set_kind("binary")
		set_default(false)
		common()
		add_files(table.unpack(files))
		add_files("tests/cpp/xmake_test_main.cpp")
		add_deps("sinen_vendor_gtest")
		for _, dep in ipairs(deps) do
			add_deps(dep)
		end
		link_vendor()
		if name == "sinen_shader_bundle_tests" then
			add_defines('SINEN_TEST_SOURCE_DIR="' .. root:gsub("\\", "/") .. '"')
		elseif name == "sinen_renderdoc_tests" then
			add_defines(
				'SINEN_TEST_RENDERDOC_PATH="' .. path.join(output, "sinen_test_renderdoc.dll"):gsub("\\", "/") .. '"'
			)
		elseif name == "sinen_ocean_render_tests" then
			add_defines(
				'SINEN_TEST_OCEAN_SOURCE="'
					.. path.join(root, "examples/graphics/ocean/ocean.slang"):gsub("\\", "/")
					.. '"'
			)
		elseif name == "sinen_engine_binding_tests" and windows then
			add_defines('SINEN_TEST_FPS_DIR="' .. path.join(root, "game/fps"):gsub("\\", "/") .. '"')
			add_deps("sinen_plugin_fps_collision")
		elseif name == "sinen_luau_binding_tests" and windows then
			add_defines(
				'SINEN_TEST_NATIVE_MODULE_PATH="'
					.. path.join(output, "sinen_test_native_module.dll"):gsub("\\", "/")
					.. '"',
				'SINEN_TEST_FAILING_NATIVE_MODULE_PATH="'
					.. path.join(output, "sinen_test_failing_native_module.dll"):gsub("\\", "/")
					.. '"',
				'SINEN_TEST_OCEAN_PATH="' .. path.join(output, "ocean.dll"):gsub("\\", "/") .. '"',
				'SINEN_TEST_LUAU_TO_CPP_DIR="' .. output:gsub("\\", "/") .. '"',
				'SINEN_TEST_FPS_DIR="' .. path.join(root, "game/fps"):gsub("\\", "/") .. '"'
			)
			add_defines('SINEN_TEST_SHARED_BINDING_CONSUMER_PATH="'
				.. path.join(output, "sinen_test_shared_binding_consumer.dll"):gsub("\\", "/") .. '"')
			add_deps(
				"sinen_test_native_module",
				"sinen_test_shared_binding_consumer",
				"sinen_test_failing_native_module",
				"sinen_plugin_ocean",
				"sinen_plugin_fps_collision"
			)
			if slang then
				add_defines(
					'SINEN_TEST_SHADER_COMPILER_PLUGIN_PATH="'
						.. path.join(output, "shader_compiler.dll"):gsub("\\", "/")
						.. '"'
				)
				add_deps("sinen_plugin_shader_compiler")
			end
			if has_config("sinen_nativefs") then
				add_defines(
					'SINEN_TEST_NATIVEFS_PATH="' .. path.join(output, "nativefs.dll"):gsub("\\", "/") .. '"',
					'SINEN_TEST_SOURCE_DIR="' .. root:gsub("\\", "/") .. '"'
				)
				add_deps("sinen_plugin_nativefs")
			end
		end
		set_rundir(output)
		if not wasm then
			add_tests("default")
		end
		target_end()
	end
	test_target("sinen_allocator_tests", { "tests/cpp/allocator_tests.cpp" }, { "sinen_core" })
	test_target("sinen_license_report_tests", { "tests/cpp/license_report_tests.cpp" }, { "sinen_core" })
	test_target("sinen_ocean_tests", { "tests/cpp/ocean_tests.cpp" }, { "sinen_ocean" })
	local gi_files = { "tests/cpp/global_illumination_tests.cpp" }
	if windows then
		table.insert(gi_files, "tests/cpp/realtime_gi_tests.cpp")
	end
	test_target("sinen_global_illumination_tests", gi_files, {
		"sinen_renderer",
		"sinen_script",
		"sinen_graphics",
		"sinen_gpu",
		"sinen_platform",
		"sinen_audio",
		"sinen_physics",
		"sinen_core",
		"sinen_math",
	})
	test_target("sinen_dds_decoder_tests", { "tests/cpp/dds_decoder_tests.cpp" }, {})
	if not wasm then
		target("sinen_test_h264_decoder")
		set_kind("static")
		common()
		add_files("libs/openh264/codec/decoder/core/src/*.cpp", "libs/openh264/codec/decoder/plus/src/*.cpp")
		add_includedirs(
			"libs/openh264/codec/api/wels",
			"libs/openh264/codec/common/inc",
			"libs/openh264/codec/decoder/core/inc",
			"libs/openh264/codec/decoder/plus/inc"
		)
		add_deps("sinen_openh264")
		target_end()
	end
	local video_deps = { "sinen_graphics", "sinen_gpu", "sinen_platform", "sinen_core", "sinen_math", "sinen_openh264" }
	if not wasm then
		table.insert(video_deps, "sinen_test_h264_decoder")
	end
	test_target("sinen_video_tests", { "tests/cpp/video_tests.cpp" }, video_deps)
	test_target(
		"sinen_external_file_tests",
		{ "tests/cpp/external_file_tests.cpp" },
		{ "sinen_platform", "sinen_core", "sinen_math" }
	)
	test_target(
		"sinen_shader_bundle_tests",
		{ "tests/cpp/shader_bundle_tests.cpp" },
		{ "sinen_gpu", "sinen_core", "sinen_math" }
	)
	test_target("sinen_engine_binding_tests", { "tests/cpp/engine_binding_tests.cpp" }, {
		"sinen_script",
		"sinen_graphics",
		"sinen_gpu",
		"sinen_platform",
		"sinen_audio",
		"sinen_physics",
		"sinen_renderer",
		"sinen_core",
		"sinen_math",
	})
	local luau_files = {
		"tests/cpp/luau_binding_tests.cpp",
		"tests/cpp/luau_bind_library_tests.cpp",
		"src/script/require_alias.cpp",
		"src/script/bindings/vec2_lua.cpp",
	}
	if windows then
		table.insert(luau_files, "tests/cpp/native_module_tests.cpp")
		table.insert(luau_files, "src/script/native_module.cpp")
	end
	test_target("sinen_luau_binding_tests", luau_files, { "sinen_math", "sinen_core" })
	if windows then
		test_target(
			"sinen_renderdoc_tests",
			{ "tests/cpp/renderdoc_tests.cpp" },
			{ "sinen_gpu", "sinen_core", "sinen_math", "sinen_test_renderdoc" }
		)
		if slang then
			test_target("sinen_ocean_render_tests", { "tests/cpp/ocean_render_tests.cpp" }, {
				"sinen_script",
				"sinen_ocean",
				"sinen_graphics",
				"sinen_gpu",
				"sinen_platform",
				"sinen_audio",
				"sinen_physics",
				"sinen_renderer",
				"sinen_core",
				"sinen_math",
				"sinen_plugin_shader_compiler",
			})
		end
		test_target(
			"sinen_gamepad_tests",
			{ "tests/cpp/gamepad_tests.cpp" },
			{ "sinen_platform", "sinen_core", "sinen_math" }
		)
		test_target(
			"sinen_d3d12_frame_tests",
			{ "tests/cpp/d3d12_frame_tests.cpp" },
			{ "sinen_gpu", "sinen_core", "sinen_math" }
		)
		test_target("sinen_text_cache_tests", { "tests/cpp/text_cache_tests.cpp" }, {
			"sinen_script",
			"sinen_graphics",
			"sinen_gpu",
			"sinen_platform",
			"sinen_audio",
			"sinen_physics",
			"sinen_renderer",
			"sinen_core",
			"sinen_math",
		})
	end
end

if android then
	target("sinen_android_package")
	set_kind("phony")
	add_deps("sinen")
	add_deps("box2d", "box3d", "msdfgen-core", "cppdap")
	if slang then
		add_deps("sinen_plugin_shader_compiler")
	end
	if has_config("sinen_profiler") then
		add_deps("TracyClient")
	end
	on_build(function(target)
		import("android_stage", { rootdir = path.join(os.projectdir(), "xmake") })
		android_stage(target)
	end)
	target_end()
end
