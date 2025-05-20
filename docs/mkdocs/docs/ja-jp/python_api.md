# Python API
## 呼び出しについて
スクリプトは`sinen.exeが存在するディレクトリ/script/`に配置します  
```python

# セットアップ

def update()
 # 毎フレーム呼び出される関数

def draw()
  # 毎フレーム呼び出される描画関数
```
### Vector2
#### Vector2(x, y) -> Vector2
- `x`: ベクトルのx成分
- `y`: ベクトルのy成分  
Vector2オブジェクトを作成します。
#### Vector2.x = float
Vector2のx成分。
#### Vector2.y = float
Vector2のy成分。
#### Vector2:length() -> float
ベクトルの長さを取得します
#### Vector2:normalize() -> Vector2
ベクトルを正規化します
#### Vector2:copy() -> Vector2
ベクトルをコピーします
#### Vector2:dot(Vector2, Vector2) -> float
- `Vector2`: ベクトル1
- `Vector2`: ベクトル2
内積を取得します
#### Vector2:lerp(Vector2, Vector2, float) -> Vector2
- `Vector2`: ベクトル1
- `Vector2`: ベクトル2
- `float`: 補間係数
線形補間を取得します
#### Vector2:reflect(Vector2, Vector2) -> Vector2
- `Vector2`: ベクトル
- `Vector2`: 法線ベクトル
反射ベクトルを取得します
### Vector3
#### Vector3(x, y, z) -> Vector3
- `x`: ベクトルのx成分
- `y`: ベクトルのy成分
- `z`: ベクトルのz成分  
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
#### Vector3:forward(Vector3) -> Vector3
- `Vector3`: 軸ベクトル  
前方ベクトルを取得します
#### Vector3:dot(Vector3, Vector3) -> float
- `Vector3`: ベクトル1
- `Vector3`: ベクトル2
内積を取得します
#### Vector3:cross(Vector3, Vector3) -> Vector3
- `Vector3`: ベクトル1
- `Vector3`: ベクトル2
外積を取得します
#### Vector3:lerp(Vector3, Vector3, float) -> Vector3
- `Vector3`: ベクトル1
- `Vector3`: ベクトル2
- `float`: 補間係数
線形補間を取得します
#### Vector3:reflect(Vector3, Vector3) -> Vector3
- `Vector3`: ベクトル
- `Vector3`: 法線ベクトル
反射ベクトルを取得します
### Point2i
2次元の整数
#### Point2i(x, y) -> Point2i
- `x`: Point2iのx成分
- `y`: Point2iのy成分
#### Point2i.x = int
Point2iのx成分
#### Point2i.y = int
Point2iのy成分
### Color
#### Color(r, g, b, a) -> color
- `r`: 色の赤成分(0.0~1.0)
- `g`: 色の緑成分(0.0~1.0)
- `b`: 色の青成分(0.0~1.0)
- `a`: 色のアルファ成分(0.0~1.0)  
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
テクスチャオブジェクトを作成します
#### Texture:load(string)
- `string`: data/texture/にあるテクスチャファイルの名前  
テクスチャファイルを読み込みます
#### Texture:fill_color(Color)
- `Color`: テクスチャを塗りつぶす色  
テクスチャを塗りつぶします
#### Texture:blend_color(Color)
- `Color`: テクスチャをブレンドする色  
テクスチャをブレンドします
#### Texture:copy() -> Texture
テクスチャをコピーします  
Luaでは基本的に参照渡しのため、コピーが必要な場合に使用します
#### Texture:size() -> Vector2
テクスチャのサイズを取得します
### RenderTexture
#### RenderTexture()
レンダーテクスチャオブジェクトを作成します
#### RenderTexture:create(int, int)
- `int`: 幅
- `int`: 高さ  
レンダーテクスチャを作成します
### Font
#### Font() -> Font
フォントオブジェクトを作成します
#### Font:load(string)
string: data/font/にあるフォントファイルの名前
.ttfファイルを読み込みます  
#### Font:render_text(Texture, string, color)
- `Texture`: 描画先のテクスチャ
- `string`: 描画する文字列
- `Color`: テキストの色
CPUでテキストを描画します  
テクスチャは描画結果に合わせてリサイズされます
### Draw2D
#### Draw2D(Texture) -> Draw2D
- `Texture`: テクスチャ
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
- `Vector2`: 位置情報
- `float`: 時計回りの回転角度
- `Vector2`: 拡大率  
同じテクスチャ、モデルで複数のオブジェクトを追加して描画します  
インスタンス生成時点で既に一つのオブジェクトが存在しています
#### Draw2D:clear()
追加されたオブジェクトをクリアします  
ただし、インスタンス生成時点で存在しているオブジェクトはクリアされません
### Draw3D
#### Draw3D(Texture) -> Draw3D
- `Texture`: テクスチャ
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
- `Vector3`: 位置情報
- `Vector3`: 回転角度
- `Vector3`: 拡大率  
同じテクスチャ、モデルで複数のオブジェクトを追加して描画します  
インスタンス生成時点で既に一つのオブジェクトが存在しています
#### Draw3D:clear()
追加されたオブジェクトをクリアします  
ただし、インスタンス生成時点で存在しているオブジェクトはクリアされません
### UniformData
#### UniformData() -> UniformData
ユニフォームデータオブジェクトを作成します
#### UniformData:add(float)
- `float`: データ  
ユニフォームデータを追加します
#### UniformData:at(float, int)
- `float`: データ
- `int`: インデックス  
ユニフォームデータを指定したインデックスに追加します
### Shader
#### Shader() -> Shader
シェーダオブジェクトを作成します
#### Shader:load_vertex_shader(string,int)
- `string`: data/shader/にある頂点シェーダファイルの名前
- `int`: UniformDataの数  
頂点シェーダを読み込みます
#### Shader:load_fragment_shader(string,int)
- `string`: data/shader/にあるフラグメントシェーダファイルの名前
- `int`: UniformDataの数  
フラグメントシェーダを読み込みます
#### Shader:compile_and_load_vertex_shader(string,int)
- `string`: 頂点シェーダのソースコード
- `int`: UniformDataの数
頂点シェーダをコンパイルして読み込みます  
`slangc`がインストールされている必要があります
#### Shader:compile_and_load_fragment_shader(string,int)
- `string`: フラグメントシェーダのソースコード
- `int`: UniformDataの数
フラグメントシェーダをコンパイルして読み込みます  
`slangc`がインストールされている必要があります
### RenderPipeline2D
#### RenderPipeline2D() -> RenderPipeline2D
2Dレンダーパイプラインオブジェクトを作成します
#### RenderPipeline2D:set_vertex_shader(Shader)
- `Shader`: 頂点シェーダ  
頂点シェーダを設定します
#### RenderPipeline2D:set_fragment_shader(Shader)
- `Shader`: フラグメントシェーダ  
フラグメントシェーダを設定します
#### RenderPipeline2D:build()
レンダーパイプラインを構築します
### RenderPipeline3D
#### RenderPipeline3D() -> RenderPipeline3D
3Dレンダーパイプラインオブジェクトを作成します
#### RenderPipeline3D:set_vertex_shader(Shader)
- `Shader`: 頂点シェーダ
頂点シェーダを設定します
#### RenderPipeline3D:set_fragment_shader(Shader)
- `Shader`: フラグメントシェーダ
フラグメントシェーダを設定します
#### RenderPipeline3D:build()
レンダーパイプラインを構築します
### Camera
#### Camera() -> Camera
3D空間のカメラオブジェクトを作成します
#### Camera:look_at(Vector3, Vector3, Vector3)
- `Vector3`: 位置
- `Vector3`: 注視点
- `Vector3`: 上方向  
カメラの位置、注視点、上方向を設定します
#### Camera:perspective(float, float, float, float)
- `float`: 視野角
- `float`: アスペクト比
- `float`: 近面
- `float`: 遠面  
カメラの透視投影を設定します
#### Camera:orthographic(float, float, float, float, float, float)
- `float`: 左
- `float`: 右
- `float`: 下
- `float`: 上
- `float`: 近面
- `float`: 遠面  
カメラの正射影投影を設定します
#### Camera:is_aabb_in_frustum(AABB) -> bool
- `AABB`: AABB  
AABBがカメラの視錐台に含まれているかどうかを確認します
#### Camera.position = Vector3
カメラの位置
#### Camera.target = Vector3
カメラの注視点
#### Camera.up = Vector3
カメラの上方向
### Music
#### Music() -> Music
#### Music:load(string)
- `string`: data/music/にある音楽ファイルの名前  
音楽をファイルから読み込みます  
.ogg, .wav, .mp3形式に対応しています
#### Music:play()
音楽を再生します
#### Music:set_volume(float)
- `float`: 音量  
音楽の音量を設定します
### Sound
#### Sound() -> Sound
#### Sound:load(string)
- `string`: data/sound/にある音声ファイルの名前  
音声をファイルから読み込みます  
.wav形式に対応しています
#### Sound:play()
音声を再生します
#### Sound:set_volume(float)
- `float`: 音量  
音声の音量を設定します
#### Sound:set_pitch(float)
- `float`: ピッチ  
音声のピッチを設定します
#### Sound:set_listener(Vector3)
- `Vector3`: リスナーの位置  
リスナーの位置を設定します
#### Sound:set_position(Vector3)
- `Vector3`: 音源の位置  
音源の位置を設定します
### AABB
#### AABB.min = Vector3
AABBの最小点
#### AABB.max = Vector3
AABBの最大点
### Model
#### Model:load(string)
- `string`: data/model/にあるモデルファイルの名前  
.gltf, .glb形式に対応しています
#### Model.AABB = AABB
モデルのAABB

