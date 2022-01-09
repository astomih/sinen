[![CMake](https://github.com/astomih/sinen/actions/workflows/cmake.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/cmake.yml)
# sinen
## 概要
 sinenは、主にゲーム開発のために製作中のC++20向けメディアライブラリです。  
 自身のみで使用する予定のライブラリであるので、サポートやドキュメントは充実しないと思われます。  
 **ドキュメントは全て日本語です（All documentation is only in Japanese）。**

## 対応プラットフォーム
 今のところ、Windows、WebGL、Androidに対応しています。Linuxでは確認していませんが、少しの手直しで動くと思います。  

## サンプルコード
 exampleフォルダを参照してください。
``` c++
#include <Nen/Nen.hpp>

/**
 *  メインシーン
 * データメンバが定義出来ないので、
 * 基本的に各種設定を行って他のシーンに移るための踏み台となる
*/
void Main::Setup()
{
    //背景を黒に設定
    GetRenderer()->SetClearColor(nen::palette::Black);

    //フォントの読み込み
    auto f = std::make_shared<nen::font>();
    f->LoadFromFile("Assets/Font/mplus/mplus-1p-medium.ttf", 72);

    //アクターを追加
    auto actor = this->AddActor<nen::base_actor>();

    //アクターにコンポーネントを追加
    auto text = actor->AddComponent<nen::text_component>();
    text->SetFont(font);
    text->SetString("Hello,World!", nen::palette::White);
}

void Main::Update(float deltaTime)
{
    //キーボードのQが押されたら終了
    if (GetInput().Keyboard.GetKeyValue(nen::key_code::Q))
        Quit();

    /**
     * 描画処理は裏側で行っている
    */
}
```
![結果](https://github.com/Astomih/sinen/blob/main/example/result.png "result")

## ビルド方法
### Windows
 MinGWのみ確認しています。MSVCでも出来ると思いますが、確認はしていません。
### WebGL
 ビルドにはEmscriptenのセットアップおよびCMakeが必要です。
 その後、以下のコマンドを実行してください。
 ```
 $ cd ./Emscripten  
 $ emcmake cmake .
 $ emmake make
 ```
 sinen/Emscripten/buildフォルダ内にlibnen.aが作成されます。
 ### Android
  上記に比べて複雑なため、ここでは割愛します。tutorialフォルダのInstall.mdを御覧ください。

## 使用ライブラリ
sinenで使用しているライブラリです。  
ライセンス文はlibs内の各フォルダ内に記載されています。
- GLEW
- ImGui
- Lua
- Sol2
- OpenAL 1.1
- rapidJSON
- SDL2
- SDL2_image
- SDL2_mixer
- SDL2_net
- SDL2_ttf
  
各ライブラリはsinen/libsフォルダ内にライセンス文ともに格納されています。

## ライセンス
 MIT Lisence

## ドキュメント
[APIリファレンス](https://astomih.github.io/sinen)  
その他については製作中。。。