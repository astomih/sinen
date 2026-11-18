# RenderDoc

Sinen connects to RenderDoc's optional in-application API before GPU backend
creation on Windows and Linux (including Android). No RenderDoc installation is
required to build or run Sinen. Other platforms expose the same API and report
that capture is unavailable.

Launch Sinen from RenderDoc with the executable's working directory set to the
Sinen repository. Select a backend supported by RenderDoc, such as D3D12 or
Vulkan. RenderDoc's capture keys (F12 / Print Screen by default) work as usual.

For explicit loading, set the library path before starting Sinen:

```powershell
$env:SINEN_RENDERDOC_LIBRARY = 'C:\Program Files\RenderDoc\renderdoc.dll'
.\build\msvc2026-debug\sinen.exe
```

On Linux use `librenderdoc.so`, or preload it before launching. Android injection
uses `libVkLayer_GLES_RenderDoc.so`. Capture still requires RenderDoc to hook the
chosen graphics backend; an API connection alone does not guarantee a capture.

## Luau

```luau
if sn.RenderDoc.isAvailable() then
    sn.RenderDoc.setCaptureFilePathTemplate("captures/sinen")
    sn.RenderDoc.triggerCapture() -- next presented frame
    -- sn.RenderDoc.triggerCapture(3) -- three separate consecutive captures
end
```

Request captures once, such as from an input event. Calling `triggerCapture` in
every update continuously requests captures. It returns whether the request was
issued, not whether a capture file was saved. Inspect `getNumCaptures()` and
`getCaptureFilePath(index)` after presentation to find completed files; indexes
start at zero. `isFrameCapturing()` queries RenderDoc's current capture state.
Unavailable operations return `false`, `0`, or an empty path. A frame count must
be positive and an index non-negative.

## C++

Include `<gpu/renderdoc.hpp>` and use `sinen::RenderDoc` with the same methods.
`RenderDoc::initialize()` runs automatically from `gpu::RHI::createBackend()`.
Custom backend integrations must call it before device creation when explicitly
loading the capture library. It can be called again to detect a later injection.
Call these methods from the render thread. The library stays loaded until process
exit so its hooks remain valid during backend switches and GPU teardown.

The API header is vendored independently of the shader compiler plugin. See the
[official API reference](https://github.com/baldurk/renderdoc/blob/v1.x/docs/in_application_api.rst).
