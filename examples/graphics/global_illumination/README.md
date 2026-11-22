# Real-time global illumination

Open **Graphics → Global Illumination** in the examples launcher. The scene is
procedural and needs no downloaded assets or shader compiler plugin at runtime.
There is no pre-bake. The light and sphere move continuously, and indirect light
is recomputed on the GPU every frame.

| Direct light only | Direct light + real-time GI |
| --- | --- |
| ![Direct light only](direct.png) | ![Diffuse global illumination](gi.png) |

These PNGs are actual 960 × 720 engine renders using Direct3D 12. Red and blue
walls reflect their colors onto the neutral objects and the floor. The volume
also lights the underside of the sphere, which receives no direct light.

- **Space:** toggle indirect lighting.
- **P:** pause/resume the moving light and sphere.
- **S:** reproduce `direct.png` and `gi.png` in the current scene directory.
- **Escape:** return to the examples launcher.

`main.luau` runs the interactive example. `capture.luau` is an alternative scene
entry point that writes both PNGs and exits; both use the same `scene.luau`.
Capture freezes motion and warms up temporal history for 32 frames so the images
show the same scene state.

```luau
local gi = sn.GlobalIllumination.new()
assert(gi:initializeRealtime(minimum, maximum, 9, 7, 9, 3))
renderer:setGlobalIllumination(gi, 1.0)

-- Every frame, before opening a render pass:
gi:beginFrame() -- Replace geometry while retaining lighting history.
for _, object in objects do
    gi:addModel(object.model, object.transform, sn.Color.new(0.8))
end
gi:setLight(light) -- Copies the current DirectionalLight, PointLight or SpotLight.
assert(gi:update(0.85))
-- Draw normally with renderer:drawModel(...).
```

The CPU builds a triangle BVH when geometry changes. A compute shader traverses
that BVH to trace **64 rays per probe per frame**, with shadow-tested direct
lighting, emission and colored diffuse path bounces. This is software ray tracing
on the GPU; dedicated ray-tracing hardware is not required. Each probe uses one
64-thread workgroup and stores six directional irradiance lobes. The shader
blends new samples with the previous frame, then a GPU buffer-to-texture copy
updates the HDR atlas. The update submits GPU work without a CPU readback or
explicit GPU-idle wait. Ray tests, integration and temporal filtering run on the
GPU; geometry transformation and BVH construction remain on the CPU.

The PBR shader interpolates eight neighboring probes and combines three lobes
according to the surface normal. It applies the material's diffuse BRDF,
metallic factor, occlusion and exposure. With IBL attached, local GI replaces
environment diffuse lighting and retains environment specular reflections.

The example uses 9 × 7 × 9 probes and up to three diffuse hits per ray, totaling
36,288 primary rays per frame, plus secondary and shadow rays. It uses a smaller
sphere mesh as a GI proxy. An animated 300-frame run on the development machine
with the MSVC Debug build and Direct3D 12 averaged approximately 60 FPS with
presentation synchronization; this is a whole-scene measurement, not GPU-only
timing or a performance guarantee.

`initializeRealtime` accepts 2–32 probes per axis and 1–4 diffuse hits; defaults
are 8 × 6 × 8 and 2 hits. Bounds include the end probes and must have positive
finite extent. Invalid settings leave the previous volume intact. Real-time
updates are implemented for Direct3D 12, Vulkan and WebGPU; SDL GPU currently
returns `false`. WebGPU requires its normal runtime dependencies, including DXC
when Dawn selects Direct3D 12. The compute bundle contains SPIR-V, SPIR-V 1.3,
WGSL and DXIL; the compiler plugin is only needed to regenerate that bundle.

When only lighting changes, keep the registered geometry, set the new light or
sky color and call `update`. When geometry moves, call `beginFrame` and register
all participating geometry at its current transforms before `update`. The atlas
updates in place, so the renderer only needs attaching once. Reattach after
reinitializing the volume. History weights are clamped to 0–0.99; `0` uses only
the current frame. The default 0.85 smooths noise while allowing changes to settle
over several frames. `clear` resets geometry, lighting and GPU state.

This is diffuse GI with coarse directional storage. Imported textures, skinning,
transparency and specular transport are not evaluated by the ray tracer. Model
loading must have completed before registration; supply representative linear
diffuse colors and emission for textured assets. Vertex colors multiply the
supplied reflectance. The existing PBR direct-light path is unchanged: the GI
shadow rays affect reflected lighting, not direct-light shadows.

Coarse probe grids can leak light through thin walls or show artifacts where
probes lie inside solid geometry. Keep the volume in usable scene space, use a
denser grid around small geometry and avoid spanning unrelated rooms. Sampling
outside the bounds clamps to the boundary probes. Temporal filtering can lag
behind abrupt changes; lowering the history weight trades smoothness for a
faster response. An attached volume with
intensity zero disables ambient diffuse for a direct-only comparison;
`clearGlobalIllumination` restores the renderer's usual ambient/IBL behavior.

The optional CPU `bake(...)` API remains available for static scenes.
`isBaked()` and `sampleIrradiance(position, normal)` refer to that CPU result;
real-time volumes stay on the GPU and are identified by `isRealtime()`.
