# Lua API
mainブランチと乖離している場合があります。  
型情報は、sinen.luaファイルを参照してください。
## 呼び出しについて
Luaスクリプトは`sinen.exeが存在するディレクトリ/script/`に配置します  
```lua

-- セットアップ

function Update()
 -- 毎フレーム呼び出される関数
end

function Draw()
  -- 毎フレーム呼び出される描画関数
end
```
## クラス
Luaにはクラスはありませんが、テーブルで代用します  
Sinenでは、あらかじめ幾つかのクラスを用意しています
### 命名規則
Sinenでは、以下の命名規則を使用しています  
- クラス名: CamelCase(例: `Vec2`, `Vec3`, `Color`)
- メソッド名: CamelCase(例: `Length`, `Normalize`, `Copy`)
- プロパティ名: lowerCase(例: `x`, `y`, `z`, `r`, `g`, `b`, `a`)
インスタンスの生成はクラス名()で行います(例: `a = Vec2(1, 2)`)  
インスタンス内の関数はコロン(:)で呼び出し可能です(例: `a:Length()`)  
### Vec2
#### メタテーブル
- `__add`: ベクトルの加算を行います (a + b)
- `__sub`: ベクトルの減算を行います (a - b)
- `__mul`: ベクトルの乗算を行います (a * b)
- `__div`: ベクトルの除算を行います (a / b)
#### Vec2(x, y) -> Vec2
- `x`: ベクトルのx成分
- `y`: ベクトルのy成分  
Vec2オブジェクトを作成します。
#### Vec2(value) -> Vec2
- `value`: 数値またはVec2オブジェクト
Vec2オブジェクトをx, y成分に同じvalueを持つベクトルとして作成します 
#### Vec2.x = number
Vec2のx成分。
#### Vec2.y = number
Vec2のy成分。
#### Vec2:Length() -> number
ベクトルの長さを取得します
#### Vec2:Normalize() -> Vec2
ベクトルを正規化します
#### Vec2:Copy() -> Vec2
ベクトルをコピーします
#### Vec2:Dot(Vec2, Vec2) -> number
- `Vec2`: ベクトル1
- `Vec2`: ベクトル2
内積を取得します
#### Vec2:Lerp(Vec2, Vec2, number) -> Vec2
- `Vec2`: ベクトル1
- `Vec2`: ベクトル2
- `number`: 補間係数
線形補間を取得します
#### Vec2:Reflect(Vec2, Vec2) -> Vec2
- `Vec2`: ベクトル
- `Vec2`: 法線ベクトル
反射ベクトルを取得します
### Vec3
#### メタテーブル
- `__add`: ベクトルの加算を行います (a + b)
- `__sub`: ベクトルの減算を行います (a - b)
- `__mul`: ベクトルの乗算を行います (a * b)
- `__div`: ベクトルの除算を行います (a / b)
#### Vec3(x, y, z) -> Vec3
- `x`: ベクトルのx成分
- `y`: ベクトルのy成分
- `z`: ベクトルのz成分  
Vec3オブジェクトを作成します
#### Vec3(value) -> Vec3
- `value`: 数値またはVec3オブジェクト
Vec3オブジェクトをx, y, z成分に同じvalueを持つベクトルとして作成します
#### Vec3.x = number
Vec3のx成分
#### Vec3.y = number
Vec3のy成分
#### Vec3.z = number
Vec3のz成分
#### Vec3:Length() -> number
ベクトルの長さを取得します
#### Vec3:Normalize() -> Vec3
ベクトルを正規化します
#### Vec3:Copy() -> Vec3
ベクトルをコピーします  
Luaでは基本的に参照渡しのため、コピーが必要な場合に使用します
#### Vec3:Forward(Vec3) -> Vec3
- `Vec3`: 軸ベクトル  
前方ベクトルを取得します
#### Vec3:Dot(Vec3, Vec3) -> number
- `Vec3`: ベクトル1
- `Vec3`: ベクトル2
内積を取得します
#### Vec3:Cross(Vec3, Vec3) -> Vec3
- `Vec3`: ベクトル1
- `Vec3`: ベクトル2
外積を取得します
#### Vec3:Lerp(Vec3, Vec3, number) -> Vec3
- `Vec3`: ベクトル1
- `Vec3`: ベクトル2
- `number`: 補間係数
線形補間を取得します
#### Vec3:Reflect(Vec3, Vec3) -> Vec3
- `Vec3`: ベクトル
- `Vec3`: 法線ベクトル
反射ベクトルを取得します
### Color
#### Color() -> Color
Colorオブジェクトを作成します
#### Color(r, g, b, a) -> color
- `r`: 色の赤成分(0.0~1.0)
- `g`: 色の緑成分(0.0~1.0)
- `b`: 色の青成分(0.0~1.0)
- `a`: 色のアルファ成分(0.0~1.0)  
色オブジェクトを作成します
#### Color.r = number
色の赤成分(0.0~1.0)
#### Color.g = number
色の緑成分(0.0~1.0)
#### Color.b = number
色の青成分(0.0~1.0)
#### Color.a = number
色のアルファ成分(0.0~1.0)
### Texture
#### Texture()
テクスチャオブジェクトを作成します
#### Texture:Load(string)
- `string`: data/texture/にあるテクスチャファイルの名前  
テクスチャファイルを読み込みます
#### Texture:FillColor(Color)
- `Color`: テクスチャを塗りつぶす色  
テクスチャを塗りつぶします
#### Texture:BlendColor(Color)
- `Color`: テクスチャをブレンドする色  
テクスチャをブレンドします
#### Texture:Copy() -> Texture
テクスチャをコピーします  
Luaでは基本的に参照渡しのため、コピーが必要な場合に使用します
#### Texture:Size() -> Vec2
テクスチャのサイズを取得します
### RenderTexture
#### RenderTexture()
レンダーテクスチャオブジェクトを作成します
#### RenderTexture:Create(number, number)
- `number`: 幅
- `number`: 高さ  
レンダーテクスチャを作成します
### Material
#### Material() -> Material
マテリアルオブジェクトを作成します
#### Material:AppendTexture(Texture)
- `Texture`: テクスチャ  
テクスチャをマテリアルに追加します
### Font
#### Font() -> Font
フォントオブジェクトを作成します
#### Font:Load(string, number)
- `string`: data/font/にあるフォントファイルの名前
- `number`: フォントのサイズ(ピクセル)
.ttfファイルを読み込みます  
#### Font:RenderText(Texture, string, color)
- `Texture`: 描画先のテクスチャ
- `string`: 描画する文字列
- `Color`: テキストの色
CPUでテキストを描画します  
テクスチャは描画結果に合わせてリサイズされます
### Draw2D
#### Draw2D() -> Draw2D
Draw2Dオブジェクトを作成します  
#### Draw2D.position = Vec2
位置情報  
Sinenのウィンドウの中心が(0, 0)です
#### Draw2D.scale = Vec2
拡大率
#### Draw2D.rotation = number
時計回りの回転角度
#### Draw2D.material = Material
マテリアル
#### Draw2D.model = Model
モデル
#### Draw2D:Draw()
2Dオブジェクトを描画します
#### Draw2D:Add(Vec2, number, Vec2)
- `Vec2`: 位置情報
- `number`: 時計回りの回転角度
- `Vec2`: 拡大率  
同じテクスチャ、モデルで複数のオブジェクトを追加して描画します  
インスタンス生成時点で既に一つのオブジェクトが存在しています
#### Draw2D:Clear()
追加されたオブジェクトをクリアします  
ただし、インスタンス生成時点で存在しているオブジェクトはクリアされません
### Draw3D
#### Draw3D() -> Draw3D
Draw3Dオブジェクトを作成します  
#### Draw3D.position = Vec3
位置情報(ワールド座標)
#### Draw3D.scale = Vec3
拡大率
#### Draw3D.rotation = Vec3
回転角度(オイラー角)
#### Draw3D.material = Material
マテリアル
#### Draw3D.model = Model
モデル
#### Draw3D:Draw()
3Dオブジェクトを描画します
#### Draw3D:Add(Vec3,Vec3,Vec3)
- `Vec3`: 位置情報
- `Vec3`: 回転角度
- `Vec3`: 拡大率  
同じテクスチャ、モデルで複数のオブジェクトを追加して描画します  
インスタンス生成時点で既に一つのオブジェクトが存在しています
#### Draw3D:Clear()
追加されたオブジェクトをクリアします  
ただし、インスタンス生成時点で存在しているオブジェクトはクリアされません
### UniformData
#### UniformData() -> UniformData
ユニフォームデータオブジェクトを作成します
#### UniformData:Add(number)
- `number`: データ  
ユニフォームデータを追加します
#### UniformData:At(number, number)
- `number`: データ
- `number`: インデックス  
ユニフォームデータを指定したインデックスに追加します
### Shader
#### Shader() -> Shader
シェーダオブジェクトを作成します
#### Shader:LoadVertexShader(string,number)
- `string`: data/shader/にある頂点シェーダファイルの名前
- `number`: UniformDataの数  
頂点シェーダを読み込みます
#### Shader:LoadFragmentShader(string,number)
- `string`: data/shader/にあるフラグメントシェーダファイルの名前
- `number`: UniformDataの数  
フラグメントシェーダを読み込みます
#### Shader:CompileAndLoadVertexShader(string,number)
- `string`: 頂点シェーダのソースコード
- `number`: UniformDataの数
頂点シェーダをコンパイルして読み込みます  
`slangc`がインストールされている必要があります
#### Shader:CompileAndLoadFragmentShader(string,number)
- `string`: フラグメントシェーダのソースコード
- `number`: UniformDataの数
フラグメントシェーダをコンパイルして読み込みます  
`slangc`がインストールされている必要があります
### RenderPipeline2D
#### RenderPipeline2D() -> RenderPipeline2D
2Dレンダーパイプラインオブジェクトを作成します
#### RenderPipeline2D:SetVertexShader(Shader)
- `Shader`: 頂点シェーダ  
頂点シェーダを設定します
#### RenderPipeline2D:SetFragmentShader(Shader)
- `Shader`: フラグメントシェーダ  
フラグメントシェーダを設定します
#### RenderPipeline2D:Build()
レンダーパイプラインを構築します
### RenderPipeline3D
#### RenderPipeline3D() -> RenderPipeline3D
3Dレンダーパイプラインオブジェクトを作成します
#### RenderPipeline3D:SetVertexShader(Shader)
- `Shader`: 頂点シェーダ
頂点シェーダを設定します
#### RenderPipeline3D:SetFragmentShader(Shader)
- `Shader`: フラグメントシェーダ
フラグメントシェーダを設定します
#### RenderPipeline3D:Build()
レンダーパイプラインを構築します
### Camera
#### Camera() -> Camera
3D空間のカメラオブジェクトを作成します
#### Camera:LookAt(Vec3, Vec3, Vec3)
- `Vec3`: 位置
- `Vec3`: 注視点
- `Vec3`: 上方向  
カメラの位置、注視点、上方向を設定します
#### Camera:Perspective(number, number, number, number)
- `number`: 視野角
- `number`: アスペクト比
- `number`: 近面
- `number`: 遠面  
カメラの透視投影を設定します
#### Camera:Orthographic(number, number, number, number, number, number)
- `number`: 左
- `number`: 右
- `number`: 下
- `number`: 上
- `number`: 近面
- `number`: 遠面  
カメラの正射影投影を設定します
#### Camera:IsAABBInFrustum(AABB) -> bool
- `AABB`: AABB  
AABBがカメラの視錐台に含まれているかどうかを確認します
#### Camera:GetPosition = Vec3
カメラの位置
#### Camera:GetTarget = Vec3
カメラの注視点
#### Camera.GetUp = Vec3
カメラの上方向
### Music
#### Music() -> Music
#### Music:Load(string)
- `string`: data/music/にある音楽ファイルの名前  
音楽をファイルから読み込みます  
.ogg, .wav, .mp3形式に対応しています
#### Music:Play()
音楽を再生します
#### Music:SetVolume(number)
- `number`: 音量  
音楽の音量を設定します
### Sound
#### Sound() -> Sound
#### Sound:Load(string)
- `string`: data/sound/にある音声ファイルの名前  
音声をファイルから読み込みます  
.wav形式に対応しています
#### Sound:Play()
音声を再生します
#### Sound:SetVolume(number)
- `number`: 音量  
音声の音量を設定します
#### Sound:SetPitch(number)
- `number`: ピッチ  
音声のピッチを設定します
#### Sound:SetListener(Vec3)
- `Vec3`: リスナーの位置  
リスナーの位置を設定します
#### Sound:SetPosition(Vec3)
- `Vec3`: 音源の位置  
音源の位置を設定します
### AABB
#### AABB.min = Vec3
AABBの最小点
#### AABB.max = Vec3
AABBの最大点
### Model
#### Model.AABB = AABB
モデルのAABB
#### Model:Load(string)
- `string`: data/model/にあるモデルファイルの名前  
.gltf, .glb形式に対応しています

