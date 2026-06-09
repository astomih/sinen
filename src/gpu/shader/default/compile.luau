--!strict

local sn = require("@sinen")

type SourceTask = {
	src: string,
	out: string,
	stage: any,
	numSamplers: number,
	numStorageBuffers: number,
	numStorageTextures: number,
	numUniformBuffers: number,
}

type BundleTask = {
	out: string,
	entries: { sn.ShaderBundleEntry },
}

local sourceTasks: { SourceTask } = {
	{
		src = "shader.slang",
		out = "shader.frag",
		stage = sn.ShaderStage.Fragment,
		numSamplers = 1,
		numStorageBuffers = 0,
		numStorageTextures = 0,
		numUniformBuffers = 1,
	},
	{
		src = "font.slang",
		out = "font.frag",
		stage = sn.ShaderStage.Fragment,
		numSamplers = 1,
		numStorageBuffers = 0,
		numStorageTextures = 0,
		numUniformBuffers = 2,
	},
	{
		src = "shader.slang",
		out = "shader.vert",
		stage = sn.ShaderStage.Vertex,
		numSamplers = 0,
		numStorageBuffers = 0,
		numStorageTextures = 0,
		numUniformBuffers = 1,
	},
	{
		src = "shader_instance.slang",
		out = "shader_instance.vert",
		stage = sn.ShaderStage.Vertex,
		numSamplers = 0,
		numStorageBuffers = 0,
		numStorageTextures = 0,
		numUniformBuffers = 1,
	},
	{
		src = "rect_color.slang",
		out = "rect_color.frag",
		stage = sn.ShaderStage.Fragment,
		numSamplers = 0,
		numStorageBuffers = 0,
		numStorageTextures = 0,
		numUniformBuffers = 2,
	},
	{
		src = "cubemap.slang",
		out = "cubemap.frag",
		stage = sn.ShaderStage.Fragment,
		numSamplers = 1,
		numStorageBuffers = 0,
		numStorageTextures = 0,
		numUniformBuffers = 1,
	},
	{
		src = "cubemap.slang",
		out = "cubemap.vert",
		stage = sn.ShaderStage.Vertex,
		numSamplers = 0,
		numStorageBuffers = 0,
		numStorageTextures = 0,
		numUniformBuffers = 1,
	},
}

local tasks: { BundleTask } = {}
local written = false

local function formatName(format: any): string
	if format == sn.ShaderFormat.SPIRV then
		return "SPIR-V"
	elseif format == sn.ShaderFormat.SPIRV_1_3 then
		return "SPIR-V 1.3"
	elseif format == sn.ShaderFormat.WGSL then
		return "WGSL"
	elseif format == sn.ShaderFormat.DXIL then
		return "DXIL"
	elseif format == sn.ShaderFormat.DXBC then
		return "DXBC"
	end
	return "Unknown"
end

local function stageName(stage: any): string
	if stage == sn.ShaderStage.Vertex then
		return "Vertex"
	elseif stage == sn.ShaderStage.Fragment then
		return "Fragment"
	elseif stage == sn.ShaderStage.Compute then
		return "Compute"
	end
	return "Unknown"
end

local function symbolName(path: string): string
	return (string.gsub(path, "[^%w]", "_"))
end

local function writeFile(path: string, data: sn.Buffer)
	if not sn.Filesystem.write(path, data) then
		error("Failed to open " .. path)
	end
end

local function compileTask(task: SourceTask, format: any): sn.ShaderBundleEntry
	sn.Log.info(
		"Compiling "
			.. task.out
			.. " from "
			.. task.src
			.. " ("
			.. stageName(task.stage)
			.. ", "
			.. formatName(format)
			.. ")"
	)
	local shader = sn.Shader.new()
	shader:compile(task.src, task.stage, format)
	local code = shader:getCode()
	if code:size() == 0 then
		error("Shader compiled with empty code: " .. task.out)
	end
	return {
		src = task.src,
		stage = task.stage,
		format = format,
		numSamplers = task.numSamplers,
		numStorageBuffers = task.numStorageBuffers,
		numStorageTextures = task.numStorageTextures,
		numUniformBuffers = task.numUniformBuffers,
		code = code,
	}
end

local function addBundleTask(task: SourceTask)
	local entries = {
		compileTask(task, sn.ShaderFormat.SPIRV),
		compileTask(task, sn.ShaderFormat.SPIRV_1_3),
		compileTask(task, sn.ShaderFormat.WGSL),
		compileTask(task, sn.ShaderFormat.DXIL),
	}
	table.insert(tasks, {
		out = task.out .. ".snb",
		entries = entries,
	})
end

local function writeTask(task: BundleTask)
	local bundle = sn.ShaderBundle.pack(task.entries)
	if bundle:size() == 0 then
		error("Shader bundle is empty: " .. task.out)
	end
	sn.Log.info("Writing " .. task.out .. " (" .. tostring(bundle:size()) .. " bytes)")
	writeFile(task.out, bundle)
	sn.Log.info("Writing " .. task.out .. ".hpp")
	writeFile(task.out .. ".hpp", bundle:toHeader(symbolName(task.out)))
end

function setup()
	sn.Log.info("Builtin shader compile started")
	for _, task in sourceTasks do
		addBundleTask(task)
	end
	sn.Log.info("Queued " .. tostring(#tasks) .. " builtin shader compile tasks")
end

function update()
	if written then
		return
	end

	for _, task in tasks do
		writeTask(task)
	end

	written = true
	sn.Log.info("Builtin shader compile finished")
	sn.Event.quit()
end

return {}