## static class 
static classはインスタンスを生成せずに呼び出すことができます
### Random
#### Random:get_int_range(integer, integer) -> integer
- `integer`: 最小値
- `integer`: 最大値  
指定された範囲の整数の乱数を取得します
#### Random:get_float_range(float, float) -> float
- `float`: 最小値
- `float`: 最大値  
指定された範囲の浮動小数点数の乱数を取得します
### Window
#### Window.name() -> string
ウィンドウの名前を取得します
#### Window.rename(string)
- `string`: ウィンドウの名前  
ウィンドウの名前を変更します
#### Window.size() -> Vector2
ウィンドウのサイズを取得します
#### Window.resize(Vector2)
- `Vector2`: ウィンドウのサイズ  
ウィンドウのサイズを変更します
#### Window.resized() -> bool
ウィンドウが今のフレームでリサイズされたかどうかを取得します
#### Window.half() -> Vector2
ウィンドウの半分のサイズを取得します
#### Window.set_fullscreen(bool)
- `bool`: ウィンドウをフルスクリーンにするかどうか  
ウィンドウをフルスクリーン、またはウィンドウモードにします
### Renderer
#### Renderer.clear_color() -> color
画面クリアの色を取得します
#### Renderer.set_clear_color(color)
- `Color`: 画面クリアの色  
画面クリアの色を設定します
#### Renderer.begin_pipeline2d(RenderPipeline2D)
- `RenderPipeline2D`: レンダーパイプライン  
2Dレンダリングを開始します
#### Renderer.end_pipeline2d()
2Dレンダリングを終了します
#### Renderer.begin_pipeline3d(RenderPipeline3D)
- `RenderPipeline3D`: レンダーパイプライン  
3Dレンダリングを開始します
#### Renderer.end_pipeline3d()
3Dレンダリングを終了します
#### Renderer.begin_render_texture2d(RenderTexture)
- `RenderTexture`: レンダーテクスチャ
レンダーテクスチャの描画を開始します
#### renderer.begin_render_texture3d(RenderTexture)
- `RenderTexture`: レンダーテクスチャ
#### renderer.end_render_texture(RenderTexture, Texture)
- `RenderTexture`: レンダーテクスチャ
- `Texture`: 描画結果  
レンダーテクスチャの描画を終了します  
`Texture`は`RenderTexture`と同じサイズである必要があります
### Scene
#### Scene.camera() -> Camera
シーンのメインカメラを取得します
#### Scene.size() -> Vector2
シーンのサイズを取得します
#### Scene.resize(Vector2)
- `Vector2`: シーンのサイズ  
シーンのサイズを変更します
#### Scene.half() -> Vector2
シーンの半分のサイズを取得します
### Collision
#### Collision.aabb_aabb(AABB, AABB) -> bool
- `AABB`: AABB
- `AABB`: AABB  
AABB同士の衝突判定を行います

