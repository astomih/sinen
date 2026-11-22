# Bistro: 高度なグラフィックス

Amazon Lumberyard の **Bistro Exterior** を元の FBX から直接読み込み、PBR マテリアル、アルファテスト付きの植生、HDR 反射、リアルタイムのプローブ GI で描画します。

![Sinen で描画した Bistro Exterior](preview.png)

シーン: **Amazon Lumberyard Bistro**、[NVIDIA ORCA](https://developer.nvidia.com/orca/amazon-lumberyard-bistro)、Amazon Lumberyard、2017 年 7 月、[CC BY 4.0](https://creativecommons.org/licenses/by/4.0/)。画像はエンジンで描画したものです。元のライセンスと README はローカルのアセットと一緒にコピーされます。

## 元のアセットをダウンロードする

リポジトリのルートから実行します。

```powershell
./examples/graphics/bistro/prepare.ps1
```

公式の **Bistro v5.2** アーカイブ（約 **853 MiB**）をダウンロードし、SHA-256 を検証して Git の対象外であるビルドディレクトリに展開します。その後、元の外観 FBX、DDS テクスチャ、HDR パノラマ、ライセンスを `assets/Bistro_v5_2/` にコピーします。変換やメッシュ分割は行わず、Blender も不要です。ディスクには数 GiB の空き容量を確保してください。

ダウンロード済みのアーカイブや展開済みディレクトリも再利用できます。

```powershell
./examples/graphics/bistro/prepare.ps1 -Archive C:/Assets/Bistro_v5_2.zip
./examples/graphics/bistro/prepare.ps1 -SourceDirectory C:/Assets/Bistro_v5_2
```

その他のプラットフォームでは、公式アーカイブから展開した `BistroExterior.fbx`、`Textures/`、`san_giuseppe_bridge_4k.hdr`、`LICENSE.txt`、`README.txt` を、このサンプルの `assets/Bistro_v5_2/` にコピーします。

サンプルランチャーを起動し、**Graphics → Bistro (Advanced)** を選択します。ロード画面はモデルのインポートと外部テクスチャの読み込みを待ってから GI を開始します。大きなアセットは Git の対象外です。初回読み込みでは小さなサンプルより多くのメモリを使用します。

## モデルの直接読み込み

```luau
local model = sn.Model.new("assets/Bistro_v5_2/BistroExterior.fbx", {
    maxTextureSize = 1024,
    packedSpecular = true,
    flipNormalY = true,
    staticScene = true,
})
-- draw() 内で、PBRRenderer が各パーツのマテリアルを自動でバインドします。
renderer:drawModel(pass, model, transform)
```

静的モデルのインポーターは、FBX ノード階層にあるメッシュの配置とマテリアルをすべて維持します。ノード変換、FBX の軸・単位メタデータ、法線、UV、頂点カラーを適用し、内部の描画パーツを作ります。中間の GLB、マニフェスト、PNG、EXR、プロキシファイルは生成しません。PBR は読み込んだパーツを視錐台でカリングし、同じファイルを参照するマテリアル間でテクスチャを共有します。

`isReady()`、`getPartCount()`、`getMaterialCount()`、`getTriangleCount()` から読み込み状態とシーンの統計を取得できます。既存のアニメーション・スキニング付きモデルの読み込み経路は維持されます。階層とマテリアルの拡張は静的モデル向けです。Bistro のようにアニメーショントラックを含むファイルでは、`staticScene` が作者のノード姿勢を焼き付けます。スキニング付きメッシュはアニメーション経路を使います。

`maxTextureSize` は適切な DDS ミップを選ぶか、読み込み時にデコード済み画像を縮小します。`0` は元の解像度を保持します。DDS デコーダーは 2D の BC1～BC5 UNORM とマスク付き RGB/RGBA 画像に対応し、ミップチェーンも扱えます。BC6/BC7、配列、キューブマップ、符号付き BC4/BC5 には対応しません。

ORCA の旧形式の `Specular` DDS では、R にオクルージョン、G に粗さ、B に金属度を格納しています。`packedSpecular` でこの規約を明示的に有効化します。PBR は BC5 法線の Z を再構成し、`flipNormalY` は元画像の DirectX 規約を選びます。このシーンのベースカラーのアルファには 0.5 のカットオフを使い、ガラスはアルファテストで近似します。HDR パノラマは IBL 用に直接読み込み、`sky.slang` が背景へ SDR 表示マッピングを適用します。

## 操作

| 入力 | 操作 |
| --- | --- |
| 右マウスボタン長押し / 矢印キー | 視点移動 |
| W / A / S / D | 水平方向に移動 |
| Q / E | 下降 / 上昇 |
| 左 Shift | 高速移動 |
| R | 作者のカメラ位置に戻す |
| G | 局所的な間接光を切り替え |
| P | 太陽を動かす。GI も照明の変化に追従 |
| Z / X | 露出を下げる / 上げる |
| F10 | 40 フレームの履歴を蓄積してから `bistro.png` を保存 |
| Esc | Graphics のサンプル一覧に戻る |

`capture.luau` は作者のカメラ位置で画像を保存して終了する別のエントリーポイントです。キャプチャ中は移動を止め、HUD を表示しません。

## リアルタイム GI

`ready()` 内の `gi:addModel(model, transform, nil, nil, 40000)` は、meshoptimizer を使って **メモリ上**に近似 GI プロキシを生成します。頂点数の予算は読み込んだパーツに配分され、小さなパーツにも最低限の形状を残すため、厳密な全体上限ではなく簡略化の目標です。不透明な葉カードが遮蔽物になるのを避けるため、アルファマスク付きパーツは除外します。テクスチャの平均色と頂点カラーで拡散反射率を近似し、発光マップから放射輝度を取得します。

静的なプロキシと BVH は一度登録します。**12 × 6 × 12** の局所プローブボリュームは、プローブごとに毎フレーム **64 本の GPU レイ**を追跡し、拡散反射は最大 **2 回**、履歴の重みは **0.88** です。幅 64 m のボリュームはカメラに追従して中心を移し、時間履歴をリセットします。トレーサーは太陽と一定の空の光を評価します。PBR は HDR の鏡面 IBL と弱い青色の指向性補助光を保持します。GI は拡散光の近似で、高解像度のレイトレーシングによる直接光の影は生成しません。

元の v5.2 FBX は **2,832,120 三角形**、使用中の **132 マテリアル**、**1,591 内部パーツ**として読み込まれます。RTX 4070 Ti SUPER では、Windows Release / D3D12 ビルドで GI を有効にした 1280 × 720 の描画が約 **60 FPS** でした（ウォームアップ後の 180 フレーム、VSync 有効）。これは特定のマシンでの測定であり、GPU 単体の無制限な処理時間や他のハードウェアでの性能保証ではありません。

`sky.frag.snb` は SPIR-V、SPIR-V 1.3、WGSL、DXIL の各形式を含みます。サンプルの実行にシェーダーコンパイラプラグインは不要です。
