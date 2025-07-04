# Lua API

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

クラスはすべてsn.から始まります(例: `sn.Vec2`, `sn.Color`, `sn.Texture`)

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

#### Vec2:Dot(other) -> number

- `other`: Vec2オブジェクト
  内積を取得します

#### Vec2:Lerp(other, t) -> Vec2

- `other`: Vec2オブジェクト
- `t`: 補間係数
  線形補間を取得します

#### Vec2:Reflect(n) -> Vec2

- `n`: 法線ベクトル
  反射ベクトルを取得します

### Vec2i

#### メタテーブル

- `__add`: ベクトルの加算を行います (a + b)
- `__sub`: ベクトルの減算を行います (a - b)

#### Vec2i(x, y) -> Vec2i

- `x`: ベクトルのx成分(整数)
- `y`: ベクトルのy成分(整数)  
  Vec2iオブジェクトを作成します。

#### Vec2i(value) -> Vec2i

- `value`: 整数値
  Vec2iオブジェクトをx, y成分に同じvalueを持つベクトルとして作成します

#### Vec2i.x = integer

Vec2iのx成分。

#### Vec2i.y = integer

Vec2iのy成分。

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

#### Vec3:Forward(rotation) -> Vec3

- `rotation`: 回転角度ベクトル  
  前方ベクトルを取得します

#### Vec3:Dot(other) -> number

- `other`: Vec3オブジェクト
  内積を取得します

#### Vec3:Cross(other) -> Vec3

- `other`: Vec3オブジェクト
  外積を取得します

#### Vec3:Lerp(other, t) -> Vec3

- `other`: Vec3オブジェクト
- `t`: 補間係数
  線形補間を取得します

#### Vec3:Reflect(n) -> Vec3

- `n`: 法線ベクトル
  反射ベクトルを取得します

### Vec3i

#### メタテーブル

- `__add`: ベクトルの加算を行います (a + b)
- `__sub`: ベクトルの減算を行います (a - b)

#### Vec3i(x, y, z) -> Vec3i

- `x`: ベクトルのx成分(整数)
- `y`: ベクトルのy成分(整数)
- `z`: ベクトルのz成分(整数)
  Vec3iオブジェクトを作成します

#### Vec3i(value) -> Vec3i

- `value`: 整数値
  Vec3iオブジェクトをx, y, z成分に同じvalueを持つベクトルとして作成します

#### Vec3i.x = integer

Vec3iのx成分

#### Vec3i.y = integer

Vec3iのy成分

#### Vec3i.z = integer

Vec3iのz成分

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

#### Material:SetTexture(texture, index)

- `texture`: テクスチャ
- `index`: インデックス(1ベース、オプション)
  テクスチャをマテリアルに設定します

#### Material:AppendTexture(texture)

- `texture`: テクスチャ  
  テクスチャをマテリアルに追加します

#### Material:Clear()

マテリアル内のテクスチャをすべてクリアします

#### Material:GetTexture(index) -> Texture

- `index`: インデックス
  指定したインデックスのテクスチャを取得します

### Font

#### Font() -> Font

フォントオブジェクトを作成します

#### Font:Load(size, path)

- `size`: フォントのサイズ(ピクセル)
- `path`: data/font/にあるフォントファイルの名前(オプション)
  .ttfファイルを読み込みます

#### Font:RenderText(texture, text, color) -> Texture

- `texture`: 描画先のテクスチャ
- `text`: 描画する文字列
- `color`: テキストの色
  CPUでテキストを描画します  
  テクスチャは描画結果に合わせてリサイズされます

#### Font:Resize(size)

- `size`: フォントのサイズ(ピクセル)
  フォントのサイズを変更します

### Timer

#### Timer() -> Timer

タイマーオブジェクトを作成します

#### Timer:Start()

タイマーを開始します

#### Timer:Stop()

タイマーを停止します

#### Timer:IsStarted() -> boolean

タイマーが開始されているかどうかを確認します

#### Timer:SetTime(time)

- `time`: 時間(秒)
  タイマーの時間を設定します

#### Timer:Check() -> boolean

タイマーが設定した時間に達したかどうかを確認します

### Grid

#### Grid(w, h) -> Grid

- `w`: 幅
- `h`: 高さ
  グリッドオブジェクトを作成します

#### Grid:At(x, y) -> integer

- `x`: x座標
- `y`: y座標
  指定した座標の値を取得します

#### Grid:Set(x, y, v)

- `x`: x座標
- `y`: y座標
- `v`: 値
  指定した座標に値を設定します

