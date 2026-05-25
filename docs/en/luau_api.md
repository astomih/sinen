# [Luau API](https://github.com/astomih/sinen/blob/main/api/sinen.luau)

## 2D Coordinates

2D drawing uses a top-left origin.

- `(0, 0)` is the top-left of the window
- x grows to the right
- y grows downward
- `Rect.new(x, y, width, height)` treats `x, y` as the rectangle's top-left
- `Graphics.drawRect` and `Graphics.drawImage` draw from the `Rect` top-left
- `Graphics.drawText` uses the top-left of the rendered text as `position`

Use the `Pivot` overload of `Rect.new` when you want to construct a rectangle from another anchor such as the center or bottom-right.

```luau
local rect = sn.Rect.new(sn.Pivot.Center, sn.Vec2.new(400, 300), sn.Vec2.new(120, 80))
sn.Graphics.drawRect(rect, sn.Color.new(1, 1, 1, 1))
```

2D drawing can also be scoped with an explicit `Camera2D`. Without `begin2D`, or after `finish()`, 2D drawing uses the window as the implicit camera.

```luau
local uiCamera = sn.Camera2D.new()
uiCamera:resize(sn.Vec2.new(1280, 720))

sn.Graphics.begin2D(uiCamera)
sn.Graphics.drawText("HUD", font, sn.Vec2.new(20, 20))
sn.Graphics.finish()
```

## 3D Passes

3D drawing is scoped by an explicit camera pass. Call `Graphics.begin3D(camera)`, issue 3D draw calls, then call `Graphics.finish()` before returning to implicit 2D drawing.

```luau
local camera = sn.Camera3D.new()
camera:lookat(sn.Vec3.new(1, 1, 3), sn.Vec3.new(0), sn.Vec3.new(0, 1, 0))

sn.Graphics.begin3D(camera)
sn.Graphics.drawModel(model, transform)
sn.Graphics.finish()

sn.Graphics.drawText("HUD", font, sn.Vec2.new(20, 20))
```

## Procedural 3D Models

`sn.MeshBuilder` lets Luau assemble vertices, triangles, and primitive shapes, then convert the result into a `Model`. The vertex ids returned by `vertex()` are 1-based for Luau code.

```luau
local builder = sn.MeshBuilder.new()
builder:addSphere(1.0, 16, 32, sn.Color.new(0.9, 0.4, 0.2, 1.0))

local model = builder:toModel()
local transform = sn.Transform.new()

sn.Graphics.begin3D(camera)
sn.Graphics.drawModel(model, transform)
sn.Graphics.finish()
```

## Shader Resource Bindings

When writing Slang/HLSL shaders for Sinen, resource bindings must match the backend shader format.

### SPIR-V

Vertex shader:

- `set = 0`: sampled textures, then storage textures, then storage buffers
- `set = 1`: uniform buffers

Fragment shader:

- `set = 2`: sampled textures, then storage textures, then storage buffers
- `set = 3`: uniform buffers

Compute shader:

- `set = 0`: storage buffers
- `set = 1`: uniform buffers

Ray tracing pipeline shaders:

- `set = 4`: acceleration structures first, then storage buffers
- `set = 5`: uniform buffers

Ray query in graphics or compute shaders:

- `set = 6`: acceleration structures

Example:

```hlsl
[[vk::binding(0, 6)]]
RaytracingAccelerationStructure scene;

[[vk::binding(0, 0)]]
RWByteAddressBuffer outputPixels;
```

### DXBC / DXIL

Vertex shader:

- `t[n], space0`: sampled textures, then storage textures, then storage buffers
- `s[n], space0`: samplers with matching sampled-texture indices
- `b[n], space1`: uniform buffers

Pixel shader:

- `t[n], space2`: sampled textures, then storage textures, then storage buffers
- `s[n], space2`: samplers with matching sampled-texture indices
- `b[n], space3`: uniform buffers

Compute shader:

- `u[n], space0`: storage buffers
- `b[n], space0`: uniform buffers

Ray tracing pipeline shaders:

- `t[n], space4`: acceleration structures
- `u[n], space4`: storage buffers
- `b[n], space5`: uniform buffers

Ray query in graphics or compute shaders:

- `t[n], space6`: acceleration structures

Example:

```hlsl
RaytracingAccelerationStructure scene : register(t0, space6);
RWByteAddressBuffer outputPixels : register(u0, space0);
```

## Immediate GUI

`sn.Gui` provides a small immediate-mode GUI layer. Call widgets every frame from `draw()`; each widget draws itself and returns its new interaction state.

```luau
local enabled = false
local volume = 0.5

function draw()
	if sn.Gui.button("Play", sn.Rect.new(20, 20, 120, 32)) then
		print("clicked")
	end

	enabled = sn.Gui.checkbox("Enabled", enabled, sn.Rect.new(20, 64, 160, 28))
	volume = sn.Gui.sliderFloat("Volume", volume, 0.0, 1.0, sn.Rect.new(20, 104, 240, 28))
end
```

For scrollable lists, keep the scroll offset in Luau and update it with the value returned by `scrollVertical()`.

```luau
local scrollY = 0.0
local viewport = sn.Rect.new(20, 80, 360, 480)

function draw()
	scrollY = sn.Gui.scrollVertical(scrollY, viewport, 900, 48)
end
```

## Video API

`sn.VideoWriter` and `sn.VideoReader` write and read video files. The current implementation uses a simple AVI container without an external codec dependency.

```luau
local writer = sn.VideoWriter.new()
if writer:open("capture.avi", 640, 360, 30) then
	-- pixels is an RGB8/RGBA8/BGR8/BGRA8 sn.Buffer
	writer:addFrame(pixels, 640, 360, "rgba8", 90)
	writer:close()
end

local reader = sn.VideoReader.new()
if reader:open("capture.avi") then
	local info = reader:info()
	local frame = reader:readFrame(0)
	reader:close()
end
```
