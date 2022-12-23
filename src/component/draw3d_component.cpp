#include <actor/actor.hpp>
#include <camera/camera.hpp>
#include <color/palette.hpp>
#include <component/draw3d_component.hpp>
#include <render/renderer.hpp>
#include <scene/scene.hpp>


namespace sinen {
draw3d_component::draw3d_component(actor &owner) : component(owner) {
  this->texture_handle.fill_color(palette::white());
}
draw3d_component::~draw3d_component() {}
void draw3d_component::update(float delta_time) {
  auto obj = std::make_shared<drawable>();
  obj->binding_texture = this->texture_handle;
  obj->is_draw_depth = this->is_draw_depth;
  obj->vertexIndex = this->vertex_name;
  obj->param.world = this->get_actor().get_world_matrix();
  obj->param.view = scene::main_camera().view();
  obj->param.proj = scene::main_camera().projection();
  renderer::draw3d(obj);
  *this->texture_handle.is_need_update = false;
}
} // namespace sinen
