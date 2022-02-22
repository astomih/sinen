#include "Actor/Actor.hpp"
#include "Component/Component.hpp"
#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"
#include "Utility/handle_t.hpp"
#include "sol/overload.hpp"
#include "sol/types.hpp"
#include <Nen.hpp>
#include <functional>
#include <sol/sol.hpp>
namespace nen {
bool script_system::initialize() {
  lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);
  auto normalize = [](vector3 &v) { v.Normalize(); };
  lua.new_usertype<vector3>(
      "vector3",
      sol::constructors<sol::types<>, sol::types<float, float, float>>(),
      "normalize", normalize, "x", &vector3::x, "y", &vector3::y, "z",
      &vector3::z);
  lua.new_usertype<vector2>(
      "vector2", sol::constructors<sol::types<>, sol::types<float, float>>(),
      "x", &vector2::x, "y", &vector2::y);
  lua.new_usertype<quaternion>(
      "quaternion",
      sol::constructors<sol::types<>, sol::types<vector3, float>>(),
      "concatenate", &quaternion::Concatenate, "x", &quaternion::x, "y",
      &quaternion::y, "z", &quaternion::z);
  lua.new_usertype<manager>(
      "manager_t", sol::constructors<sol::types<>, sol::types<>>(),
      "get_current_scene", &manager::get_current_scene, "get_window",
      &manager::get_window, "get_renderer", &manager::get_renderer,
      "get_input_system", &manager::get_input_system, "get_sound_system",
      &manager::get_sound_system, "get_script_system",
      &manager::get_script_system);
  lua["manager"] = &m_manager;
  // lua.new_usertype<handle_t>("handle_t");
  auto get_base_actor = &base_scene::get_actor<base_actor>;
  auto add_base_actor = [](base_scene &scene) -> handle_t {
    handle_t handle;
    scene.add_actor<base_actor>(handle);
    return handle;
  };
  lua.new_usertype<base_scene>(
      "base_scene", sol::constructors<sol::types<manager &>>(),
      "add_base_actor", add_base_actor, "get_base_actor", get_base_actor);
  auto base_actor_get_position = &base_actor::GetPosition;
  auto base_actor_set_position = &base_actor::SetPosition;
  auto base_actor_get_rotation = &base_actor::GetRotation;
  auto base_actor_set_rotation = &base_actor::SetRotation;
  auto base_actor_move = [](base_actor &actor, const vector3 &move) {
    actor.Move(move);
  };
  lua.new_usertype<base_actor>(
      "base_actor", sol::constructors<sol::types<base_scene &>>(),
      "get_position", base_actor_get_position, "set_position",
      base_actor_set_position, "get_rotation", base_actor_get_rotation,
      "set_rotation", base_actor_set_rotation, "move", base_actor_move);
  lua.new_usertype<base_component>(
      "base_component", sol::constructors<sol::types<base_actor &, float>>());

  return true;
}

void script_system::DoScript(std::string_view fileName) {
  lua.script_file(fileName.data());
}

} // namespace nen
