#include <Nen/Nen.hpp>

/**
 *  メインシーン
 * データメンバが定義出来ないので、
 * 基本的に各種設定を行って他のシーンに移るための踏み台となる
*/
void Main::Setup()
{
    //背景を黒に設定
    GetRenderer()->SetClearColor(nen::Palette::Black);

    //フォントの読み込み
    auto font = std::make_shared<nen::Font>();
    font->LoadFromFile("Assets/Font/mplus/mplus-1p-medium.ttf", 72);

    //アクターを追加
    auto actor = this->AddActor<nen::Actor>();

    //アクターにコンポーネントを追加
    auto text = actor->AddComponent<nen::TextComponent>();
    text->SetFont(font);
    text->SetString("Hello,World!", nen::Palette::White);
}

void Main::Update(float deltaTime)
{
    //キーボードのQが押されたら終了
    if (GetInput().Keyboard.GetKeyValue(nen::KeyCode::Q))
        ExitScene();

    /**
     * 描画処理は裏側で行っている
    */
}