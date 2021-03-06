---
layout: page
title: Sinen Lua APIマニュアル
subtitle: 
---
# 目次
1. [はじめに](#1)
2. [Sinenとは？](#2)
3. [対象OS](#3)
4. [Sinenを使うと何が出来る？](#4)
5. [将来的に出来ること](#5)
6. [Sinenでハローワールド](#6)
  
<a id="1"></a>  
  
# はじめに
<a id="2"></a>

## Sinenとは？
Sinenはゲーム開発のためのエンジンです。  
Lua言語で直感的に書けるようになることが目標です。  
Web向けのクライアントが、[ここ](https://astomih.github.io/sinen_app/)にあります。
<a id="3"></a>
  
## 対象OS
- Windows
- Linux
- Android
- Web  
  
Apple製品でも動くと思いますが、未確認です。
  
<a id="4"></a>
  
## Sinenを使うと何が出来る？
- 2D/3Dのオブジェクトの描画
- フォントのレンダリング
- キーボードやマウス、ゲームパッド等の入力検出
- BGMの再生
- 効果音の再生  
- 衝突検知
- 任意のモデルデータの読み込み
  
など
<a id="6"></a>

## Sinenでハローワールド
Sinenを使用するには、ビルドが必要です。Windowsであればビルド済みのバイナリを配布しています。
ゲームか否かに関わらず、プログラムの入門時に画面に「Hello, World!」と表示する習慣があります。「プログラムで画面に文字を表示する」なら「あ」でも何でも良いのですが、とりあえずここは先人の思念に倣ってHello,Sinen World!と表示してみます。  
``` lua
local hello_texture = {}
local hello_font = {}
local hello_drawer = {}

function setup()
	hello_texture = texture()
	hello_drawer = draw2d(hello_texture)
	hello_drawer.scale = vector2(1, 1)
	hello_font = font()
	hello_font:load(DEFAULT_FONT, 128)
	hello_font:render_text(hello_texture, "Hello Sinen World!", color(1, 1, 1, 1))
end

function update() 
	hello_drawer:draw()
end
```
# API リファレンス
C++のAPIは[https://astomih.github.io/sinen/](https://astomih.github.io/sinen/)を参照して下さい。
