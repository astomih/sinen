#include <actor/actor.hpp>
#include <component/script_component.hpp>
#include <io/data_stream.hpp>
#include <script/script.hpp>
#include <sol/sol.hpp>

namespace sinen {
script_component::script_component(actor &owner) : component(owner) {}
script_component::~script_component() {}

void script_component::update(float delta_time) {
  if (!this->script_raw_str.empty()) {
    auto *state = static_cast<sol::state *>(script::get_state());
    state->do_string(this->script_raw_str);
    (*state)["update"]();
  }
}
void script_component::load_script(std::string_view path) {
  this->script_raw_str = data_stream::open_as_string(asset_type::Script, path);
}
} // namespace sinen
