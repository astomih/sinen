# Ocean

Phillips / JONSWAP スペクトルから、風浪と長周期のうねりを生成するネイティブ Luau プラグインです。
CPU の radix-2 二次元逆 FFT で高さと水平変位を計算し、変位後の面から法線と圧縮による白波マスクを生成します。
計算部分は描画エンジンに依存せず、C++ からも `sinen_ocean` ターゲットで利用できます。

## ビルドとサンプル

`SINEN_MODULE_SCRIPT=ON` の構成では `SINEN_PLUGIN_OCEAN` は標準で ON です。
Windows はリポジトリ直下から `build\build_msvc2026-debug.bat` を実行してください。
新規構成は先に `build\generate_msvc2026-debug.bat` を実行します。
生成物は `build/msvc2026-debug/ocean.dll` です。プラグインはプロセスのカレントディレクトリからロードされます。

Examples の **Graphics → FFT Ocean** で海面を表示できます。
この描画サンプルは `shader_compiler` プラグインも必要です。
P: 風浪の Phillips / JONSWAP 切替、S: うねり切替、Space: 一時停止、上下: 視点の高さ、Esc: 戻る。
シミュレーション自体は GPU や shader_compiler を必要としません。

## Luau

```luau
local sn = require("@sinen")
local ok, err = sn.Script.loadPlugin("ocean")
assert(ok, err)
local Ocean = require("@sinen/plugin/ocean")

local sea = Ocean.new({
    resolution = 128, patchSize = 256, depth = 200, seed = 42,
    choppiness = 1.2,
    wind = { spectrum = "phillips", significantWaveHeight = 2.4,
        windSpeed = 12, direction = 0.2 },
    swell = { spectrum = "jonswap", significantWaveHeight = 1.6,
        peakPeriod = 11, peakEnhancement = 5, direction = 1.1, spread = 18 },
})
local vertices, indices = sea:createMeshBuffers()
-- 毎フレーム: 絶対時刻（秒）で更新。バッファは再利用できます。
sea:update(12.5)
sea:writeVertices(vertices)
local s = sea:sample(10, 20)
-- draw() 内の Render3DPass:
-- pass:drawMesh(vertices, indices, transform, oceanMaterial)
```

`update(timeSeconds)` は絶対時刻を受け取り、負の時刻や巻き戻しにも対応します。
同一の設定・seed・時刻で同じ海面を再現します。
`sample(x,z)` は変位前の XZ 座標を周期的にラップして双線形補間し、
`height`, `displacementX/Z`, `normalX/Y/Z`, `foam` を返します。
水平変位後の世界座標から高さを逆算する API ではありません。
`info()` は `resolution`, `patchSize`, `vertexCount`, `indexCount` を返します。

### 設定

長さと波高は m、時間は s、角度は rad。方向 0 は +X、π/2 は +Z へ波が進みます。
`wind` と `swell` は独立の乱数系列を使い、どちらにも両方のスペクトルを指定できます。

| 設定 | 初期値 | 範囲・意味 |
| --- | --- | --- |
| resolution | 128 | 8〜512 の2の累乗 |
| patchSize | 256 | 1〜1,000,000、正方形タイルの一辺 |
| depth | 200 | 0.1〜11,000、水深 |
| gravity | 9.81 | 0.1〜100、重力加速度 |
| choppiness | 1 | 0〜4、水平変位。0 は高さのみ |
| foamThreshold | 0.8 | 0〜1、変位のヤコビアンがこの値より小さいと白波 |
| foamGain | 2 | 0〜100、白波マスクの強度 |
| seed | 1 | uint32 整数 |

| wind / swell の設定 | wind 初期値 | swell 初期値 | 範囲・意味 |
| --- | --- | --- | --- |
| spectrum | jonswap | jonswap | `phillips` または `jonswap` |
| significantWaveHeight | 2 | 1 | 0〜100、有義波高 Hs。0 で無効化 |
| windSpeed | 12 | 12 | 0.1〜100、Phillips の波長スケール |
| direction | 0 | 0.6 | 有限値、進行方向 |
| peakPeriod | 7 | 12 | 0.1〜100、JONSWAP ピーク周期 |
| peakEnhancement | 3.3 | 5 | 1〜20、JONSWAP γ |
| spread | 4 | 16 | 0〜128、JONSWAP の前方 cos^spread 分布 |
| shortWaveDamping | 0.1 | 0.1 | 0〜100、短波を減衰させる長さ |

波高は各波系の離散スペクトルを `sum(P Δk²) = Hs²/16` に正規化します。
そのため解像度を変えても期待波高は変わりません。有限タイル・単一 seed の実現波高にはばらつきがあります。
JONSWAP は周波数密度を `P(kx,kz) = S(ω) D(θ) (dω/dk)/k` で変換します。
分散関係は `ω² = g k tanh(k depth)`。Phillips は `k^-4`、風向への整列、逆風側 0.07、短波減衰を使います。
DC と Nyquist 軸はゼロにして実数面と水平変位の整合性を保ちます。
波長をタイルや格子で十分に表現できるよう設定してください。スペクトル全体が数値的に消失する設定はエラーになります。

### メッシュと描画

`createMeshBuffers()` は Luau 組み込み `buffer` を2個返します（`sn.Buffer` とは別の型です）。
頂点は `(N+1)²` 個、1頂点48 byte、float32 の position.xyz / normal.xyz / uv.xy / color.rgba。
座標範囲は `[0, patchSize]` と水平変位で、Y が高さです。
`color.r` が白波マスク、g/b は0、a は1。白波を表示するには対応シェーダーが必要です。
インデックスは uint32、0始まり、`6N²` 個。端の頂点は隣の周期タイルと一致します。
`writeVertices(buffer)` は正確なサイズを検証し、既存頂点バッファを更新します。

`Render3DPass:drawMesh(vertices, indices, transform, material?)` はこのレイアウトを受け取り、
インデックス範囲と頂点の有限値を検証します。D3D12 はコマンドに属する一時バッファでストリーミングし、
未対応バックエンドは通常の頂点・インデックスアップロードにフォールバックします。
サンプルシェーダーには Fresnel 反射、空の近似、太陽の反射光、波頭の色、白波、距離フォグが含まれます。

計算量は `O(N² log N)`、CPU 実装です。まず128以下を推奨します。
これは周期的な統計波モデルです。海岸での砕波・障害物との流体相互作用・反射屈折のレイトレーシング、
複数スケールの LOD、白波の時間的な蓄積は含みません。水深は分散と周波数変換に用い、浅海砕波モデルではありません。

## 検証

`ctest --test-dir build/msvc2026-debug -R "sinen_ocean|sinen_luau_binding_tests" --output-on-failure`
で、FFT 周波数の直接 DFT 検証、有限水深での時間発展、期待波高、seed、周期境界、法線、白波、
DLL の Luau バッファ生成、D3D12/Vulkan の描画を検証します。

## 参考

- Jerry Tessendorf, [Simulating Ocean Water](https://jtessen.people.clemson.edu/reports/papers_files/waterslides2001.pdf): 統計海面と FFT による合成。
- Orcina, [Wave Spectra used in OrcaFlex](https://www.orcina.com/wp-content/uploads/Wave-Spectra-Used-in-OrcaFlex.pdf): JONSWAP の式。
