local sn = require("@sinen")
local G = require("../../global")

type SlangFile = {
	name: string,
	path: string,
}

type DirectoryItem = {
	name: string,
	path: string,
}

local currentDirectory = "/"
local parentDirectory = "/"
local directories: { DirectoryItem } = {}
local slangFiles: { SlangFile } = {}
local selectedFile: SlangFile? = nil
local selectedStage = sn.ShaderStage.Fragment
local selectedStageName = "Fragment"
local selectedStageSuffix = "frag"
local includeSPIRV = true
local includeSPIRV13 = true
local includeWGSL = true
local includeDXIL = true
local directoryScrollY = 0.0
local fileScrollY = 0.0
local status = "Select a directory and a .slang file."

local function endsWith(text: string, suffix: string): boolean
	return string.sub(text, #text - #suffix + 1) == suffix
end

local function stripTrailingSlash(path: string): string
	while #path > 1 and (endsWith(path, "/") or endsWith(path, "\\")) do
		path = string.sub(path, 1, #path - 1)
	end
	return path
end

local function joinPath(base: string, name: string): string
	if base == "" or base == "." then
		return name
	end
	if base == "/" then
		return "/" .. name
	end
	return base .. "/" .. name
end

local function parentPath(path: string): string
	path = stripTrailingSlash(path)
	if path == "." or path == "/" or path == "" then
		return path == "/" and "/" or "."
	end

	local i = #path
	while i > 0 do
		local c = string.sub(path, i, i)
		if c == "/" or c == "\\" then
			break
		end
		i -= 1
	end
	if i == 1 then
		return "/"
	end
	if i < 1 then
		return "."
	end
	return string.sub(path, 1, i - 1)
end

local function fileStem(name: string): string
	if endsWith(name, ".slang") then
		return string.sub(name, 1, #name - 6)
	end
	return name
end

local function outputPath(file: SlangFile): string
	return joinPath(currentDirectory, fileStem(file.name) .. "." .. selectedStageSuffix .. ".snb")
end

local function isDirectory(path: string): boolean
	local children = sn.Filesystem.enumerateDirectory(path)
	return #children > 0
end

local function setStage(stage: any, name: string, suffix: string)
	selectedStage = stage
	selectedStageName = name
	selectedStageSuffix = suffix
end

local function refreshDirectory()
	directories = {}
	slangFiles = {}
	selectedFile = nil
	directoryScrollY = 0.0
	fileScrollY = 0.0

	local entries = sn.Filesystem.enumerateDirectory(currentDirectory)
	table.sort(entries)

	for _, name in entries do
		local path = joinPath(currentDirectory, name)
		if endsWith(name, ".slang") then
			table.insert(slangFiles, {
				name = name,
				path = path,
			})
		elseif isDirectory(path) then
			table.insert(directories, {
				name = name,
				path = path,
			})
		end
	end

	parentDirectory = parentPath(currentDirectory)
	status = "Found " .. tostring(#slangFiles) .. " .slang file(s) in " .. currentDirectory
end

local function formatName(format: any): string
	if format == sn.ShaderFormat.SPIRV then
		return "SPIR-V"
	elseif format == sn.ShaderFormat.SPIRV_1_3 then
		return "SPIR-V 1.3"
	elseif format == sn.ShaderFormat.WGSL then
		return "WGSL"
	elseif format == sn.ShaderFormat.DXIL then
		return "DXIL"
	end
	return "Unknown"
end

local function compileEntry(file: SlangFile, format: any): sn.ShaderBundleEntry
	sn.Log.info("Compiling " .. file.path .. " (" .. selectedStageName .. ", " .. formatName(format) .. ")")
	local shader = sn.Shader.new()
	shader:compile(file.path, selectedStage, format)
	local code = shader:getCode()
	if code:size() == 0 then
		error("Shader compiled with empty code: " .. file.path)
	end
	return {
		src = file.path,
		stage = selectedStage,
		format = format,
		numSamplers = shader:getNumSamplers(),
		numStorageBuffers = shader:getNumStorageBuffers(),
		numStorageTextures = shader:getNumStorageTextures(),
		numUniformBuffers = shader:getNumUniformBuffers(),
		code = code,
	}
end

local function compileSelected()
	if selectedFile == nil then
		status = "Select a .slang file first."
		return
	end

	local formats = {}
	if includeSPIRV then
		table.insert(formats, sn.ShaderFormat.SPIRV)
	end
	if includeSPIRV13 then
		table.insert(formats, sn.ShaderFormat.SPIRV_1_3)
	end
	if includeWGSL then
		table.insert(formats, sn.ShaderFormat.WGSL)
	end
	if includeDXIL then
		table.insert(formats, sn.ShaderFormat.DXIL)
	end

	if #formats == 0 then
		status = "Enable at least one output format."
		return
	end

	local ok, message = pcall(function()
		local entries = {}
		for _, format in formats do
			table.insert(entries, compileEntry(selectedFile :: SlangFile, format))
		end

		local bundle = sn.ShaderBundle.pack(entries)
		if bundle:size() == 0 then
			error("Shader bundle is empty.")
		end

		local out = outputPath(selectedFile :: SlangFile)
		if not sn.Filesystem.write(out, bundle) then
			error("Failed to write " .. out)
		end
		status = "Wrote " .. out .. " (" .. tostring(bundle:size()) .. " bytes)"
	end)

	if not ok then
		status = "Compile failed: " .. tostring(message)
	end
end

local function itemRect(index: number, viewport: sn.Rect, height: number): sn.Rect
	return sn.Rect.new(viewport.x, viewport.y + (index - 1) * (height + 8.0), viewport.width - 12.0, height)
end

local function scrolledRect(rect: sn.Rect, scrollY: number): sn.Rect
	return sn.Rect.new(rect.x, rect.y - scrollY, rect.width, rect.height)
end

local function intersectsViewport(rect: sn.Rect, viewport: sn.Rect): boolean
	return rect.y + rect.height >= viewport.y and rect.y <= viewport.y + viewport.height
end

local function drawLabel(text: string, x: number, y: number, size: number?)
	sn.Gui.label(text, sn.Vec2.new(x, y), sn.Color.new(1, 1, 1, 0.92), size or 18)
end

local function drawDirectories(viewport: sn.Rect)
	local rowHeight = 40.0
	local contentRows = #directories + 1
	local contentHeight = contentRows * (rowHeight + 8.0)
	directoryScrollY = sn.Gui.scrollVertical(directoryScrollY, viewport, contentHeight, rowHeight)

	local parentRect = scrolledRect(itemRect(1, viewport, rowHeight), directoryScrollY)
	if intersectsViewport(parentRect, viewport) and sn.Gui.button("..  " .. parentDirectory, parentRect) then
		currentDirectory = parentDirectory
		refreshDirectory()
		return
	end

	for i, item in directories do
		local rect = scrolledRect(itemRect(i + 1, viewport, rowHeight), directoryScrollY)
		if intersectsViewport(rect, viewport) and sn.Gui.button(item.name .. "/", rect) then
			currentDirectory = item.path
			refreshDirectory()
			return
		end
	end
end

local function drawFiles(viewport: sn.Rect)
	local rowHeight = 44.0
	local contentHeight = math.max(1, #slangFiles) * (rowHeight + 8.0)
	fileScrollY = sn.Gui.scrollVertical(fileScrollY, viewport, contentHeight, rowHeight)

	for i, file in slangFiles do
		local label = file.name
		if selectedFile ~= nil and selectedFile.path == file.path then
			label = "> " .. label
		end

		local rect = scrolledRect(itemRect(i, viewport, rowHeight), fileScrollY)
		if intersectsViewport(rect, viewport) and sn.Gui.button(label, rect) then
			selectedFile = file
			status = "Selected " .. file.path
		end
	end
end

local function drawStageButton(label: string, stage: any, suffix: string, rect: sn.Rect)
	local text = label
	if selectedStage == stage then
		text = "> " .. text
	end
	if sn.Gui.button(text, rect) then
		setStage(stage, label, suffix)
	end
end

function setup()
	sn.Graphics.resetGraphicsPipeline()
	setStage(sn.ShaderStage.Fragment, "Fragment", "frag")
	refreshDirectory()
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
end

function draw()
	sn.Gui.setFont(G.font)
	sn.Gui.setFontSize(18)

	local windowSize = sn.Window.size()
	local margin = 22.0
	local top = 24.0
	local columnGap = 18.0
	local leftWidth = math.max(230.0, windowSize.x * 0.32)
	local rightX = margin + leftWidth + columnGap
	local rightWidth = math.max(260.0, windowSize.x - rightX - margin)

	drawLabel("Shader Compile", margin, top, 26)
	drawLabel("Directory: " .. currentDirectory, margin, top + 38.0, 18)

	local dirViewport = sn.Rect.new(margin, top + 78.0, leftWidth, math.max(120.0, windowSize.y - top - 106.0))
	local fileViewport = sn.Rect.new(rightX, top + 78.0, rightWidth, math.max(120.0, windowSize.y * 0.42))
	drawLabel("Directories", dirViewport.x, dirViewport.y - 28.0, 18)
	drawLabel(".slang files", fileViewport.x, fileViewport.y - 28.0, 18)
	drawDirectories(dirViewport)
	drawFiles(fileViewport)

	local controlsY = fileViewport.y + fileViewport.height + 18.0
	local buttonW = math.max(90.0, (rightWidth - 16.0) / 3.0)
	drawLabel("Stage", rightX, controlsY, 18)
	drawStageButton("Vertex", sn.ShaderStage.Vertex, "vert", sn.Rect.new(rightX, controlsY + 26.0, buttonW, 38.0))
	drawStageButton(
		"Fragment",
		sn.ShaderStage.Fragment,
		"frag",
		sn.Rect.new(rightX + buttonW + 8.0, controlsY + 26.0, buttonW, 38.0)
	)
	drawStageButton(
		"Compute",
		sn.ShaderStage.Compute,
		"comp",
		sn.Rect.new(rightX + (buttonW + 8.0) * 2.0, controlsY + 26.0, buttonW, 38.0)
	)

	local formatY = controlsY + 88.0
	local formatW = (rightWidth - 18.0) / 4.0
	includeSPIRV = sn.Gui.checkbox("SPIR-V", includeSPIRV, sn.Rect.new(rightX, formatY, formatW, 30.0))
	includeSPIRV13 =
		sn.Gui.checkbox("SPV 1.3", includeSPIRV13, sn.Rect.new(rightX + formatW + 6.0, formatY, formatW, 30.0))
	includeWGSL =
		sn.Gui.checkbox("WGSL", includeWGSL, sn.Rect.new(rightX + (formatW + 6.0) * 2.0, formatY, formatW, 30.0))
	includeDXIL =
		sn.Gui.checkbox("DXIL", includeDXIL, sn.Rect.new(rightX + (formatW + 6.0) * 3.0, formatY, formatW, 30.0))

	local output = "Output: "
	if selectedFile ~= nil then
		output ..= outputPath(selectedFile)
	else
		output ..= "(select a file)"
	end
	drawLabel(output, rightX, formatY + 48.0, 17)

	if sn.Gui.button("Compile to .snb", sn.Rect.new(rightX, formatY + 78.0, 210.0, 44.0)) then
		compileSelected()
	end
	if sn.Gui.button("Refresh", sn.Rect.new(rightX + 224.0, formatY + 78.0, 132.0, 44.0)) then
		refreshDirectory()
	end

	drawLabel(status, margin, windowSize.y - 34.0, 17)
end
