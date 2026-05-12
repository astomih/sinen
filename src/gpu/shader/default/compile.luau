--!strict

local sn = require("@sinen")

type SourceTask = {
	src: string,
	out: string,
	stage: any,
}

type CompileTask = {
	src: string,
	out: string,
	stage: any,
	format: any,
	code: sn.Buffer,
}

local sourceTasks: { SourceTask } = {
	{ src = "shader.slang", out = "shader.frag", stage = sn.ShaderStage.Fragment },
	{ src = "font.slang", out = "font.frag", stage = sn.ShaderStage.Fragment },
	{ src = "shader.slang", out = "shader.vert", stage = sn.ShaderStage.Vertex },
	{ src = "shader_instance.slang", out = "shader_instance.vert", stage = sn.ShaderStage.Vertex },
	{ src = "rect_color.slang", out = "rect_color.frag", stage = sn.ShaderStage.Fragment },
	{ src = "cubemap.slang", out = "cubemap.frag", stage = sn.ShaderStage.Fragment },
	{ src = "cubemap.slang", out = "cubemap.vert", stage = sn.ShaderStage.Vertex },
}

local tasks: { CompileTask } = {}
local written = false

local function formatName(format: any): string
	if format == sn.ShaderFormat.SPIRV then
		return "SPIR-V"
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

local function addTask(src: string, out: string, stage: any, format: any)
	sn.Log.info("Compiling " .. out .. " from " .. src .. " (" .. stageName(stage) .. ", " .. formatName(format) .. ")")
	local shader = sn.Shader.new()
	shader:compile(src, stage, format)
	local code = shader:getCode()
	if code:size() == 0 then
		error("Shader compiled with empty code: " .. out)
	end
	table.insert(tasks, {
		src = src,
		out = out,
		stage = stage,
		format = format,
		code = code,
	})
end

local function writeTask(task: CompileTask)
	local code = task.code
	sn.Log.info("Writing " .. task.out .. " (" .. tostring(code:size()) .. " bytes)")
	writeFile(task.out, code)
	sn.Log.info("Writing " .. task.out .. ".hpp")
	writeFile(task.out .. ".hpp", code:toHeader(symbolName(task.out)))
end

function setup()
	sn.Log.info("Builtin shader compile started")
	for _, task in sourceTasks do
		addTask(task.src, task.out .. ".spv", task.stage, sn.ShaderFormat.SPIRV)
	end
	for _, task in sourceTasks do
		addTask(task.src, task.out .. ".wgsl", task.stage, sn.ShaderFormat.WGSL)
	end
	for _, task in sourceTasks do
		addTask(task.src, task.out .. ".dxil", task.stage, sn.ShaderFormat.DXIL)
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
