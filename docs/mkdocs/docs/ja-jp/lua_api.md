# Lua言語 API
## 呼び出しについて
Luaスクリプトは`${executable}/script/`に配置します  
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
Sinenでは、Luaのクラスは大文字で始まります  
インスタンスの生成はクラス名()で行います(例: `a = Vector2()`)  
関数はコロン(:)で呼び出し可能です(例: `a:length()`)  
### Vector2
#### メタテーブル
`__add`: ベクトルの加算を行います (a + b)  
`__sub`: ベクトルの減算を行います (a - b)  
`__mul`: ベクトルの乗算を行います (a * b)  
`__div`: ベクトルの除算を行います (a / b)  
#### Vector2(x, y) -> Vector2
- x: ベクトルのx成分。
- y: ベクトルのy成分。
Vector2オブジェクトを作成します。
#### Vector2.x = float
Vector2のx成分。
#### Vector2.y = float
Vector2のy成分。
#### Vector2:length() -> float
ベクトルの長さを取得します
#### Vector2:normalize() -> Vector2
ベクトルを正規化します
### Vector3
#### メタテーブル
`__add`: ベクトルの加算を行います (a + b)  
`__sub`: ベクトルの減算を行います (a - b)  
`__mul`: ベクトルの乗算を行います (a * b)  
`__div`: ベクトルの除算を行います (a / b)  
#### Vector3(x, y, z) -> Vector3
- x: ベクトルのx成分
- y: ベクトルのy成分
- z: ベクトルのz成分
Vector3オブジェクトを作成します
#### Vector3.x = float
Vector3のx成分
#### Vector3.y = float
Vector3のy成分
#### Vector3.z = float
Vector3のz成分
#### Vector3:length() -> float
ベクトルの長さを取得します
#### Vector3:normalize() -> Vector3
ベクトルを正規化します
#### Vector3:copy() -> Vector3
ベクトルをコピーします  
Luaでは基本的に参照渡しのため、コピーが必要な場合に使用します
#### Vector3:forward() -> Vector3
前方ベクトルを取得します
### Point2i
2次元の整数
#### Point2i(x, y) -> Point2i
- x: Point2iのx成分
- y: Point2iのy成分
#### Point2i.x = int
Point2iのx成分
#### Point2i.y = int
Point2iのy成分
### Color
#### Color(r, g, b, a) -> color
- r: 色の赤成分(0.0~1.0)
- g: 色の緑成分(0.0~1.0)
- b: 色の青成分(0.0~1.0)
- a: 色のアルファ成分(0.0~1.0)
色オブジェクトを作成します
#### Color.r = float
色の赤成分(0.0~1.0)
#### Color.g = float
色の緑成分(0.0~1.0)
#### Color.b = float
色の青成分(0.0~1.0)
#### Color.a = float
色のアルファ成分(0.0~1.0)
### Texture
#### Texture()
Create a Texture object.
#### Texture:load(string)
string: The path to the Texture.
Load a Texture from a file.
#### Texture:fill_color(color)
color: The color to fill the Texture with.
Fill the Texture with a color.
#### Texture:blend_color(color)
color: The color to blend the Texture with.
Blend the Texture with a color.
#### Texture:copy() -> Texture
Copy the Texture.
#### Texture:size() -> Vector2
Get the size of the Texture.
### Font
#### Font() -> Font
フォントオブジェクトを作成します
#### Font:load(string)
string: data/font/にあるフォントファイルの名前
.ttfファイルを読み込みます  
#### Font:render_text(Texture, string, color)
- Texture: 描画先のテクスチャ
- string: 描画する文字列
- color: テキストの色
CPUでテキストを描画します  
テクスチャは描画結果に合わせてリサイズされます
### Draw2D
#### Draw2D(Texture) -> Draw2D
- Texture: テクスチャ
Draw2Dオブジェクトを作成します  
modelは未指定の場合、四角形が割り当てられます  
#### Draw2D:draw()
2Dオブジェクトを描画します
#### Draw2D.position = Vector2
位置情報  
Sinenのウィンドウの中心が(0, 0)です
#### Draw2D.scale = Vector2
拡大率
#### Draw2D.rotation = float
時計回りの回転角度
#### Draw2D.texture = Texture
テクスチャ
#### Draw2D.model = Model
モデル
#### Draw2D:add(Vector2, float, Vector2)
- Vector2: 位置情報
- float: 時計回りの回転角度
- Vector2: 拡大率  
同じテクスチャ、モデルで複数のオブジェクトを追加して描画します  
インスタンス生成時点で既に一つのオブジェクトが存在しています
#### Draw2D:clear()
追加されたオブジェクトをクリアします  
ただし、インスタンス生成時点で存在しているオブジェクトはクリアされません
### Draw3D
#### Draw3D(Texture) -> Draw3D
- Texture: テクスチャ
Draw3Dオブジェクトを作成します  
modelは未指定の場合、立方体が割り当てられます
#### Draw3D:draw()
3Dオブジェクトを描画します
#### Draw3D.position = Vector3
位置情報(ワールド座標)
#### Draw3D.scale = Vector3
拡大率
#### Draw3D.rotation = Vector3
回転角度(オイラー角)
#### Draw3D.Texture = Texture
テクスチャ
#### Draw3D.model = Model
モデル
#### Draw3D:add(Vector3,Vector3,Vector3)
- Vector3: 位置情報
- Vector3: 回転角度
- Vector3: 拡大率  
同じテクスチャ、モデルで複数のオブジェクトを追加して描画します  
インスタンス生成時点で既に一つのオブジェクトが存在しています
#### Draw3D:clear()
追加されたオブジェクトをクリアします  
ただし、インスタンス生成時点で存在しているオブジェクトはクリアされません
### Camera
#### Camera() -> Camera
3D空間のカメラオブジェクトを作成します
#### Camera:look_at(Vector3, Vector3, Vector3)
- Vector3: 位置
- Vector3: 注視点
- Vector3: 上方向
カメラの位置、注視点、上方向を設定します
#### Camera:perspective(float, float, float, float)
- float: 視野角
- float: アスペクト比
- float: 近面
- float: 遠面
カメラの透視投影を設定します
#### Camera:orthographic(float, float, float, float, float, float)
- float: 左
- float: 右
- float: 下
- float: 上
- float: 近面
- float: 遠面
カメラの正射影投影を設定します
#### Camera.position = Vector3
カメラの位置
#### Camera.target = Vector3
カメラの注視点
#### Camera.up = Vector3
カメラの上方向
### Music
#### Music() -> Music
#### Music:load(string)
- string: data/music/にある音楽ファイルの名前
音楽をファイルから読み込みます  
.ogg, .wav, .mp3形式に対応しています
#### Music:play()
音楽を再生します
#### Music:set_volume(float)
- float: 音量
音楽の音量を設定します
### Sound
#### Sound() -> Sound
#### Sound:load(string)
- string: data/sound/にある音声ファイルの名前
音声をファイルから読み込みます  
.wav形式に対応しています
#### Sound:play()
音声を再生します
#### Sound:set_volume(float)
- float: 音量
音声の音量を設定します
#### Sound:set_pitch(float)
- float: ピッチ
音声のピッチを設定します
#### Sound:set_listener(Vector3)
- Vector3: リスナーの位置
リスナーの位置を設定します
#### Sound:set_position(Vector3)
- Vector3: 音源の位置
音源の位置を設定します
### AABB
#### AABB.min = Vector3
AABBの最小点
#### AABB.max = Vector3
AABBの最大点
### Model
#### Model:load(string)
- string: data/model/にあるモデルファイルの名前
.gltf, .glb形式に対応しています
#### Model.AABB = AABB
モデルのAABB

