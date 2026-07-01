# 22 Path Tracer

This sample renders a small Cornell-box style scene through the Luau `sn.Raytracing` API.
The default resolution and sample count are intentionally conservative so the
single dispatch stays below Windows TDR limits on typical developer GPUs.

- `main.luau` builds a procedural triangle scene, creates BLAS/TLAS, dispatches hardware ray tracing, and displays the rendered storage-buffer result.
- `path_tracer.slang` traces primary, reflection, and shadow rays, reads triangle/material records from ray tracing storage buffers, and writes the final RGBA image.

Ray tracing shader resources use SPIR-V set 4 / DX space 4. Acceleration structures start at `vk::binding(0, 4)` / `register(t0, space4)`, and storage buffers start after the acceleration-structure block at `vk::binding(8, 4)` / `register(u0, space4)`.

The sample requires a desktop backend/device with D3D12 DXR or Vulkan KHR ray tracing support.
