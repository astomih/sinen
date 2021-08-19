# インストール方法
## Windows
　zipとしてダウンロード・解凍し、NenEngine/Windowsフォルダ内のslnファイルをVisual Studio（2019以降)でビルドしてください。  
 NenEngine/DebugもしくはNenEngine/Releaseフォルダが作成され、nen.libが作成されます。  
 今のところVisual Studioでのビルドのみに対応しています。MinGWでは検証していません。  
## WebGL
 ビルドにはEmscriptenのセットアップおよびCMakeが必要です。
 その後、以下のコマンドを実行してください。
 ```
 $ cd ./Emscripten  
 $ emcmake cmake .
 $ emmake make
 ```
 NenEngine/Emscripten/buildフォルダ内にlibnen.aが作成されます。
## Android
 ビルドにはAndroid SDKおよびNDK、Android Studioが必要です。  