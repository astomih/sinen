# 20 Ray Tracing

This sample renders a triangle through the Luau `sn.Raytracing` API.

- `main.luau` creates BLAS/TLAS, builds the ray tracing pipeline, uploads the shader table, dispatches rays, and displays the storage-buffer result as a texture.
- `raytrace.slang` contains ray generation, miss, and closest-hit entry points.

Ray tracing shader resources use SPIR-V set 4 / DX space 4. Acceleration structures start at `vk::binding(0, 4)` / `register(t0, space4)`, and storage buffers start after the acceleration-structure block at `vk::binding(8, 4)` / `register(u0, space4)`.

The sample requires a desktop backend/device with D3D12 DXR or Vulkan KHR ray tracing support.
