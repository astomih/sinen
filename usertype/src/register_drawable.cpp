#include "register_script.hpp"

#include <drawable/drawable_wrapper.hpp>

namespace sinen {
void register_drawable(sol::state &lua) {
  lua["draw2d"] = []() -> draw2d { return draw2d(); };
  lua["draw2d"] = [](texture t) -> draw2d { return draw2d(t); };
  lua["drawui"] = []() -> drawui { return drawui(); };
  lua["drawui"] = [](texture t) -> drawui { return drawui(t); };
  lua["draw3d"] = []() -> draw3d { return draw3d(); };
  lua["draw3d"] = [](texture t) -> draw3d { return draw3d(t); };
  {
    auto v = lua.new_usertype<draw2d>("", sol::no_construction());
    v["draw"] = &draw2d::draw;
    v["position"] = &draw2d::position;
    v["rotation"] = &draw2d::rotation;
    v["scale"] = &draw2d::scale;
    v["texture"] = &draw2d::texture_handle;
    v["vertex_name"] = &draw2d::vertex_name;
    v["user_data_at"] = &draw2d::user_data_at;
    v["add"] = &draw2d::add;
    v["at"] = &draw2d::at;
    v["clear"] = &draw2d::clear;
  }
  {
    auto v = lua.new_usertype<drawui>("", sol::no_construction());
    v["draw"] = &drawui::draw;
    v["position"] = &drawui::position;
    v["rotation"] = &drawui::rotation;
    v["scale"] = &drawui::scale;
    v["texture"] = &drawui::texture_handle;
    v["vertex_name"] = &drawui::vertex_name;
    v["user_data_at"] = &drawui::user_data_at;
    v["add"] = &drawui::add;
    v["at"] = &drawui::at;
    v["clear"] = &drawui::clear;
  }
  {
    auto v = lua.new_usertype<draw3d>("", sol::no_construction());
    v["position"] = &draw3d::position;
    v["rotation"] = &draw3d::rotation;
    v["scale"] = &draw3d::scale;
    v["texture"] = &draw3d::texture_handle;
    v["draw"] = &draw3d::draw;
    v["vertex_name"] = &draw3d::vertex_name;
    v["is_draw_depth"] = &draw3d::is_draw_depth;
    v["user_data_at"] = &draw3d::user_data_at;
    v["add"] = &draw3d::add;
    v["at"] = &draw3d::at;
    v["clear"] = &draw3d::clear;
  }
}
} // namespace sinen
