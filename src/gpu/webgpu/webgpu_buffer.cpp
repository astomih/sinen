#include "webgpu_buffer.hpp"

namespace sinen::gpu::webgpu {
Buffer::~Buffer() {
  if (buffer) {
    wgpuBufferRelease(buffer);
    buffer = nullptr;
  }
}
} // namespace sinen::gpu::webgpu