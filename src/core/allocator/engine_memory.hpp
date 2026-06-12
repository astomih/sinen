#ifndef SINEN_ENGINE_MEMORY_HPP
#define SINEN_ENGINE_MEMORY_HPP

#include "allocator.hpp"

namespace sinen {

struct MemoryContext {
  Allocator *global;
  Allocator *frame;
  Allocator *scene;
  Allocator *asset;
  Allocator *graphics;
  Allocator *physics;
  Allocator *script;
};

class EngineMemory {
public:
  static bool initialize();
  // Does not release GlobalAllocator because SDL memory hooks may outlive
  // SDL_AppQuit.
  static void shutdown();

  static MemoryContext context();

  static Allocator *global();
  static Allocator *frame();
  static Allocator *scene();
  static Allocator *asset();
  static Allocator *graphics();
  static Allocator *physics();
  static Allocator *script();

  static void beginFrame();
  static void endFrame();

  static void resetScene();
};

} // namespace sinen

#endif // !SINEN_ENGINE_MEMORY_HPP