## グローバル関数
インスタンスを生成せず、直接呼び出せる関数です
### Random
#### Random:GetRange(number, number) -> number
- `number`: 最小値
- `number`: 最大値  
指定された範囲の浮動小数点数の乱数を取得します

### Window
#### Window:Name() -> string
ウィンドウの名前を取得します
#### Window:Rename(string)
- `string`: ウィンドウの名前  
ウィンドウの名前を変更します
#### Window:Size() -> Vec2
ウィンドウのサイズを取得します
#### Window:Resize(Vec2)
- `Vec2`: ウィンドウのサイズ  
ウィンドウのサイズを変更します
#### Window:Resized() -> bool
ウィンドウが今のフレームでリサイズされたかどうかを取得します
#### Window:Half() -> Vec2
ウィンドウの半分のサイズを取得します
#### Window:SetFullscreen(bool)
- `bool`: ウィンドウをフルスクリーンにするかどうか  
ウィンドウをフルスクリーン、またはウィンドウモードにします

### Graphics
#### Graphics:SetClearColor() -> Color
画面クリアの色を取得します
#### Graphics:SetClearColor(Color)
- `Color`: 画面クリアの色  
画面クリアの色を設定します
#### Graphics:BeginPipeline2D(RenderPipeline2D)
- `RenderPipeline2D`: レンダーパイプライン  
2Dレンダリングを開始します
#### Graphics:EndPipeline2D()
2Dレンダリングを終了します
#### Graphics:BeginPipeline3D(RenderPipeline3D)
- `RenderPipeline3D`: レンダーパイプライン  
3Dレンダリングを開始します
#### Graphics:EndPipeline3D()
3Dレンダリングを終了します
#### Graphics:BeginRenderTexture2D(RenderTexture)
- `RenderTexture`: レンダーテクスチャ
レンダーテクスチャの描画を開始します
#### Graphics:BeginRenderTexture3D(RenderTexture)
- `RenderTexture`: レンダーテクスチャ
#### Graphics:EndRenderTexture(RenderTexture, Texture)
- `RenderTexture`: レンダーテクスチャ
- `Texture`: 描画結果  
レンダーテクスチャの描画を終了します  
`Texture`は`RenderTexture`と同じサイズである必要があります

