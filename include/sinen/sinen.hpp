#ifndef SINEN_HPP
#define SINEN_HPP
#include "actor/actor.hpp"
#include "allocator/pool_allocator.hpp"
#include "allocator/stack_allocator.hpp"
#include "audio/music.hpp"
#include "audio/sound.hpp"
#include "camera/camera.hpp"
#include "color/color.hpp"
#include "color/hsv.hpp"
#include "color/palette.hpp"
#include "component/components.hpp"
#include "drawable/drawable.hpp"
#include "drawable/drawable_wrapper.hpp"
#include "font/font.hpp"
#include "graph/bfs_grid.hpp"
#include "graph/grid.hpp"
#include "input/gamepad.hpp"
#include "input/keyboard.hpp"
#include "input/mouse.hpp"
#include "io/data_stream.hpp"
#include "io/file.hpp"
#include "io/json.hpp"
#include "logger/logger.hpp"
#include "math/math.hpp"
#include "math/matrix3.hpp"
#include "math/matrix4.hpp"
#include "math/point2.hpp"
#include "math/point3.hpp"
#include "math/quaternion.hpp"
#include "math/random.hpp"
#include "math/rect.hpp"
#include "math/transform.hpp"
#include "math/vector2.hpp"
#include "math/vector3.hpp"
#include "model/model.hpp"
#include "physics/collision.hpp"
#include "physics/physics.hpp"
#include "physics/primitive2.hpp"
#include "render/renderer.hpp"
#include "scene/scene.hpp"
#include "script/script.hpp"
#include "shader/shader.hpp"
#include "time/time.hpp"
#include "time/timer.hpp"
#include "vertex/vertex.hpp"
#include "vertex/vertex_array.hpp"
#include "window/window.hpp"

namespace sinen {
/**
 * @brief Initialize sinen engine
 *
 * @return true Success to initialize
 * @return false Failed to initialize
 */
bool Initialize(int argc, char *argv[]);
/**
 * @brief Run sinen engine
 *
 */
void Run();
/**
 * @brief Shutdown sinen engine
 *
 * @return true Success to shutdown
 * @return false Failed to shutdown
 */
bool Shutdown();
} // namespace sinen

#endif

#ifdef NO_USE_SINEN_NAMESPACE
using namespace sinen;
#endif
