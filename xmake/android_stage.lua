function main(target)
	local app = target:dep("sinen")
	local abi = target:arch()
	local destination = path.join(os.projectdir(), "build/android/app/src/main/jniLibs", abi)
	local upstream = path.join(app:targetdir(), "vendor/upstream/link")
	os.mkdir(destination)
	for _, name in ipairs({"sinen", "box2d", "box3d", "msdfgen-core", "cppdap", "TracyClient", "sinen_plugin_shader_compiler"}) do
		local dep = target:dep(name)
		if dep then os.cp(dep:targetfile(), destination) end
	end
	for _, name in ipairs({"SDL3", "assimp", "ktx_read"}) do
		os.cp(path.join(upstream, "lib" .. name .. ".so"), destination)
	end
	if target:dep("sinen_plugin_shader_compiler") then
		for _, name in ipairs({"slang-compiler", "miniz"}) do
			os.cp(path.join(upstream, "lib" .. name .. ".so"), destination)
		end
	end
end
