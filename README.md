# NenEngine
## 概要
 NenEngineは、主にゲーム開発のために製作中のC++20向けメディアライブラリです。  
 某ゲーム開発アニメのNeneEngineに影響を受けています。そのためにエンジンと冠していますが実際はライブラリ程度です。  
 自身のみで使用する予定のライブラリであるので、サポートやドキュメントは充実しないと思われます。  
 **ドキュメントは全て日本語です（All documentation is only in Japanese）。**

## 対応プラットフォーム
 今のところ、Windows、WebGL、Androidに対応しています。Linuxでは確認していませんが、少しの手直しで動くと思います。  

## コード例
 exampleフォルダを参照してください。

## ビルド方法
### Windows
　zipとしてダウンロード・解凍し、NenEngine/Windowsフォルダ内のslnファイルをVisual Studio（2019以降)でビルドしてください。  
 NenEngine/DebugもしくはNenEngine/Releaseフォルダが作成され、nen.libが作成されます。  
 今のところVisual Studioでのビルドのみに対応しています。MinGWでは検証していません。  
### WebGL
 ビルドにはEmscriptenのセットアップおよびCMakeが必要です。
 その後、以下のコマンドを実行してください。
 ```
 $ cd ./Emscripten  
 $ emcmake cmake .
 $ emmake make
 ```
 NenEngine/Emscripten/buildフォルダ内にlibnen.aが作成されます。
 ### Android
  上記に比べて複雑なため、ここでは割愛します。tutorialフォルダのInstall.mdを御覧ください。

## 使用ライブラリ
NenEngineで使用しているライブラリです。  
ライセンス文はlibs内の各フォルダ内に記載されています。
- Effekseer
- The OpenGL Extension Wrangler Library (GLEW)
- Dear ImGui (ImGui)
- LLGI
- Lua
- mojoAL  (実装に不具合があるようなので見送り)
- OpenAL 1.1  (mojoALが機能するまでの間)
- rapidJSON
- Simple DirectMedia Layer (SDL2)
- SDL2_image
- SDL2_mixer
- SDL2_net
- SDL2_ttf
- sol2
- tinyobjloader
  
各ライブラリはNenEngine/libsフォルダ内にライセンス文ともに格納されています。

## ライセンス
 NenEngineのライセンスはMIT Lisenceです。

## ドキュメント
製作中...