### Keyboard
#### Keyboard.is_pressed(code) -> bool
- `code`: キーコード  
キーが現在のフレームで押されたかどうかを確認します
#### Keyboard.is_released(code) -> bool
- `code`: キーコード  
キーが現在のフレームで離されたかどうかを確認します
#### Keyboard.is_down(code) -> bool
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
#### Mouse.position() -> Vector2
マウスのウィンドウ内の位置を取得します  
ウィンドウの中心が原点(0, 0)です
#### Mouse.position_on_scene() -> Vector2
マウスのシーン内の位置を取得します  
シーンの中心が原点(0, 0)です
#### Mouse.set_position(Vector2)
- `Vector2`: マウスのウィンドウ上の位置  
マウスのウィンドウ上の位置を設定します  
ウィンドウの中心が原点(0, 0)です
#### Mouse.set_position_on_scene(Vector2)
- `Vector2`: シーン上のマウスの位置  
マウスのシーン上の位置を設定します  
シーンの中心が原点(0, 0)です
#### Mouse.is_pressed(code) -> bool
- `code`: マウスのボタンコード  
ボタンが現在のフレームで押されたかどうかを確認します
#### Mouse.is_released(code) -> bool
- `code`: マウスのボタンコード  
ボタンが現在のフレームで離されたかどうかを確認します
#### Mouse.is_down(code) -> bool
- `code`: マウスのボタンコード  
ボタンが押されているかどうかを確認します
#### Mouse.scroll_wheel() -> Vector2
マウスのスクロールの変化量を取得します
#### Mouse.hide_cursor(bool)
- `bool`: マウスカーソルを隠すかどうか  
カーソルを表示、または非表示にします
#### Mouse.set_relative(bool)
- `bool`: マウスを相対座標として取得するかどうか  
相対座標でマウスの位置を取得するかどうかを設定します。
#### Mouse.is_relative() -> bool
相対座標でマウスの位置を取得しているかどうかを確認します
#### Mouse.LEFT = code
#### Mouse.RIGHT = code
#### Mouse.MIDDLE = code
#### Mouse.X1 = code
#### Mouse.X2 = code
### Gamepad
#### Gamepad.is_connected() -> bool
ゲームパッドが接続されているかどうかを確認します
#### Gamepad.is_pressed(code) -> bool
- `code`: ゲームパッドのボタンコード  
ボタンが現在のフレームで押されたかどうかを確認します
#### Gamepad.is_released(code) -> bool
- `code`: ゲームパッドのボタンコード  
ボタンが現在のフレームで離されたかどうかを確認します
#### Gamepad.is_down(code) -> bool
- `code`: ゲームパッドのボタンコード  
ボタンが押されているかどうかを確認します  
#### Gamepad.left_stick() -> Vector2
左スティックの変化量を取得します
#### Gamepad.right_stick() -> Vector2
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
