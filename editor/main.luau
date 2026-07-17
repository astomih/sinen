--!strict

local sn = require("@sinen")
local Components: any = nil
local Scene: any = nil
local document: any = nil
local loadError: string? = nil

local loaded, loadResult = pcall(function()
	local components = require("./components")
	local scene = require("./scene")
	local sceneDefinition = require("./scenes/main_scene")
	if type(scene) ~= "table" then
		error("scene module load failed: " .. tostring(scene))
	end
	return {
		components = components,
		scene = scene,
		document = scene.instantiate(sceneDefinition),
	}
end)
if loaded then
	Components = loadResult.components
	Scene = loadResult.scene
	document = loadResult.document
else
	loadError = tostring(loadResult)
end

local font = sn.Font.new(18)
local selected: sn.Entity? = if document and document.entities[1] then document.entities[1].entity else nil
local nextEntityNumber = if document then #document.entities + 1 else 1
local status = "Ready"
local statusTime = 0.0
local hierarchyScroll = 0.0

local background = sn.Color.new(0.055, 0.065, 0.085, 1)
local panel = sn.Color.new(0.085, 0.1, 0.13, 1)
local canvas = sn.Color.new(0.035, 0.042, 0.055, 1)
local accent = sn.Color.new(0.25, 0.65, 1.0, 1)
local muted = sn.Color.new(0.62, 0.68, 0.76, 1)
local viewportCamera = sn.Camera3D.new()
local modelCache: { [string]: sn.Model } = {}
local transformCache: { [number]: sn.Transform } = {}

local function setStatus(message: string)
	status = message
	statusTime = 3.0
end

local function selectedRecord(): any?
	if selected == nil then
		return nil
	end
	return document.byIndex[selected.index]
end

local function saveScene()
	if Scene.write("scenes/main_scene.luau", document, "../scene") then
		setStatus("Saved scenes/main_scene.luau")
	else
		setStatus("Save failed")
	end
end

local function reloadScene()
	sn.Script.clearRequireCache()
	sn.Script.load("/main.luau")
end

local function addEntity2D()
	local record = document:add("Entity " .. tostring(nextEntityNumber), {
		Transform2D = {
			x = 360,
			y = 260,
			rotation = 0,
		},
		Rectangle = {
			width = 100,
			height = 70,
			color = { 0.35, 0.75, 0.5, 1 },
			visible = true,
		},
	})
	nextEntityNumber += 1
	selected = record.entity
	setStatus("Entity added")
end

local function addEntity3D()
	local offset = (nextEntityNumber % 5) - 2
	local record = document:add("3D Entity " .. tostring(nextEntityNumber), {
		Transform3D = {
			position = { offset * 0.8, 0, 0 },
			rotation = { 0, 0, 0 },
			scale = { 1, 1, 1 },
		},
		Box3D = {
			color = { 0.35, 0.75, 0.5, 1 },
			visible = true,
		},
	})
	nextEntityNumber += 1
	selected = record.entity
	setStatus("3D entity added")
end

local function removeSelected()
	local record = selectedRecord()
	if record and document:remove(record.entity) then
		selected = if document.entities[1] then document.entities[1].entity else nil
		setStatus("Entity deleted")
	end
end

function setup()
	sn.Window.rename("Sinen ECS Editor")
	sn.Window.resize(sn.Vec2.new(1200, 760))
	sn.Graphics.setClearColor(background)
	sn.Gui.setThemeColor(
		sn.Color.new(0.12, 0.14, 0.18, 1),
		sn.Color.new(0.18, 0.22, 0.29, 1),
		sn.Color.new(0.12, 0.48, 0.82, 1),
		sn.Color.new(0.94, 0.96, 1, 1),
		accent
	)
end

function update()
	if loadError then
		return
	end
	if statusTime > 0 then
		statusTime -= sn.Time.delta()
	end
	if (sn.Keyboard.isDown(sn.Scancode.LCTRL) or sn.Keyboard.isDown(sn.Scancode.RCTRL))
		and sn.Keyboard.isPressed(sn.Scancode.S)
	then
		saveScene()
	end
end

local function modelForColor(color: { number }): sn.Model
	local key = string.format("%.1f:%.1f:%.1f:%.1f", color[1], color[2], color[3], color[4])
	local model = modelCache[key]
	if model == nil then
		local builder = sn.MeshBuilder.new()
		builder:addBox(1, sn.Color.new(color[1], color[2], color[3], color[4]))
		model = builder:toModel()
		modelCache[key] = model
	end
	return model
