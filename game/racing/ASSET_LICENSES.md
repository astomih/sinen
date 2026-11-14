# RIDGELINE アセットライセンス一覧

## Poly Haven

以下は [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/legalcode) の素材です。[Poly Haven のライセンス方針](https://polyhaven.com/license)

| 素材 | 作者 | 同梱ファイル（`assets/` 内） | 配布元 |
| --- | --- | --- | --- |
| Asphalt Track | Dimitrios Savva | `asphalt_track_diff_1k.ktx2`、`asphalt_track_nor_gl_1k.ktx2`、`asphalt_track_arm_1k.ktx2` | [Poly Haven](https://polyhaven.com/a/asphalt_track) |
| Leafy Grass | Charlotte Baglioni | `leafy_grass_diff_1k.ktx2`、`leafy_grass_nor_gl_1k.ktx2`、`leafy_grass_arm_1k.ktx2` | [Poly Haven](https://polyhaven.com/a/leafy_grass) |
| Kloofendal 48d Partly Cloudy (Pure Sky) | Greg Zaal（撮影）、Jarod Guest（空の編集） | `kloofendal_48d_partly_cloudy_puresky_1k.exr` | [Poly Haven](https://polyhaven.com/a/kloofendal_48d_partly_cloudy_puresky) |

テクスチャは配布元の 1K マップを Khronos KTX-Software の `ktx create --generate-mipmap` で KTX2 に変換しました。HDRI は配布元の 1K EXR です。

## ゲーム用に制作した素材

コースと車両は [art.luau](art.luau) の `MeshBuilder` で生成します。`assets/engine.wav`、`assets/countdown.wav`、`assets/start.wav` は本作向けに合成した音声です。[リポジトリの MIT ライセンス](../../LICENSE.md)

HUD は Sinen 内蔵の [M+ 1p medium](../../src/graphics/font/default/mplus-1p-medium.ttf) を使用します。著作権表示は「Copyright(c) 2017 M+ FONTS PROJECT」で、フォントに M+ Fonts License が埋め込まれています。
