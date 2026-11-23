-- Build the pinned upstream projects that provide their own CMake build.
-- Each project gets its own build tree; there is no Sinen CMake project here.
import("core.project.config")
import("core.base.option")
import("lib.detect.find_tool")

local function copy_if_changed(source, destination)
	if not os.isfile(destination) or os.mtime(source) > os.mtime(destination) then
		os.cp(source, destination)
	end
end

function main(target)
	local root = os.projectdir()
	local vendor = target:targetdir()
	local windows = target:is_plat("windows")
	local android = target:is_plat("android")
	local wasm = target:is_plat("wasm")
	local mode = config.mode() == "debug" and "Debug" or "Release"
	local toolchain = target:toolchain(windows and "msvc" or wasm and "emcc" or "ndk")
	local envs = toolchain and table.copy(toolchain:runenvs()) or {}
	local cmake = assert(find_tool("cmake"), "CMake is required by the upstream submodules").program
	local jobs = tostring(option.get("jobs") or os.default_njob())
	local common = {
		"-G", "Ninja", "-DCMAKE_BUILD_TYPE=" .. mode,
		"-DBUILD_SHARED_LIBS=" .. (android and "ON" or "OFF"),
		"-DCMAKE_POLICY_VERSION_MINIMUM=3.5", "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
	}
	if windows then
		envs.VSLANG = "1033"
	elseif android then
		local ndk = config.get("ndk") or os.getenv("ANDROID_NDK_ROOT") or os.getenv("ANDROID_NDK_HOME")
		assert(ndk, "Use --ndk or set ANDROID_NDK_ROOT")
		table.join2(common, {"-DCMAKE_TOOLCHAIN_FILE=" .. path.join(ndk, "build/cmake/android.toolchain.cmake"),
			"-DANDROID_ABI=" .. target:arch(), "-DANDROID_PLATFORM=android-" .. (config.get("ndk_sdkver") or "28"),
			"-DANDROID_STL=c++_static"})
	elseif wasm then
		local emscripten = assert(toolchain and toolchain:config("bindir"), "Activate the Emscripten SDK before building")
		table.join2(common, {"-DCMAKE_TOOLCHAIN_FILE=" .. path.join(emscripten, "cmake/Modules/Platform/Emscripten.cmake"),
			"-DCMAKE_CXX_FLAGS=-fexceptions"})
	end

	local function build(name, source, options, ...)
		local dir = path.join(vendor, name)
		local args = table.join({"-S", path.join(root, source), "-B", dir}, common)
		for _, key in ipairs(table.orderkeys(options)) do
			table.insert(args, "-D" .. key .. "=" .. options[key])
		end
		local stamp = path.join(dir, "xmake-configure.txt")
		local signature = table.concat(args, "\n")
		if not os.isfile(path.join(dir, "build.ninja")) or not os.isfile(stamp) or io.readfile(stamp) ~= signature then
			cprint("${bright}configuring %s", name)
			os.vrunv(cmake, args, {envs = envs})
			io.writefile(stamp, signature)
		end
		local targets = {...}
		if #targets > 0 then
			os.vrunv(cmake, table.join({"--build", dir, "--parallel", jobs, "--target"}, targets),
				{envs = envs})
		end
		return dir
	end

	local libdir = path.join(vendor, "link")
	os.mkdir(libdir)
	local function stage(dir, pattern, name)
		local files = os.files(path.join(dir, pattern))
		assert(#files == 1, "Expected one upstream artifact matching %s in %s, found %d", pattern, dir, #files)
		local filename = windows and (name .. ".lib") or ("lib" .. name .. (android and ".so" or ".a"))
		copy_if_changed(files[1], path.join(libdir, filename))
	end
	local debug_suffix = windows and mode == "Debug" and "d" or ""
	local sdl = build("sdl", "libs/SDL", {
		SDL_STATIC = android and "OFF" or "ON", SDL_SHARED = android and "ON" or "OFF",
		SDL_TEST_LIBRARY = "OFF", SDL_TESTS = "OFF", SDL_EXAMPLES = "OFF", SDL_INSTALL = "OFF"
	}, android and "SDL3-shared" or "SDL3-static")
	stage(sdl, windows and "SDL3-static.lib" or android and "libSDL3.so" or "libSDL3.a", "SDL3")

	local assimp = build("assimp", "libs/assimp", {
		ASSIMP_BUILD_ZLIB = "ON", ASSIMP_BUILD_ASSIMP_TOOLS = "OFF", ASSIMP_BUILD_TESTS = "OFF",
		ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT = "OFF", ASSIMP_WARNINGS_AS_ERRORS = "OFF", ASSIMP_INSTALL = "OFF"
	}, "assimp")
	stage(assimp, windows and "lib/assimp*.lib" or android and "bin/libassimp.so" or "lib/libassimp.a", "assimp")
	if not android then
		stage(assimp, windows and ("contrib/zlib/zlibstatic" .. debug_suffix .. ".lib") or "contrib/zlib/libzlibstatic.a", "zlib")
	end

	local ktx = build("ktx", "libs/ktx-software", {
		KTX_FEATURE_JS = "OFF", KTX_FEATURE_TESTS = "OFF", KTX_FEATURE_LOADTEST_APPS = "OFF",
		LIBKTX_FEATURE_KTX1 = "ON", LIBKTX_FEATURE_KTX2 = "ON", LIBKTX_FEATURE_VK_UPLOAD = "ON",
		LIBKTX_FEATURE_GL_UPLOAD = "ON", LIBKTX_VERSION_FULL = "OFF", CMAKE_DISABLE_FIND_PACKAGE_Perl = "ON"
	}, "ktx_read")
	stage(ktx, windows and "lib/ktx_read*.lib" or android and "lib/libktx_read.so" or "lib/libktx_read.a", "ktx_read")

	if target:values("sinen.dawn") then
		local dawn = build("dawn", "libs/WebGPU-distribution", {WEBGPU_BACKEND = "DAWN", WEBGPU_LINK_TYPE = "SHARED"})
		local dir = assert(os.dirs(path.join(dawn, "_deps/dawn-*-windows-x64-release-src"))[1], "Dawn archive did not unpack")
		stage(dir, "lib/webgpu_dawn.lib", "webgpu_dawn")
		local includes = path.join(dir, "include")
		for _, header in ipairs(os.files(path.join(includes, "**.h"))) do
			local destination = path.join(dawn, "include", path.relative(header, includes))
			os.mkdir(path.directory(destination))
			copy_if_changed(header, destination)
		end
		copy_if_changed(path.join(dir, "bin/webgpu_dawn.dll"), path.join(path.directory(vendor), "webgpu_dawn.dll"))
	end

	if target:values("sinen.slang") then
		local options = {SLANG_LIB_TYPE = "SHARED",
			SLANG_RHI_ENABLE = "OFF", SLANG_ENABLE_SLANG_RHI = "OFF", SLANG_ENABLE_SLANG_PROXY = "OFF",
			SLANG_EXCLUDE_DAWN = "OFF", SLANG_EXCLUDE_TINT = "OFF",
			SLANG_EMBED_CORE_MODULE = android and "OFF" or "ON",
			SLANG_EMBED_CORE_MODULE_SOURCE = android and "ON" or "OFF"}
		local disabled = {"GFX", "SLANGD", "SLANGC", "SLANGI", "TESTS", "EXAMPLES", "REPLAYER"}
		if android then
			table.join2(disabled, {"CUDA", "DXIL", "OPTIX", "NVAPI", "SLANGRT", "SLANG_GLSLANG", "XLIB", "AFTERMATH"})
			options.SLANG_ENABLE_PREBUILT_BINARIES = "OFF"
			options.SLANG_SLANG_LLVM_FLAVOR = "DISABLE"
		end
		for _, feature in ipairs(disabled) do
			options["SLANG_ENABLE_" .. feature] = "OFF"
		end
		if windows then
			options.CMAKE_C_FLAGS = "/utf-8"
		end
		local generators = config.get("sinen_slang_generators")
		if not generators or #generators == 0 then
			generators = os.getenv("SINEN_SLANG_GENERATORS_PATH")
		end
		if android and generators and #generators > 0 then
			options.SLANG_GENERATORS_PATH = path.absolute(generators, root)
		end
		local dir = build("slang", "plugins/shader_compiler/libs/slang", options, "slang")
		if windows then
			stage(dir, mode .. "/lib/slang-compiler*.lib", "slang-compiler")
			local dll = assert(os.files(path.join(dir, mode, "bin/slang-compiler*.dll"))[1], "No Slang compiler DLL")
			copy_if_changed(dll, path.join(path.directory(vendor), path.filename(dll)))
		else
			stage(dir, mode .. "/lib/libslang-compiler.so", "slang-compiler")
			stage(dir, "external/miniz/libminiz.so", "miniz")
		end
	end
end
