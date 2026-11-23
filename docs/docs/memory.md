# メモリ割り当て方針

Sinen はアロケータのインターフェースに `std::pmr::memory_resource` を使用します。割り当ての寿命が分かっている新しいエンジンコードには、`Allocator*` を明示的に渡してください。

## アロケータの役割

- `EngineMemory::global()` はプロセス存続中のエンジン割り当てに使うフォールバック兼起動時のアロケータです。SDL ビルドでは、`SDL_AppQuit()` 後に SDL のメモリフックが呼ばれる可能性があるため、プロセス終了まで保持します。
- `EngineMemory::frame()` は各フレームの終了時に破棄する一時的な割り当てに使います。
- `EngineMemory::scene()` はシーンの再読み込み時に破棄できる割り当てに使います。
- `EngineMemory::asset()` はデコード済みバッファ、シェーダーバンドル、テクスチャ、音声の元データ、アーカイブやファイルの内容など、読み込んだリソースに使います。
- `EngineMemory::graphics()`、`EngineMemory::physics()`、`EngineMemory::script()` は各サブシステムの入口です。移行が完了していない箇所では現在 `global()` にフォールバックします。

## グローバルアロケータ

グローバル TLSF の初期領域は、デスクトップビルドで既定 256 MiB、Emscripten ビルドで既定 128 MiB です。デスクトップでより大きな領域が必要な場合は、起動前に `SINEN_GLOBAL_ALLOCATOR_SIZE_MB` を設定してください。

非同期読み込みには小規模なグローバルスレッドプールを使います。ワーカースレッド数は既定で最大 4、`SINEN_THREAD_POOL_SIZE` で変更できます。

新しいコードで `GlobalAllocator` を直接使用できるのは、次の場合に限ります。

1. 割り当てがプロセスの存続期間全体にわたる。
2. 呼び出し元にアロケータを渡せない。
3. サードパーティーコードのフォールバック処理である。
4. `EngineMemory` 初期化前の起動処理である。

アロケータを指定しない `makePtr` と `makeUnique` のオーバーロードは互換性のために残しています。新しいコードでは `Allocator*` を受け取るオーバーロードを使ってください。

## 使用例

永続的なグラフィックスオブジェクト:

```cpp
auto texture = makePtr<Texture>(EngineMemory::graphics());
```

フレーム内だけで使う作業領域:

```cpp
Array<UInt8> bytes(EngineMemory::frame());
bytes.resize(requiredSize);
```

アセットのバッファ:

```cpp
Buffer data = makeBuffer(size, BufferType::Binary, EngineMemory::asset());
```

## 安全上の注意

`EngineMemory::frame()` のメモリは `EngineMemory::endFrame()` で解放されます。これを使うポインタ、参照、コンテナを次のフレームまで保持しないでください。

`EngineMemory::scene()` のメモリは `EngineMemory::resetScene()` で解放されます。リセット前にシーン所有のオブジェクトを終了または切り離してください。

寿命が不明な割り当ては `EngineMemory::global()` に置き、所有関係の確認事項を TODO に記してください。短命なアロケータを誤って使うより、当面はフォールバックを使う方が安全です。
