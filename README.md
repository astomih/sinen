# Sinen
Sinen is a creative coding framework for Luau/Slang
<p align="center"><a href="https://astomih.github.io/sinen"><img src="https://raw.githubusercontent.com/astomih/sinen/main/docs/logo/logo_bg_white.png" width="480" alt="Sinen Logo"></a></p>  

# Hello World

``` luau
local sn = require("@sinen")
local font: sn.Font = sn.Font.new(32)

function draw()
	sn.Graphics.drawText(
		"Hello World!",
		sn.TextStyle.new(font, sn.Color.new(1.0), 32),
		sn.TextTransform.new(sn.Window.center(), 0.0, sn.Pivot.Center)
	)
end

```

![Hello World](https://raw.githubusercontent.com/astomih/sinen/refs/heads/main/examples/screenshot/01.png)

# Games

[FOUNDRY](game/fps/README.md) is a single-player PBR FPS set in an industrial drone bay.

[RIDGELINE](game/racing/README.md) is a PBR circuit racer with analog gamepad controls.

# Documents

[![Documents](https://github.com/astomih/sinen/actions/workflows/documents.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/documents.yml)  
[https://astomih.github.io/sinen](https://astomih.github.io/sinen)
