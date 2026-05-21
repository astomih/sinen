# 20 Ray Tracing

This sample renders a triangle through the Luau `sn.Raytracing` API.

- `main.luau` creates BLAS/TLAS, builds the ray tracing pipeline, uploads the shader table, dispatches rays, and displays the storage-buffer result as a texture.
- `raytrace.slang` contains ray generation, miss, and closest-hit entry points.

The sample requires a desktop backend/device with D3D12 DXR or Vulkan KHR ray tracing support.
