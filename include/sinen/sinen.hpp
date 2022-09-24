#ifndef SINEN_HPP
#define SINEN_HPP
#ifdef ANDROID
#define MOBILE
#endif

#include "actor/actor.hpp"
#include "audio/music.hpp"
#include "audio/sound.hpp"
#include "camera/camera.hpp"
#include "color/color.hpp"
#include "color/hsv.hpp"
#include "color/palette.hpp"
#include "component/component.hpp"
#include "drawable/drawable.hpp"
#include "drawable/drawable_wrapper.hpp"
#include "exception/exception.hpp"
#include "font/font.hpp"
#include "graph/bfs.hpp"
#include "graph/grid.hpp"
#include "input/input.hpp"
#include "instancing/instancing.hpp"
#include "instancing/instancing_wrapper.hpp"
#include "io/dstream.hpp"
#include "io/file.hpp"
#include "logger/logger.hpp"
#include "math/math.hpp"
#include "math/matrix3.hpp"
#include "math/matrix4.hpp"
#include "math/point2.hpp"
#include "math/point3.hpp"
#include "math/quaternion.hpp"
#include "math/random.hpp"
#include "math/transform.hpp"
#include "math/vector2.hpp"
#include "math/vector3.hpp"
#include "model/model.hpp"
#include "network/tcp_client.hpp"
#include "network/tcp_socket.hpp"
#include "physics/collision.hpp"
#include "render/renderer.hpp"
#include "scene/scene.hpp"
#include "script/script.hpp"
#include "shader/shader.hpp"
#include "time/time.hpp"
#include "time/timer.hpp"
#include "utility/handler.hpp"
#include "utility/launcher.hpp"
#include "utility/non_copyable.hpp"
#include "utility/singleton.hpp"
#include "vertex/vertex.hpp"
#include "vertex/vertex_array.hpp"
#include "window/window.hpp"

#endif
#ifdef NO_USE_SINEN_NAMESPACE
using namespace sinen;
#endif