end

local function draw3DViewport(canvasRect: sn.Rect): sn.Entity?
	local cameraConfig: any = nil
	for _, candidate in document.world:query(Components.Camera3D) do
		if candidate.active then
			cameraConfig = candidate
			break
		end
	end
	if cameraConfig == nil then
		cameraConfig = Components.defaults.Camera3D
	end

	local position = cameraConfig.position
	local target = cameraConfig.target
	viewportCamera:lookat(
		sn.Vec3.new(position[1], position[2], position[3]),
		sn.Vec3.new(target[1], target[2], target[3]),
		sn.Vec3.new(0, 1, 0)
	)
	local windowSize = sn.Window.size()
	viewportCamera:perspective(cameraConfig.fov, windowSize.x / math.max(windowSize.y, 1), cameraConfig.near, cameraConfig.far)

	sn.Graphics.begin3D(viewportCamera)
	for entity, transform3D, box in document.world:query(Components.Transform3D, Components.Box3D) do
		if box.visible then
			local transform = transformCache[entity.index]
			if transform == nil then
				transform = sn.Transform.new()
				transformCache[entity.index] = transform
			end
			local p = transform3D.position
			local r = transform3D.rotation
			local s = transform3D.scale
			transform.position = sn.Vec3.new(p[1], p[2], p[3])
			transform.rotation = sn.Vec3.new(r[1], r[2], r[3])
			transform.scale = sn.Vec3.new(s[1], s[2], s[3])
			sn.Graphics.drawModel(modelForColor(box.color), transform)
		end
	end
	sn.Graphics.finish()

	if not sn.Mouse.isPressed(sn.Mouse.LEFT) then
		return nil
	end
	local mouse = sn.Mouse.getPosition()
	if mouse.x < canvasRect.x
		or mouse.x > canvasRect.x + canvasRect.width
		or mouse.y < canvasRect.y
		or mouse.y > canvasRect.y + canvasRect.height
	then
		return nil
	end

	local closest: sn.Entity? = nil
	local closestDistance = 48.0
	for entity, transform3D in document.world:query(Components.Transform3D) do
		local p = transform3D.position
		local screen = viewportCamera:worldToScreen(sn.Vec3.new(p[1], p[2], p[3]), windowSize)
		local distance = math.sqrt((screen.x - mouse.x) ^ 2 + (screen.y - mouse.y) ^ 2)
		if distance < closestDistance then
			closestDistance = distance
			closest = entity
		end
	end
	return closest
end

local function drawCanvas(canvasRect: sn.Rect)
	sn.Graphics.drawRect(canvasRect, canvas)
	local hit3D = draw3DViewport(canvasRect)
	if hit3D then
		selected = hit3D
	end
	local mouse = sn.Mouse.getPosition()
	local clicked = sn.Mouse.isPressed(sn.Mouse.LEFT)
	local hit: sn.Entity? = nil

	for entity, transform, rectangle in document.world:query(Components.Transform2D, Components.Rectangle) do
		if rectangle.visible then
			local x = canvasRect.x + transform.x
			local y = canvasRect.y + transform.y
			local rect = sn.Rect.new(sn.Pivot.Center, x, y, rectangle.width, rectangle.height)
			if selected == entity then
				sn.Graphics.drawRect(
					sn.Rect.new(sn.Pivot.Center, x, y, rectangle.width + 8, rectangle.height + 8),
					accent,
					transform.rotation
				)
			end
			local color = rectangle.color
			sn.Graphics.drawRect(rect, sn.Color.new(color[1], color[2], color[3], color[4]), transform.rotation)

			if clicked
				and mouse.x >= rect.x
				and mouse.x <= rect.x + rect.width
				and mouse.y >= rect.y
				and mouse.y <= rect.y + rect.height
			then
				hit = entity
			end
		end
	end
	if hit then
		selected = hit
	end
end