## グローバル関数
インスタンスを生成せず、直接呼び出せる関数です
## 命名規則
グローバル関数は小文字から始まる各テーブルに属しています(テーブル例: `random`)  
関数はドット(.)で呼び出し可能です(例: `random.get_int_range(0, 10)`)
### random
#### random:get_int_range(integer, integer) -> integer
- integer: 最小値
- integer: 最大値
指定された範囲の整数の乱数を取得します
#### random:get_float_range(float, float) -> float
- float: 最小値
- float: 最大値
指定された範囲の浮動小数点数の乱数を取得します
### window
#### window.name() -> string
ウィンドウの名前を取得します
#### window.rename(string)
- string: ウィンドウの名前
ウィンドウの名前を変更します
#### window.size() -> Vector2
ウィンドウのサイズを取得します
#### window.resize(Vector2)
- Vector2: ウィンドウのサイズ
ウィンドウのサイズを変更します
#### window.resized() -> bool
ウィンドウが今のフレームでリサイズされたかどうかを取得します
#### window.half() -> Vector2
ウィンドウの半分のサイズを取得します
#### window.set_fullscreen(bool)
- bool: ウィンドウをフルスクリーンにするかどうか
ウィンドウをフルスクリーン、またはウィンドウモードにします
### renderer
#### renderer.clear_color() -> color
画面クリアの色を取得します
#### renderer.set_clear_color(color)
- color: 画面クリアの色
画面クリアの色を設定します
### scene
#### scene.main_Camera() -> Camera
シーンのメインカメラを取得します
#### scene.size() -> Vector2
シーンのサイズを取得します
#### scene.resize(Vector2)
- Vector2: シーンのサイズ
シーンのサイズを変更します
#### scene.half() -> Vector2
シーンの半分のサイズを取得します
### collision
#### collision.aabb_aabb(AABB, AABB) -> bool
- AABB: AABB
- AABB: AABB
AABB同士の衝突判定を行います

