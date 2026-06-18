#include <core/allocator/engine_memory.hpp>

#include <core/allocator/global_allocator.hpp>

#include <core/def/types.hpp>
#include <memory_resource>

namespace sinen {
namespace {
constexpr Size frameAllocatorSize = 16 * 1024 * 1024;
constexpr Size sceneAllocatorSize = 64 * 1024 * 1024;

struct EngineMemoryState {
  Allocator *global = nullptr;
  std::pmr::monotonic_buffer_resource *frame = nullptr;
  std::pmr::monotonic_buffer_resource *scene = nullptr;
  Allocator *asset = nullptr;
  Allocator *graphics = nullptr;
  Allocator *physics = nullptr;
  Allocator *script = nullptr;
  bool initialized = false;
};

EngineMemoryState gMemory;

Allocator *fallbackGlobal() { return GlobalAllocator::get(); }
} // namespace

bool EngineMemory::initialize() {
  if (gMemory.initialized) {
    return true;
  }

  gMemory.global = GlobalAllocator::get();
  gMemory.frame = new std::pmr::monotonic_buffer_resource(frameAllocatorSize,
                                                          gMemory.global);
  gMemory.scene = new std::pmr::monotonic_buffer_resource(sceneAllocatorSize,
                                                          gMemory.global);
  gMemory.asset = gMemory.global;
  gMemory.graphics = gMemory.global;
  gMemory.physics = gMemory.global;
  gMemory.script = gMemory.global;
  gMemory.initialized = true;
  return true;
}

void EngineMemory::shutdown() {
  if (!gMemory.initialized) {
    return;
  }

  std::pmr::set_default_resource(std::pmr::new_delete_resource());

  delete gMemory.frame;
  gMemory.frame = nullptr;
  delete gMemory.scene;
  gMemory.scene = nullptr;
  gMemory.asset = nullptr;
  gMemory.graphics = nullptr;
  gMemory.physics = nullptr;
  gMemory.script = nullptr;
  gMemory.global = nullptr;
  gMemory.initialized = false;
}

MemoryContext EngineMemory::context() {
  return {global(), frame(), scene(), asset(), graphics(), physics(), script()};
}

Allocator *EngineMemory::global() {
  return gMemory.global ? gMemory.global : fallbackGlobal();
}

Allocator *EngineMemory::frame() {
  return gMemory.frame ? gMemory.frame : global();
}

Allocator *EngineMemory::scene() {
  return gMemory.scene ? gMemory.scene : global();
}

Allocator *EngineMemory::asset() {
  return gMemory.asset ? gMemory.asset : global();
}

Allocator *EngineMemory::graphics() {
  return gMemory.graphics ? gMemory.graphics : global();
}

Allocator *EngineMemory::physics() {
  return gMemory.physics ? gMemory.physics : global();
}

Allocator *EngineMemory::script() {
  return gMemory.script ? gMemory.script : global();
}

void EngineMemory::beginFrame() {}

void EngineMemory::endFrame() {
  if (gMemory.frame) {
    gMemory.frame->release();
  }
}

void EngineMemory::resetScene() {
  if (gMemory.scene) {
    gMemory.scene->release();
  }
}

} // namespace sinen