local function drawHierarchy(rect: sn.Rect)
	sn.Graphics.drawRect(rect, panel)
	sn.Gui.label("HIERARCHY", sn.Vec2.new(rect.x + 14, rect.y + 14), muted, 15)
	local viewport = sn.Rect.new(rect.x + 10, rect.y + 42, rect.width - 20, rect.height - 52)
	local rowHeight = 36
	hierarchyScroll = sn.Gui.scrollVertical(hierarchyScroll, viewport, #document.entities * rowHeight, rowHeight)
	for index, record in ipairs(document.entities) do
		local row = sn.Rect.new(viewport.x, viewport.y + (index - 1) * rowHeight - hierarchyScroll, viewport.width, 30)
		if row.y + row.height >= viewport.y and row.y <= viewport.y + viewport.height then
			local prefix = if selected == record.entity then "* " else "  "
			if sn.Gui.button(prefix .. record.name, row) then
				selected = record.entity
			end
		end
	end
end

local function slider(label: string, value: number, minimum: number, maximum: number, x: number, y: number, width: number): number
	return sn.Gui.sliderFloat(label, value, minimum, maximum, sn.Rect.new(x, y, width, 28))
end

local function drawInspector(rect: sn.Rect)
	sn.Graphics.drawRect(rect, panel)
	sn.Gui.label("INSPECTOR", sn.Vec2.new(rect.x + 14, rect.y + 14), muted, 15)
	local record = selectedRecord()
	if record == nil then
		sn.Gui.label("No entity selected", sn.Vec2.new(rect.x + 14, rect.y + 52), muted, 16)
		return
	end

	local x = rect.x + 14
	local width = rect.width - 28
	local y = rect.y + 50
	sn.Gui.label(record.name, sn.Vec2.new(x, y), nil, 20)
	y += 40

	local camera3D = record.components.Camera3D
	if camera3D then
		sn.Gui.label("Camera3D", sn.Vec2.new(x, y), accent, 17)
		y += 28
		camera3D.active = sn.Gui.checkbox("Active", camera3D.active, sn.Rect.new(x, y, width, 28))
		y += 34
		camera3D.position[1] = slider("Position X", camera3D.position[1], -20, 20, x, y, width)
		y += 34
		camera3D.position[2] = slider("Position Y", camera3D.position[2], -20, 20, x, y, width)
		y += 34
		camera3D.position[3] = slider("Position Z", camera3D.position[3], -20, 20, x, y, width)
		y += 34
		camera3D.target[1] = slider("Target X", camera3D.target[1], -10, 10, x, y, width)
		y += 34
		camera3D.target[2] = slider("Target Y", camera3D.target[2], -10, 10, x, y, width)
		y += 34
		camera3D.target[3] = slider("Target Z", camera3D.target[3], -10, 10, x, y, width)
		y += 34
		camera3D.fov = slider("Field of view", camera3D.fov, 20, 100, x, y, width)
		return
	end

	local transform3D = record.components.Transform3D
	local box3D = record.components.Box3D
	if transform3D or box3D then
		if transform3D == nil then
			if sn.Gui.button("+ Transform3D", sn.Rect.new(x, y, width, 30)) then
				transform3D = document:addComponent(record, "Transform3D")
			end
			y += 40
		else
			sn.Gui.label("Transform3D", sn.Vec2.new(x, y), accent, 17)
			y += 28
			transform3D.position[1] = slider("Position X", transform3D.position[1], -10, 10, x, y, width)
			y += 34
			transform3D.position[2] = slider("Position Y", transform3D.position[2], -10, 10, x, y, width)
			y += 34
			transform3D.position[3] = slider("Position Z", transform3D.position[3], -10, 10, x, y, width)
			y += 34
			transform3D.rotation[1] = slider("Rotation X", transform3D.rotation[1], -180, 180, x, y, width)
			y += 34
			transform3D.rotation[2] = slider("Rotation Y", transform3D.rotation[2], -180, 180, x, y, width)
			y += 34
			transform3D.rotation[3] = slider("Rotation Z", transform3D.rotation[3], -180, 180, x, y, width)
			y += 34
			transform3D.scale[1] = slider("Scale X", transform3D.scale[1], 0.1, 5, x, y, width)
			y += 34
			transform3D.scale[2] = slider("Scale Y", transform3D.scale[2], 0.1, 5, x, y, width)
			y += 34
			transform3D.scale[3] = slider("Scale Z", transform3D.scale[3], 0.1, 5, x, y, width)
			y += 40
		end

		if box3D == nil then
			if sn.Gui.button("+ Box3D", sn.Rect.new(x, y, width, 30)) then
				box3D = document:addComponent(record, "Box3D")
			end
		else
			sn.Gui.label("Box3D", sn.Vec2.new(x, y), accent, 17)
			y += 28
			box3D.visible = sn.Gui.checkbox("Visible", box3D.visible, sn.Rect.new(x, y, width, 28))
			y += 34
			box3D.color[1] = slider("Red", box3D.color[1], 0, 1, x, y, width)
			y += 34
			box3D.color[2] = slider("Green", box3D.color[2], 0, 1, x, y, width)
			y += 34
			box3D.color[3] = slider("Blue", box3D.color[3], 0, 1, x, y, width)
		end
		return
	end

	local transform = record.components.Transform2D
	if transform == nil then
		if sn.Gui.button("+ Transform2D", sn.Rect.new(x, y, width, 30)) then
			document:addComponent(record, "Transform2D")
		end
		y += 40
	else
		sn.Gui.label("Transform2D", sn.Vec2.new(x, y), accent, 17)
		y += 28
		transform.x = slider("X", transform.x, 0, 900, x, y, width)
		y += 34
		transform.y = slider("Y", transform.y, 0, 650, x, y, width)
		y += 34
		transform.rotation = slider("Rotation", transform.rotation, -math.pi, math.pi, x, y, width)
		y += 42
	end

	local rectangle = record.components.Rectangle
	if rectangle == nil then
		if sn.Gui.button("+ Rectangle", sn.Rect.new(x, y, width, 30)) then
			document:addComponent(record, "Rectangle")
		end
	else
		sn.Gui.label("Rectangle", sn.Vec2.new(x, y), accent, 17)
		y += 28
		rectangle.visible = sn.Gui.checkbox("Visible", rectangle.visible, sn.Rect.new(x, y, width, 28))
		y += 34
		rectangle.width = slider("Width", rectangle.width, 8, 400, x, y, width)
		y += 34
		rectangle.height = slider("Height", rectangle.height, 8, 400, x, y, width)
		y += 34
		rectangle.color[1] = slider("Red", rectangle.color[1], 0, 1, x, y, width)
		y += 34
		rectangle.color[2] = slider("Green", rectangle.color[2], 0, 1, x, y, width)
		y += 34
		rectangle.color[3] = slider("Blue", rectangle.color[3], 0, 1, x, y, width)
	end
end

function draw()
	sn.Gui.setFont(font)
	sn.Gui.setFontSize(17)
	local size = sn.Window.size()
	if loadError then
		sn.Graphics.drawRect(sn.Rect.new(0, 0, size.x, size.y), background)
		sn.Gui.label("SCENE LOAD ERROR", sn.Vec2.new(28, 28), sn.Color.new(1, 0.3, 0.3, 1), 22)
		local line = 0
		for start = 1, #loadError, 55 do
			sn.Gui.label(string.sub(loadError, start, start + 54), sn.Vec2.new(28, 72 + line * 20), nil, 12)
			line += 1
		end
		return
	end
	local toolbarHeight = 54
	local margin = 10
	local hierarchyWidth = 230
	local inspectorWidth = 300

	sn.Graphics.drawRect(sn.Rect.new(0, 0, size.x, toolbarHeight), panel)
	if sn.Gui.button("+ 2D", sn.Rect.new(margin, 10, 78, 34)) then
		addEntity2D()
	end
	if sn.Gui.button("+ 3D", sn.Rect.new(96, 10, 78, 34)) then
		addEntity3D()
	end
	if sn.Gui.button("Delete", sn.Rect.new(182, 10, 86, 34)) then
		removeSelected()
	end
	if sn.Gui.button("Save", sn.Rect.new(276, 10, 78, 34)) then
		saveScene()
	end
	if sn.Gui.button("Reload", sn.Rect.new(362, 10, 86, 34)) then
		reloadScene()
	end
	sn.Gui.label("Sinen ECS Editor 2D / 3D", sn.Vec2.new(470, 17), nil, 18)
	if statusTime > 0 then
		sn.Gui.label(status, sn.Vec2.new(size.x - 300, 18), muted, 15)
	end

	local contentY = toolbarHeight + margin
	local contentHeight = size.y - contentY - margin
	local hierarchyRect = sn.Rect.new(margin, contentY, hierarchyWidth, contentHeight)
	local inspectorRect = sn.Rect.new(size.x - inspectorWidth - margin, contentY, inspectorWidth, contentHeight)
	local canvasRect = sn.Rect.new(
		hierarchyRect.x + hierarchyRect.width + margin,
		contentY,
		inspectorRect.x - hierarchyRect.x - hierarchyRect.width - margin * 2,
		contentHeight
	)

	drawCanvas(canvasRect)
	drawHierarchy(hierarchyRect)
	drawInspector(inspectorRect)
end
