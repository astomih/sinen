# RenderDoc

Sinen は Windows と Linux（Android を含む）で、GPU バックエンドの生成前に RenderDoc の任意のアプリ内 API へ接続します。Sinen のビルドや実行に RenderDoc のインストールは不要です。その他のプラットフォームでも同じ API を公開しますが、キャプチャは利用不可と報告されます。

RenderDoc から Sinen を起動し、実行ファイルの作業ディレクトリを Sinen のリポジトリに設定してください。D3D12 や Vulkan など、RenderDoc が対応するバックエンドを選びます。キャプチャキー（既定では F12 または Print Screen）は通常どおり使えます。

ライブラリを明示的に読み込むには、起動前にパスを設定します。

```powershell
$env:SINEN_RENDERDOC_LIBRARY = 'C:\Program Files\RenderDoc\renderdoc.dll'
.\build\msvc2026-debug\sinen.exe
```

Linux では `librenderdoc.so` を指定するか、起動前にプリロードします。Android のインジェクションには `libVkLayer_GLES_RenderDoc.so` を使います。キャプチャには RenderDoc が選択したグラフィックスバックエンドへフックする必要があります。API へ接続できただけではキャプチャできるとは限りません。

## Luau

```luau
if sn.RenderDoc.isAvailable() then
    sn.RenderDoc.setCaptureFilePathTemplate("captures/sinen")
    sn.RenderDoc.triggerCapture() -- 次に表示するフレーム
    -- sn.RenderDoc.triggerCapture(3) -- 連続する3フレームを別々にキャプチャ
end
```

キャプチャ要求は入力イベントなどから一度だけ行ってください。毎回の `update` で `triggerCapture` を呼ぶと、継続してキャプチャを要求します。戻り値は要求を出せたかどうかで、ファイルが保存されたかどうかではありません。表示後に `getNumCaptures()` と `getCaptureFilePath(index)` で完成したファイルを確認できます。インデックスは 0 から始まります。`isFrameCapturing()` は RenderDoc の現在のキャプチャ状態を問い合わせます。利用できない操作は `false`、`0`、または空のパスを返します。フレーム数は正、インデックスは 0 以上で指定してください。

## C++

`<gpu/renderdoc.hpp>` をインクルードし、Luau と同じメソッドを持つ `sinen::RenderDoc` を使います。`RenderDoc::initialize()` は `gpu::RHI::createBackend()` から自動的に実行されます。独自のバックエンド統合でキャプチャライブラリを明示的に読み込む場合は、デバイス生成前に呼んでください。後からのインジェクションを検出するために再度呼ぶこともできます。

これらのメソッドはレンダースレッドから呼んでください。バックエンドの切り替えや GPU の終了処理中もフックが有効になるよう、ライブラリはプロセス終了まで読み込んだままにします。

API ヘッダーはシェーダーコンパイラプラグインとは独立して同梱しています。[公式 API リファレンス](https://github.com/baldurk/renderdoc/blob/v1.x/docs/in_application_api.rst)を参照してください。