### Scene
#### Scene:Camera() -> Camera
シーンのメインカメラを取得します
#### Scene:Size() -> Vec2
シーンのサイズを取得します
#### Scene:Resize(Vec2)
- `Vec2`: シーンのサイズ  
シーンのサイズを変更します
#### Scene:Half() -> Vec2
シーンの半分のサイズを取得します

### Collision
#### Collision:AABBvsAABB(AABB, AABB) -> bool
- `AABB`: AABB
- `AABB`: AABB  
AABB同士の衝突判定を行います

### Keyboard
#### Keyboard:IsPressed(code) -> bool
- `code`: キーコード  
キーが現在のフレームで押されたかどうかを確認します
#### Keyboard:IsReleased(code) -> bool
- `code`: キーコード  
キーが現在のフレームで離されたかどうかを確認します
#### Keyboard:IsDown(code) -> bool
- `code`: キーコード  
キーが押されているかどうかを確認します
#### Keyboard.A = code
#### Keyboard.B = code
#### Keyboard.C = code
#### Keyboard.D = code
#### Keyboard.E = code
#### Keyboard.F = code
#### Keyboard.G = code
#### Keyboard.H = code
#### Keyboard.I = code
#### Keyboard.J = code
#### Keyboard.K = code
#### Keyboard.L = code
#### Keyboard.M = code
#### Keyboard.N = code
#### Keyboard.O = code
#### Keyboard.P = code
#### Keyboard.Q = code
#### Keyboard.R = code
#### Keyboard.S = code
#### Keyboard.T = code
#### Keyboard.U = code
#### Keyboard.V = code
#### Keyboard.W = code
#### Keyboard.X = code
#### Keyboard.Y = code
#### Keyboard.Z = code
#### Keyboard.key0 = code
#### Keyboard.key1 = code
#### Keyboard.key2 = code
#### Keyboard.key3 = code
#### Keyboard.key4 = code
#### Keyboard.key5 = code
#### Keyboard.key6 = code
#### Keyboard.key7 = code
#### Keyboard.key8 = code
#### Keyboard.key9 = code
#### Keyboard.F1 = code
#### Keyboard.F2 = code
#### Keyboard.F3 = code
#### Keyboard.F4 = code
#### Keyboard.F5 = code
#### Keyboard.F6 = code
#### Keyboard.F7 = code
#### Keyboard.F8 = code
#### Keyboard.F9 = code
#### Keyboard.F10 = code
#### Keyboard.F11 = code
#### Keyboard.F12 = code
#### Keyboard.UP = code
#### Keyboard.DOWN = code
#### Keyboard.LEFT = code
#### Keyboard.RIGHT = code
#### Keyboard.ESCAPE = code
#### Keyboard.SPACE = code
#### Keyboard.ENTER = code
#### Keyboard.BACKSPACE = code
#### Keyboard.TAB = code
#### Keyboard.LSHIFT = code
#### Keyboard.RSHIFT = code
#### Keyboard.LCTRL = code
#### Keyboard.RCTRL = code
#### Keyboard.ALT = code
### Mouse
#### Mouse.GetPosition() -> Vec2
マウスのウィンドウ内の位置を取得します  
ウィンドウの中心が原点(0, 0)です
#### Mouse.GetPositionOnScene() -> Vec2
マウスのシーン内の位置を取得します  
シーンの中心が原点(0, 0)です
#### Mouse.SetPosition(Vec2)
- `Vec2`: マウスのウィンドウ上の位置  
マウスのウィンドウ上の位置を設定します  
ウィンドウの中心が原点(0, 0)です
#### Mouse.SetPositionOnScene(Vec2)
- `Vec2`: シーン上のマウスの位置  
マウスのシーン上の位置を設定します  
シーンの中心が原点(0, 0)です
#### Mouse.IsPressed(code) -> bool
- `code`: マウスのボタンコード  
ボタンが現在のフレームで押されたかどうかを確認します
#### Mouse.IsPressed(code) -> bool
- `code`: マウスのボタンコード  
ボタンが現在のフレームで離されたかどうかを確認します
#### Mouse.IsDown(code) -> bool
- `code`: マウスのボタンコード  
ボタンが押されているかどうかを確認します
#### Mouse.ScrollWheel() -> Vec2
マウスのスクロールの変化量を取得します
#### Mouse.HideCursor(bool)
- `bool`: マウスカーソルを隠すかどうか  
カーソルを表示、または非表示にします
#### Mouse.SetRelative(bool)
- `bool`: マウスを相対座標として取得するかどうか  
相対座標でマウスの位置を取得するかどうかを設定します。
#### Mouse.IsRelative() -> bool
相対座標でマウスの位置を取得しているかどうかを確認します
#### Mouse.LEFT = code
#### Mouse.RIGHT = code
#### Mouse.MIDDLE = code
#### Mouse.X1 = code
#### Mouse.X2 = code
### Gamepad
#### Gamepad.IsConnected() -> bool
ゲームパッドが接続されているかどうかを確認します
#### Gamepad.IsPressed(code) -> bool
- `code`: ゲームパッドのボタンコード  
ボタンが現在のフレームで押されたかどうかを確認します
#### Gamepad.IsReleased(code) -> bool
- `code`: ゲームパッドのボタンコード  
ボタンが現在のフレームで離されたかどうかを確認します
#### Gamepad.IsDown(code) -> bool
- `code`: ゲームパッドのボタンコード  
ボタンが押されているかどうかを確認します  
#### Gamepad.GetLeftStick() -> Vec2
左スティックの変化量を取得します
#### Gamepad.GetRightStick() -> Vec2
右スティックの変化量を取得します
#### Gamepad.INVALID = code
#### Gamepad.A = code
#### Gamepad.B = code
#### Gamepad.X = code
#### Gamepad.Y = code
#### Gamepad.BACK = code
#### Gamepad.START = code
#### Gamepad.GUIDE = code
#### Gamepad.LEFTSTICK = code
#### Gamepad.RIGHTSTICK = code
#### Gamepad.LEFTSHOULDER = code
#### Gamepad.RIGHTSHOULDER = code
#### Gamepad.DPAD_UP = code
#### Gamepad.DPAD_DOWN = code
#### Gamepad.DPAD_LEFT = code
#### Gamepad.DPAD_RIGHT = code
#### Gamepad.MISC1 = code
#### Gamepad.PADDLE1 = code
#### Gamepad.PADDLE2 = code
#### Gamepad.PADDLE3 = code
#### Gamepad.PADDLE4 = code
#### Gamepad.TOUCHPAD = code

### Periodic
#### Periodic.Sin0_1(time: number) -> number
- `time`: 時間(秒)  
0.0から1.0の間で周期的に変化する値を取得します
#### Periodic.Cos0_1(time: number) -> number
- `time`: 時間(秒)  
0.0から1.0の間で周期的に変化する値を取得します

### Time
#### Time.Seconds() -> number
現在のフレームの経過時間を秒単位で取得します
#### Time.Milli() -> number
現在のフレームの経過時間をミリ秒単位で取得します

### Logger
#### Logger.Info(string)
- `string`: ログメッセージ  
情報レベルのログを出力します
#### Logger.Warn(string)
- `string`: ログメッセージ  
警告レベルのログを出力します
#### Logger.Error(string)
- `string`: ログメッセージ  
エラーレベルのログを出力します
#### Logger.Critical(string)
- `string`: ログメッセージ  
致命的なエラーレベルのログを出力します
#### Logger.Debug(string)
- `string`: ログメッセージ  
デバッグレベルのログを出力します