local sn = require("@sinen")
local font = sn.Font.new(24)
local texture: sn.Texture? = nil
local selectedName = "Press O or drop an image file"
local dialogOpen = false

local function loadImage(file: sn.ExternalFile)
	selectedName = file:name()
	texture = sn.Texture.new(file)
end

local function openImageDialog()
	if dialogOpen then
		return
	end
	dialogOpen = true
	sn.FileDialog.open(function(files: { sn.ExternalFile }, errorMessage: string?)
		dialogOpen = false
		if errorMessage then
			selectedName = errorMessage
			return
		end
		if files[1] then
			loadImage(files[1])
		end
	end, {
		{ name = "Images", pattern = "png;jpg;jpeg;bmp;tga;hdr;ktx2" },
	})
end

function fileDropped(file: sn.ExternalFile)
	loadImage(file)
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.O) then
		openImageDialog()
	end
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/media/main.luau")
	end
end

function draw()
	sn.Graphics.drawText(
		selectedName,
		sn.TextStyle.new(font, sn.Color.new(1), 24),
		sn.TextTransform.new(sn.Vec2.new(24, 24))
	)

	if texture then
		local size = texture:size()
		if size.x > 0 and size.y > 0 then
			local maxWidth = math.max(1, sn.Window.size().x - 48)
			local maxHeight = math.max(1, sn.Window.size().y - 96)
			local scale = math.min(maxWidth / size.x, maxHeight / size.y, 1)
			sn.Graphics.drawImage(
				texture,
				sn.Rect.new(24, 72, size.x * scale, size.y * scale)
			)
		end
	end
end
