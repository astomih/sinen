// internal
#include <actor/actor.hpp>
#include <camera/camera.hpp>
#include <color/palette.hpp>
#include <component/draw3d_component.hpp>
#include <render/renderer.hpp>
#include <scene/scene.hpp>

namespace sinen {
draw3d_component::draw3d_component(Actor &owner) : component(owner) {
  this->texture_handle.fill_color(Palette::white());
  draw_object = std::make_shared<Drawable>();
  draw_object->binding_texture = this->texture_handle;
  draw_object->vertexIndex = this->vertex_name;
  draw_object->param.world = this->get_actor().get_world_matrix();
  draw_object->param.view = Scene::main_camera().view();
  draw_object->param.proj = Scene::main_camera().projection();
  Renderer::add_queue_3d(draw_object);
  *this->texture_handle.is_need_update = false;
}
draw3d_component::~draw3d_component() {
  Renderer::remove_queue_3d(draw_object);
}
void draw3d_component::update(float delta_time) {
  draw_object->param.world = this->get_actor().get_world_matrix();
  draw_object->param.view = Scene::main_camera().view();
  draw_object->param.proj = Scene::main_camera().projection();
  draw_object->binding_texture = this->texture_handle;
  draw_object->vertexIndex = this->vertex_name;
}
} // namespace sinen
