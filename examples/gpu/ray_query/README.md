# 21 Ray Query

This sample builds BLAS/TLAS through the Luau `sn.Raytracing` API, binds the TLAS to a compute shader, and traces inline rays with `RayQuery`.

- `main.luau` creates the acceleration structures, dispatches a compute pipeline, and displays the storage-buffer result as a texture.
- `ray_query.slang` uses `RaytracingAccelerationStructure` at SPIR-V `set 6` / DX `space6`.

The sample requires a desktop backend/device with ray query support.
