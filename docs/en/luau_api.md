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

New drawing code creates an explicit `Render2DPass` and issues commands through that pass. Omitting the camera uses window coordinates. End the pass in the frame where it was created by passing it to `Graphics.endPass(pass)`.

```luau
local uiCamera = sn.Camera2D.new()
uiCamera:resize(sn.Vec2.new(1280, 720))

local pass = sn.Graphics.begin2DPass(uiCamera)
pass:drawText("HUD", style, transform)
sn.Graphics.endPass(pass)
```

## 3D Passes

3D drawing is scoped by an explicit `Render3DPass`. The camera and render target belong to the pass, while a reusable `Material` owns the pipeline, textures, and uniforms. `Graphics` owns GPU command buffer closure and final frame submission.

```luau
local camera = sn.Camera3D.new()
camera:lookat(sn.Vec3.new(1, 1, 3), sn.Vec3.new(0), sn.Vec3.new(0, 1, 0))

local pass = sn.Graphics.begin3DPass(camera)
pass:drawModel(model, transform)
sn.Graphics.endPass(pass)

sn.Graphics.drawText("HUD", font, sn.Vec2.new(20, 20))
```

## Materials

Group a custom pipeline and its resources in a `Material`, then pass it explicitly to the draw command. Build the `GraphicsPipeline` before constructing the material. Material contents are applied when the draw command is encoded, so one material can be updated between multiple draws.

```luau
pipeline:build()

local material = sn.Material.new(pipeline)
material:setTexture("normalMap", normalTexture)
material:setUniformBuffer("MaterialData", materialBuffer)

local pass = sn.Graphics.begin3DPass(camera)
pass:drawModel(model, transform, material)
sn.Graphics.endPass(pass)
```

Omitting the material uses the built-in pipeline and the model's base-color texture. The pass-level `setGraphicsPipeline`, `setTexture`, and `setUniformBuffer` methods remain as compatibility APIs; new code should use materials.

## Procedural 3D Models

`sn.MeshBuilder` lets Luau assemble vertices, triangles, and primitive shapes, then convert the result into a `Model`. The vertex ids returned by `vertex()` are 1-based for Luau code.

```luau
local builder = sn.MeshBuilder.new()
builder:addSphere(1.0, 16, 32, sn.Color.new(0.9, 0.4, 0.2, 1.0))

local model = builder:toModel()
local transform = sn.Transform.new()

local pass = sn.Graphics.begin3DPass(camera)
pass:drawModel(model, transform)
sn.Graphics.endPass(pass)
```

Pass a render texture as the second argument to draw offscreen.

```luau
local pass = sn.Graphics.begin3DPass(camera, renderTexture)
pass:drawModel(model, transform)
sn.Graphics.endPass(pass)
```

`begin2D` / `begin3D` / `finish` and `beginRenderTarget` / `endRenderTarget` remain available for compatibility with existing code.

## External Files

Normal asset paths remain restricted to the project or archive. Files outside the project can be read as `ExternalFile` values only after the user selects them in a file dialog or drops them onto the window. Luau never receives the absolute path itself.

```luau
sn.FileDialog.open(function(files: { sn.ExternalFile }, err: string?)
	if err then
		sn.Log.error(err)
		return
	end
	if files[1] then
		local texture = sn.Texture.new(files[1])
	end
end, {
	{ name = "Images", pattern = "png;jpg;jpeg;ktx2" },
})
```

Drag-and-drop is delivered through an optional global callback.

```luau
function fileDropped(file: sn.ExternalFile)
	local model = sn.Model.new(file)
end
```

An `ExternalFile` can be passed directly to `Texture.new`, `Model.new`, `Sound.new`, and `Font.new`, or read into a read-only `Buffer` with `read()`. Model loading does not read unselected neighboring files, so use self-contained `.glb` files or embedded data instead of a `.gltf` with external dependencies.

## Ray Tracing Support

Check `sn.Raytracing.isDeviceSupported()` before using ray tracing features. `sn.Raytracing.isSupported()` is kept as a compatibility alias and returns the same value.

```luau
if not sn.Raytracing.isDeviceSupported() then
	return
end
```

Ray query support is separate and can be checked with `sn.Raytracing.isRayQuerySupported()`. Calling any `sn.Raytracing` API other than support checks on an unsupported device raises a Lua error.

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
- `b[n], space1`: uniform buffers

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
