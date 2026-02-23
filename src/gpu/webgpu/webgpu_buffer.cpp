#include "webgpu_buffer.hpp"

#ifndef EMSCRIPTEN

namespace sinen::gpu::webgpu {
Buffer::~Buffer() {
  if (buffer) {
    wgpuBufferRelease(buffer);
    buffer = nullptr;
  }
}
} // namespace sinen::gpu::webgpu

#endif // EMSCRIPTEN
