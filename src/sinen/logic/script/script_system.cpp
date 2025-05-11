// std
#include <functional>

// internal
#include "pocketpy/pocketpy.h"
#include "pybind11/internal/builtins.h"
#include "pybind11/internal/object.h"
#include "pybind11/pybind11.h"
#include "script_system.hpp"
#include <core/io/data_stream.hpp>
#include <script_engine.hpp>

// external
#include <sol/sol.hpp>

namespace sinen {
class script_system::implement {
public:
  sol::state state;
};
std::unique_ptr<script_system::implement> script_system::impl = nullptr;
void *script_system::get_state() { return impl->state.lua_state(); }
void *script_system::get_sol_state() { return &impl->state; }
bool script_system::initialize() {
  impl = std::make_unique<implement>();
  py::initialize();
  return script_engine::initialize(impl->state);
}

void script_system::do_script(std::string_view fileName) {
  impl->state.script(DataStream::open_as_string(AssetType::Script, fileName));
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

} // namespace sinen
