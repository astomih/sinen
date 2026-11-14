# FOUNDRY

工業施設の格納庫で、パルス装置を使って警備ドローンの 3 ウェーブを迎撃するシングルプレイ FPS です。ドローンのセンサーが橙色に変わったら攻撃の予兆です。遮蔽物で弾を防げます。撃破時に現れるシアンのセルは耐久値と予備チャージを回復します。

## 起動

リポジトリのルートから Windows の Debug ビルドを起動します。ビルド済みの場合は `Push-Location` の行から実行できます。初回構成には `build\generate_msvc2026-debug.bat` を使用します。

```powershell
cmd /c build\build_msvc2026-debug.bat
Push-Location game\fps
..\..\build\msvc2026-debug\sinen.exe
Pop-Location
```

配布用の `.sna` を作る場合は、このディレクトリの `main.luau`、関連する Luau モジュール、`assets/` を ZIP 内の `sna/` に配置します。配布時は [MIT ライセンス本文](../../LICENSE.md) を添付してください。実行時のネット接続は不要です。

## 操作

| 入力 | 操作 |
| --- | --- |
| Enter / 左クリック | 開始、再開、リトライ |
| マウス | 視点 |
| WASD / 左 Shift / Space | 移動、ダッシュ、ジャンプ |
| 左クリック（長押し可） / 右クリック（長押し） | パルス発射 / 照準 |
| R | リチャージ |
| Esc | ポーズ |
| Q | メニューから終了 |

## アセット

Poly Haven の素材は [CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/) です。1K のベースカラー、OpenGL 法線、AO・粗さ・金属度を詰めた ARM マップを、ミップマップ付きの KTX2 に変換しました。ベースカラーを sRGB として扱い、法線と ARM は線形データとして扱います。HDRI から起動時に拡散照度と粗さ別の反射キューブマップを生成します。

格納庫、ドローン、パルス装置は `MeshBuilder` で生成します。効果音は本作向けに合成した WAV を同梱しています。ファイル名、作者、配布元は [アセットライセンス一覧](ASSET_LICENSES.md) を参照してください。