#### Grid:Width() -> integer

グリッドの幅を取得します

#### Grid:Height() -> integer

グリッドの高さを取得します

#### Grid:Size() -> integer

グリッドのサイズ(幅×高さ)を取得します

#### Grid:Clear()

グリッドをクリアします

#### Grid:Resize(w, h)

- `w`: 幅
- `h`: 高さ
  グリッドのサイズを変更します

#### Grid:Fill(value)

- `value`: 値
  グリッドを指定した値で埋めます

### BFSGrid

#### BFSGrid(grid) -> BFSGrid

- `grid`: Gridオブジェクト
  BFS(幅優先探索)グリッドオブジェクトを作成します

#### BFSGrid:Width() -> integer

グリッドの幅を取得します

#### BFSGrid:Height() -> integer

グリッドの高さを取得します

#### BFSGrid:FindPath(start, end_) -> any

- `start`: 開始位置(Vec2)
- `end_`: 終了位置(Vec2)
  パスを検索します

#### BFSGrid:Trace() -> Vec2

次の位置を取得します

#### BFSGrid:Traceable() -> boolean

トレース可能かどうかを確認します

#### BFSGrid:Reset()

検索状態をリセットします

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

#### Draw2D:Add(drawable)

- `drawable`: 描画可能オブジェクト
  同じテクスチャ、モデルで複数のオブジェクトを追加して描画します  
  インスタンス生成時点で既に一つのオブジェクトが存在しています

#### Draw2D:At(x, y)

- `x`: x座標
- `y`: y座標
  オブジェクトの位置を設定します

#### Draw2D:Clear()

追加されたオブジェクトをクリアします  
ただし、インスタンス生成時点で存在しているオブジェクトはクリアされません

### Draw3D

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

#### Draw3D.isDrawDepth = boolean

深度描画を行うかどうか

#### Draw3D:Draw()

3Dオブジェクトを描画します

#### Draw3D:Add(position, rotation, scale)

- `position`: 位置情報(Vec3)
- `rotation`: 回転角度(Vec3)
- `scale`: 拡大率(Vec3)  
  同じテクスチャ、モデルで複数のオブジェクトを追加して描画します  
  インスタンス生成時点で既に一つのオブジェクトが存在しています

#### Draw3D:At(x, y, z)

- `x`: x座標
- `y`: y座標
- `z`: z座標
  オブジェクトの位置を設定します

#### Draw3D:Clear()

追加されたオブジェクトをクリアします  
ただし、インスタンス生成時点で存在しているオブジェクトはクリアされません

### UniformData

#### UniformData() -> UniformData

ユニフォームデータオブジェクトを作成します

#### UniformData:Add(value)

- `value`: データ  
  ユニフォームデータを追加します

#### UniformData:Change(index, value)

- `index`: インデックス
- `value`: データ
  ユニフォームデータを指定したインデックスで変更します

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

### GraphicsPipeline2D

#### GraphicsPipeline2D() -> GraphicsPipeline2D

2Dグラフィックスパイプラインオブジェクトを作成します

#### GraphicsPipeline2D:SetVertexShader(shader)

- `shader`: Shaderオブジェクト  
  頂点シェーダを設定します

#### GraphicsPipeline2D:SetFragmentShader(shader)

- `shader`: Shaderオブジェクト  
  フラグメントシェーダを設定します

#### GraphicsPipeline2D:Build()

グラフィックスパイプラインを構築します

### GraphicsPipeline3D

#### GraphicsPipeline3D() -> GraphicsPipeline3D

3Dグラフィックスパイプラインオブジェクトを作成します

#### GraphicsPipeline3D:SetVertexShader(shader)

- `shader`: Shaderオブジェクト
  頂点シェーダを設定します

#### GraphicsPipeline3D:SetVertexInstancedShader(shader)

- `shader`: Shaderオブジェクト
  インスタンス化頂点シェーダを設定します

#### GraphicsPipeline3D:SetFragmentShader(shader)

- `shader`: Shaderオブジェクト
  フラグメントシェーダを設定します

#### GraphicsPipeline3D:SetAnimation(anim)

- `anim`: アニメーション
  アニメーションを設定します

#### GraphicsPipeline3D:Build()

グラフィックスパイプラインを構築します

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

#### Camera:IsAABBInFrustum(aabb) -> boolean

- `aabb`: AABBオブジェクト  
  AABBがカメラの視錐台に含まれているかどうかを確認します

#### Camera:GetPosition() -> Vec3