### keyboard
#### keyboard.is_pressed(code) -> bool
- code: キーコード
キーが現在のフレームで押されたかどうかを確認します
#### keyboard.is_released(code) -> bool
- code: キーコード
キーが現在のフレームで離されたかどうかを確認します
#### keyboard.is_down(code) -> bool
- code: キーコード
キーが押されているかどうかを確認します
#### keyboard.A = code
#### keyboard.B = code
#### keyboard.C = code
#### keyboard.D = code
#### keyboard.E = code
#### keyboard.F = code
#### keyboard.G = code
#### keyboard.H = code
#### keyboard.I = code
#### keyboard.J = code
#### keyboard.K = code
#### keyboard.L = code
#### keyboard.M = code
#### keyboard.N = code
#### keyboard.O = code
#### keyboard.P = code
#### keyboard.Q = code
#### keyboard.R = code
#### keyboard.S = code
#### keyboard.T = code
#### keyboard.U = code
#### keyboard.V = code
#### keyboard.W = code
#### keyboard.X = code
#### keyboard.Y = code
#### keyboard.Z = code
#### keyboard.key0 = code
#### keyboard.key1 = code
#### keyboard.key2 = code
#### keyboard.key3 = code
#### keyboard.key4 = code
#### keyboard.key5 = code
#### keyboard.key6 = code
#### keyboard.key7 = code
#### keyboard.key8 = code
#### keyboard.key9 = code
#### keyboard.F1 = code
#### keyboard.F2 = code
#### keyboard.F3 = code
#### keyboard.F4 = code
#### keyboard.F5 = code
#### keyboard.F6 = code
#### keyboard.F7 = code
#### keyboard.F8 = code
#### keyboard.F9 = code
#### keyboard.F10 = code
#### keyboard.F11 = code
#### keyboard.F12 = code
#### keyboard.UP = code
#### keyboard.DOWN = code
#### keyboard.LEFT = code
#### keyboard.RIGHT = code
#### keyboard.ESCAPE = code
#### keyboard.SPACE = code
#### keyboard.ENTER = code
#### keyboard.BACKSPACE = code
#### keyboard.TAB = code
#### keyboard.LSHIFT = code
#### keyboard.RSHIFT = code
#### keyboard.LCTRL = code
#### keyboard.RCTRL = code
#### keyboard.ALT = code
### mouse
#### mouse.position() -> Vector2
マウスの位置を取得します
#### mouse.position_on_scene() -> Vector2
マウスのシーン上の位置を取得します
#### mouse.set_position(Vector2)
- Vector2: マウスの位置
マウスの位置を設定します
#### mouse.set_position_on_scene(Vector2)
- Vector2: シーン上のマウスの位置
マウスのシーン上の位置を設定します
#### mouse.is_pressed(code) -> bool
- code: マウスのボタンコード
ボタンが現在のフレームで押されたかどうかを確認します
#### mouse.is_released(code) -> bool
- code: マウスのボタンコード
ボタンが現在のフレームで離されたかどうかを確認します
#### mouse.is_down(code) -> bool
- code: マウスのボタンコード
ボタンが押されているかどうかを確認します
#### mouse.scroll_wheel() -> Vector2
マウスのスクロールの変化量を取得します
#### mouse.hide_cursor(bool)
- bool: マウスカーソルを隠すかどうか
カーソルを表示、または非表示にします
#### mouse.LEFT = code
#### mouse.RIGHT = code
#### mouse.MIDDLE = code
#### mouse.X1 = code
#### mouse.X2 = code
### gamepad
#### gamepad.is_connected() -> bool
ゲームパッドが接続されているかどうかを確認します
#### gamepad.is_pressed(code) -> bool
- code: ゲームパッドのボタンコード
ボタンが現在のフレームで押されたかどうかを確認します
#### gamepad.is_released(code) -> bool
- code: ゲームパッドのボタンコード
ボタンが現在のフレームで離されたかどうかを確認します
#### gamepad.is_down(code) -> bool
- code: ゲームパッドのボタンコード
ボタンが押されているかどうかを確認します  
#### gamepad.left_stick() -> Vector2
左スティックの変化量を取得します
#### gamepad.right_stick() -> Vector2
右スティックの変化量を取得します
#### gamepad.INVALID = code
#### gamepad.A = code
#### gamepad.B = code
#### gamepad.X = code
#### gamepad.Y = code
#### gamepad.BACK = code
#### gamepad.START = code
#### gamepad.GUIDE = code
#### gamepad.LEFTSTICK = code
#### gamepad.RIGHTSTICK = code
#### gamepad.LEFTSHOULDER = code
#### gamepad.RIGHTSHOULDER = code
#### gamepad.DPAD_UP = code
#### gamepad.DPAD_DOWN = code
#### gamepad.DPAD_LEFT = code
#### gamepad.DPAD_RIGHT = code
#### gamepad.MISC1 = code
#### gamepad.PADDLE1 = code
#### gamepad.PADDLE2 = code
#### gamepad.PADDLE3 = code
#### gamepad.PADDLE4 = code
#### gamepad.TOUCHPAD = code
