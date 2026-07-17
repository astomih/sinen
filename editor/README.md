# Sinen ECS Editor

Run `build/msvc2026-debug/sinen.exe` with `editor/` as the working directory.
The editor itself is a Sinen Luau scene and stores its mixed 2D/3D document in an ECS world.

## Scene DSL

Scenes are ordinary Luau modules. `main.luau` loads the editable scene with:

```luau
local sceneDefinition = require("./scenes/main_scene")
local document = Scene.instantiate(sceneDefinition)
```

The scene module uses a small chainable DSL:

```luau
local Scene = require("../scene")

return Scene.define(function(scene)
	scene:entity("Player")
		:with("Transform2D", { x = 320, y = 240, rotation = 0 })
		:with("Rectangle", {
			width = 96,
			height = 64,
			color = { 0.2, 0.6, 1, 1 },
			visible = true,
		})
end)
```

3D entities use the same DSL with `Transform3D`, `Box3D`, and `Camera3D`:

```luau
scene:entity("Cube")
	:with("Transform3D", {
		position = { 0, 0, 0 },
		rotation = { 0, 30, 0 },
		scale = { 1, 1, 1 },
	})
	:with("Box3D", { color = { 0.2, 0.6, 1, 1 }, visible = true })
```

Save writes deterministic Luau source back to `scenes/main_scene.luau`. Reload clears
the module cache and reloads `main.luau`. `Ctrl+S` is the save shortcut.