カメラの位置を取得します

#### Camera:GetTarget() -> Vec3

カメラの注視点を取得します

#### Camera:GetUp() -> Vec3

カメラの上方向を取得します

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

#### AABB() -> AABB

AABBオブジェクトを作成します

#### AABB.min = Vec3

AABBの最小点

#### AABB.max = Vec3

AABBの最大点

#### AABB:UpdateWorld(position, scale, modelAABB)

- `position`: 位置(Vec3)
- `scale`: スケール(Vec3)
- `modelAABB`: モデルのAABB
  ワールド座標でAABBを更新します

### Model

#### Model() -> Model

モデルオブジェクトを作成します

#### Model:GetAABB() -> AABB

モデルのAABBを取得します

#### Model:Load(path)

- `path`: data/model/にあるモデルファイルの名前  
  .gltf, .glb形式に対応しています

#### Model:LoadSprite()

スプライト用のモデルを読み込みます

#### Model:LoadBox()

ボックス形状のモデルを読み込みます

#### Model:GetBoneUniformData() -> UniformData

ボーンのユニフォームデータを取得します

#### Model:Play(position)

- `position`: 再生位置
  アニメーションを再生します

#### Model:Update(delta)

- `delta`: デルタ時間
  モデルを更新します

## グローバル関数

インスタンスを生成せず、直接呼び出せる関数です

### Random

#### Random.GetRange(a, b) -> number

- `a`: 最小値
- `b`: 最大値  
  指定された範囲の浮動小数点数の乱数を取得します

#### Random.GetIntRange(a, b) -> number

- `a`: 最小値(整数)
- `b`: 最大値(整数)  
  指定された範囲の整数の乱数を取得します

### Window

#### Window.GetName() -> string

ウィンドウの名前を取得します

#### Window.Rename(name)

- `name`: ウィンドウの名前  
  ウィンドウの名前を変更します

#### Window.Size() -> Vec2

ウィンドウのサイズを取得します

#### Window.Resize(size)

- `size`: ウィンドウのサイズ(Vec2)  
  ウィンドウのサイズを変更します

#### Window.Resized() -> boolean

ウィンドウが今のフレームでリサイズされたかどうかを取得します

#### Window.Half() -> Vec2

ウィンドウの半分のサイズを取得します

#### Window.SetFullscreen(full)

- `full`: ウィンドウをフルスクリーンにするかどうか  
  ウィンドウをフルスクリーン、またはウィンドウモードにします

### Graphics

#### Graphics.Draw2D(draw2D)

- `draw2D`: Draw2Dオブジェクト  
  2Dオブジェクトを描画します

#### Graphics.Draw3D(draw3D)

- `draw3D`: Draw3Dオブジェクト
  3Dオブジェクトを描画します

#### Graphics.GetClearColor() -> Color

画面クリアの色を取得します

#### Graphics.SetClearColor(c)

- `c`: 画面クリアの色(Color)  
  画面クリアの色を設定します

#### Graphics.BindPipeline2D(pipe)

- `pipe`: GraphicsPipeline2D  
  2Dグラフィックスパイプラインをバインドします

#### Graphics.BindDefaultPipeline2D()

デフォルトの2Dグラフィックスパイプラインをバインドします

#### Graphics.BindPipeline3D(pipe)

- `pipe`: GraphicsPipeline3D  
  3Dグラフィックスパイプラインをバインドします

#### Graphics.BindDefaultPipeline3D()

デフォルトの3Dグラフィックスパイプラインをバインドします

#### Graphics.SetUniformData(binding, data)

- `binding`: バインディング番号(整数)
- `data`: UniformDataオブジェクト
  ユニフォームデータを設定します

#### Graphics.BeginTarget2D(rt)

- `rt`: RenderTextureオブジェクト
  2Dレンダーターゲットを開始します

#### Graphics.BeginTarget3D(rt)

- `rt`: RenderTextureオブジェクト
  3Dレンダーターゲットを開始します

#### Graphics.EndTarget(rt, texture_ref)

- `rt`: RenderTextureオブジェクト
- `texture_ref`: 描画結果のTextureオブジェクト  
  レンダーターゲットを終了します

### Scene

#### Scene.GetCamera() -> Camera

シーンのメインカメラを取得します

#### Scene.Size() -> Vec2

シーンのサイズを取得します

#### Scene.Resize(size)

- `size`: シーンのサイズ(Vec2)  
  シーンのサイズを変更します

#### Scene.Half() -> Vec2

