# 21 レイクエリ

このサンプルは Luau の `sn.Raytracing` API で BLAS/TLAS を構築し、TLAS をコンピュートシェーダーへ渡して `RayQuery` でインラインのレイ追跡を行います。

- `main.luau` はアクセラレーション構造を生成し、コンピュートパイプラインを実行して、ストレージバッファの結果をテクスチャとして表示します。
- `ray_query.slang` は、SPIR-V `set 0` / DX `space0` にコンピュート出力の `RWByteAddressBuffer` を、SPIR-V `set 6` / DX `space6` に `RaytracingAccelerationStructure` を使います。

レイクエリに対応したデスクトップのバックエンドとデバイスが必要です。
