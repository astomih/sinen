#include "register_script.hpp"

#include <drawable/drawable_wrapper.hpp>

namespace sinen {
void register_drawable(sol::state &lua) {
  lua["Draw2D"] = []() -> Draw2D { return Draw2D(); };
  lua["Draw2D"] = [](Texture t) -> Draw2D { return Draw2D(t); };
  lua["Draw3D"] = []() -> Draw3D { return Draw3D(); };
  lua["Draw3D"] = [](Texture t) -> Draw3D { return Draw3D(t); };
  {
    auto v = lua.new_usertype<Draw2D>("", sol::no_construction());
    v["draw"] = &Draw2D::draw;
    v["position"] = &Draw2D::position;
    v["rotation"] = &Draw2D::rotation;
    v["scale"] = &Draw2D::scale;
    v["texture"] = &Draw2D::texture_handle;
    v["vertex_name"] = &Draw2D::vertex_name;
    v["user_data_at"] = &Draw2D::user_data_at;
    v["add"] = &Draw2D::add;
    v["at"] = &Draw2D::at;
    v["clear"] = &Draw2D::clear;
  }
  {
    auto v = lua.new_usertype<Draw3D>("", sol::no_construction());
    v["position"] = &Draw3D::position;
    v["rotation"] = &Draw3D::rotation;
    v["scale"] = &Draw3D::scale;
    v["texture"] = &Draw3D::texture_handle;
    v["draw"] = &Draw3D::draw;
    v["model"] = &Draw3D::model;
    v["is_draw_depth"] = &Draw3D::is_draw_depth;
    v["user_data_at"] = &Draw3D::user_data_at;
    v["add"] = &Draw3D::add;
    v["at"] = &Draw3D::at;
    v["clear"] = &Draw3D::clear;
  }
}
} // namespace sinen