シーンの半分のサイズを取得します

#### Scene.Ratio() -> number

シーンのアスペクト比を取得します

#### Scene.InvRatio() -> number

シーンの逆アスペクト比を取得します

#### Scene.DeltaTime() -> number

デルタ時間を取得します

#### Scene.Change(name)

- `name`: シーン名
  シーンを変更します

### Collision

#### Collision.AABBvsAABB(a, b) -> boolean

- `a`: AABBオブジェクト
- `b`: AABBオブジェクト  
  AABB同士の衝突判定を行います

### Keyboard

#### Keyboard.IsPressed(scancode) -> boolean

- `scancode`: キーコード  
  キーが現在のフレームで押されたかどうかを確認します

#### Keyboard.IsReleased(scancode) -> boolean

- `scancode`: キーコード  
  キーが現在のフレームで離されたかどうかを確認します

#### Keyboard.IsDown(scancode) -> boolean

- `scancode`: キーコード  
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

#### Keyboard.Key0 = code

#### Keyboard.Key1 = code

#### Keyboard.Key2 = code

#### Keyboard.Key3 = code

#### Keyboard.Key4 = code

#### Keyboard.Key5 = code

#### Keyboard.Key6 = code

#### Keyboard.Key7 = code

#### Keyboard.Key8 = code

#### Keyboard.Key9 = code

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

#### Keyboard.RETURN = code

#### Keyboard.BACKSPACE = code

#### Keyboard.TAB = code

#### Keyboard.LSHIFT = code

#### Keyboard.RSHIFT = code

#### Keyboard.LCTRL = code

#### Keyboard.RCTRL = code

#### Keyboard.LALT = code

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

#### Mouse.IsPressed(btn) -> boolean

- `btn`: マウスのボタンコード  
  ボタンが現在のフレームで押されたかどうかを確認します

#### Mouse.IsReleased(btn) -> boolean

- `btn`: マウスのボタンコード  
  ボタンが現在のフレームで離されたかどうかを確認します

#### Mouse.IsDown(btn) -> boolean

- `btn`: マウスのボタンコード  
  ボタンが押されているかどうかを確認します

#### Mouse.GetScrollWheel() -> number

マウスのスクロールホイールの変化量を取得します

#### Mouse.HideCursor(isHide)

- `isHide`: マウスカーソルを隠すかどうか  
  カーソルを表示、または非表示にします

#### Mouse.SetRelative(isRelative)

- `isRelative`: マウスを相対座標として取得するかどうか  
  相対座標でマウスの位置を取得するかどうかを設定します

#### Mouse.LEFT = code

#### Mouse.RIGHT = code

#### Mouse.MIDDLE = code

#### Mouse.X1 = code

#### Mouse.X2 = code

### Gamepad

#### Gamepad.IsConnected() -> boolean

ゲームパッドが接続されているかどうかを確認します

#### Gamepad.IsPressed(btn) -> boolean

- `btn`: ゲームパッドのボタンコード  
  ボタンが現在のフレームで押されたかどうかを確認します

#### Gamepad.IsReleased(btn) -> boolean

- `btn`: ゲームパッドのボタンコード  
  ボタンが現在のフレームで離されたかどうかを確認します

#### Gamepad.IsDown(btn) -> boolean

- `btn`: ゲームパッドのボタンコード  
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

#### Periodic.Sin0_1(t1, t2) -> number

- `t1`: 時間パラメータ1
- `t2`: 時間パラメータ2  
  0.0から1.0の間で正弦波で周期的に変化する値を取得します

#### Periodic.Cos0_1(t1, t2) -> number

- `t1`: 時間パラメータ1
- `t2`: 時間パラメータ2  
  0.0から1.0の間で余弦波で周期的に変化する値を取得します

### Time

#### Time.Seconds() -> number

アプリケーション開始からの経過時間を秒単位で取得します

#### Time.Milli() -> integer

アプリケーション開始からの経過時間をミリ秒単位で取得します

### Logger

#### Logger.Verbose(msg)

- `msg`: ログメッセージ  
  詳細レベルのログを出力します

#### Logger.Info(msg)

- `msg`: ログメッセージ  
  情報レベルのログを出力します

#### Logger.Warn(msg)

- `msg`: ログメッセージ  
  警告レベルのログを出力します

#### Logger.Error(msg)

- `msg`: ログメッセージ  
  エラーレベルのログを出力します

#### Logger.Critical(msg)

- `msg`: ログメッセージ  
  致命的なエラーレベルのログを出力します