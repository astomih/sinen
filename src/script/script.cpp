#include "../audio/sound_system.hpp"
#include "../render/render_system.hpp"
#include "../texture/texture_system.hpp"
#include "math/vector3.hpp"
#include "script_system.hpp"
#include <audio/music.hpp>
#include <audio/sound.hpp>
#include <camera/camera.hpp>
#include <font/font.hpp>
#include <functional>
#include <io/data_stream.hpp>
#include <main/main.hpp>
#include <math/point2.hpp>
#include <math/random.hpp>
#include <model/model.hpp>
#include <render/renderer.hpp>
#include <script/script.hpp>
#include <script_engine.hpp>
#include <sol/sol.hpp>

#ifdef main
#undef main
#endif

namespace sinen {
class script_system::implement {
public:
  sol::state state;
};
std::unique_ptr<script_system::implement> script_system::impl = nullptr;
void *script::get_state() { return script_system::get_state(); }
void *script_system::get_state() { return (void *)&impl->state; }
bool script_system::initialize() {
  impl = std::make_unique<implement>();
  return script_engine::initialize(impl->state);
}

void script_system::do_script(std::string_view fileName) {
  impl->state.script(data_stream::open_as_string(asset_type::Script, fileName));
}

void script_system::shutdown() { impl->state.collect_gc(); }

void *script_system::new_table(std::string_view table_name) {
  auto *table = new sol::table(impl->state.create_table(table_name));
  return (void *)table;
}

void script_system::register_function(std::string_view name,
                                      std::function<void()> function,
                                      void *table) {
  if (table) {
    auto *t = (sol::table *)table;
    (*t)[name] = function;
    return;
  }
  impl->state[name] = function;
}

void script::do_script(std::string_view fileName) {
  script_system::do_script(fileName);
}

script::table_handler script::new_table(std::string_view table_name) {
  return script_system::new_table(table_name);
}

void script::register_function(std::string_view name,
                               std::function<void()> function,
                               table_handler handler) {
  script_system::register_function(name, function, handler);
}

} // namespace sinen
