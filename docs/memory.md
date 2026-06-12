# Memory Allocation Policy

Sinen uses `std::pmr::memory_resource` as the allocator interface. New engine
code should receive an `Allocator*` explicitly when the allocation lifetime is
known.

## Allocator Roles

- `EngineMemory::global()` is the fallback and bootstrap allocator for
  process-lifetime engine allocations. In SDL builds, it intentionally stays
  alive through process exit because SDL memory hooks can run after
  `SDL_AppQuit()`.
- `EngineMemory::frame()` is for temporary allocations that are discarded at the
  end of each frame.
- `EngineMemory::scene()` is for allocations that can be discarded when a scene
  is reloaded.
- `EngineMemory::asset()` is for loaded resource data such as decoded buffers,
  shader bundles, textures, audio source data, and archive/file contents.
- `EngineMemory::graphics()`, `EngineMemory::physics()`, and
  `EngineMemory::script()` are subsystem allocator entry points. They currently
  fall back to global where the subsystem has not been fully migrated.

## Global Allocator Use

Do not use `GlobalAllocator` directly in new code unless:

1. the allocation is truly process-lifetime,
2. the call site cannot receive an allocator,
3. it is a third-party fallback path, or
4. it is bootstrap code before `EngineMemory` is initialized.

Allocator-less `makePtr` and `makeUnique` overloads remain for compatibility,
but new code should call the overloads that accept `Allocator*`.

## Examples

Persistent graphics object:

```cpp
auto texture = makePtr<Texture>(EngineMemory::graphics());
```

Frame-local scratch data:

```cpp
Array<UInt8> bytes(EngineMemory::frame());
bytes.resize(requiredSize);
```

Asset buffer:

```cpp
Buffer data = makeBuffer(size, BufferType::Binary, EngineMemory::asset());
```

## Safety Rules

Frame allocator memory is released by `EngineMemory::endFrame()`. Do not store
pointers, references, or containers using `EngineMemory::frame()` beyond the
current frame.

Scene allocator memory is released by `EngineMemory::resetScene()`. Shut down
or detach scene-owned objects before resetting scene memory.

When lifetime is uncertain, keep the allocation on `EngineMemory::global()` and
add a TODO describing the ownership question. A wrong short-lived allocator is
worse than leaving the allocation on the fallback path temporarily.
