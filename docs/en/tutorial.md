# Sinen Tutorial

This page explains the fastest way to run a Luau app with Sinen.

## 1. Core Ideas

Sinen is based on three simple rules:

- Run `main.luau` as the entry script
- Split logic into `setup`, `update`, and `draw`
- Place assets under the runtime base directory

Here is the minimum script:

```luau
local sn = require("@sinen")
local font: sn.Font = sn.Font.new()

function setup()
	font:load(32)
end

function update()
	if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
		-- Move to another scene if needed
		-- sn.Script.load("main", ".")
	end
end

function draw()
	sn.Graphics.drawText("Hello Sinen!", font, sn.Vec2.new(0, 0), sn.Color.new(1.0), 32)
end
```

## 2. How to Run

### Windows (this repository layout)

1. Build from the project root

```bat
build\generate_msvc2026-debug.bat
build\build_msvc2026-debug.bat
```

2. Move to your project directory and run

```bat
cd examples\01_helloworld
..\..\build\msvc2026-debug\sinen.exe
```

`main.luau` will be loaded.

### Android

- `build\generate_android.bat`
- `build\build_android.bat`
- Open `build/android` in Android Studio and run
- Place your assets under `/sdcard/Android/media/astomih.sinen.app`

## 3. Lifecycle

- `setup()`: called once when a scene is loaded; use it for loading textures/models
- `update()`: called every frame; use it for input, game logic, and physics
- `draw()`: called every frame for rendering

`setup` is optional. `update` and `draw` are enough to run.

## 4. Draw an Image

```luau
local sn = require("@sinen")
local image: sn.Texture = sn.Texture.new()

function setup()
	image:load("logo.png")
end

function draw()
	sn.Graphics.drawImage(image, sn.Rect.new(sn.Vec2.new(0), image:size()))
end
```

## 5. Input and Sound

```luau
local sn = require("@sinen")
local se: sn.Sound = sn.Sound.new()

function setup()
	se:load("shot.wav")
end

function update()
	if sn.Keyboard.isPressed(sn.Keyboard.SPACE) then
		se:play()
	end
end
```

## 6. Scene Switching

Use `sn.Script.load(filePath, baseDirPath)` to switch scenes.

- `filePath`: script name without extension (for example, `main`)
- `baseDirPath`: base directory of that scene

Example: switch to `scenes/title/main.luau`

```luau
sn.Script.load("main", "scenes/title")
```

## 7. Example Directory Layout

```text
my_game/
  main.luau
  player.luau
  logo.png
  shot.wav
  scenes/
    title/
      main.luau
```

For modules in the same directory, use normal `require`.

```luau
local Player = require("./player")
```

## 8. Next Examples to Read

- `examples/01_helloworld`: minimal setup
- `examples/02_texture`: image drawing
- `examples/03_model`: 3D model drawing
- `examples/04_sound`: sound playback
- `examples/09_physics`: physics
- `examples/14_synth`: synth

For the full API, see `docs/en/luau_api.md`.
