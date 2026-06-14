# Sinen Tutorial

This page explains the fastest way to run a Luau app with Sinen.

## 1. Core Ideas

Sinen is based on three simple rules:

- Run `main.snb` or `main.luau` as the entry script
- Split logic into `setup`, `update`, and `draw`
- Place assets under the runtime base directory

2D drawing uses Processing/raylib-style coordinates. `sn.Vec2.new(0, 0)` is the top-left of the screen, x grows to the right, and y grows downward. `sn.Rect.new(x, y, width, height)` also treats `x, y` as the rectangle's top-left corner.

Here is the minimum script:

```luau
local sn = require("@sinen")
local font: sn.Font = sn.Font.new()
font:load(32)

function draw()
	sn.Graphics.drawText(
		"Hello World!",
		sn.TextStyle.new(font, sn.Color.new(1.0), 32),
		sn.TextTransform.new(sn.Window.center(), 0.0, sn.Pivot.Center)
	)
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
cd examples\basics\01_helloworld
..\..\..\build\msvc2026-debug\sinen.exe
```

`main.snb` is loaded when present; otherwise `main.luau` is loaded.

You can also run a `.sna` archive. A `.sna` file is a zip archive whose root
contains a `sna/` directory with `main.luau` and its assets inside.
If `main.snb` is present, it takes priority over `main.luau`.

```text
hoge.sna
└─ sna
   ├─ main.luau
   └─ logo.png
```

```bat
build\msvc2026-debug\sinen.exe hoge.sna
```

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
	sn.Graphics.drawImage(image, sn.Rect.new(20, 20, image:size().x, image:size().y))
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

Use `sn.Script.load(path)` to switch scenes.

- `path`: script path with a `.luau` or `.snb` extension (for example, `scenes/title/main.luau`)
- Relative paths are based on the current scene file; paths starting with `/` are based on the root scene file

Example: switch to `scenes/title/main.luau`

```luau
sn.Script.load("/scenes/title/main.luau")
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