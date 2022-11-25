#include <actor/actor.hpp>
#include <camera/camera.hpp>
#include <component/draw3d_component.hpp>
#include <render/renderer.hpp>

namespace sinen {
draw3d_component::draw3d_component(actor &owner) : component(owner) {}
draw3d_component::~draw3d_component() {}
void draw3d_component::update(float delta_time) {
  auto obj = std::make_shared<drawable>();
  obj->binding_texture = this->texture_handle;
  obj->is_draw_depth = this->is_draw_depth;
  obj->vertexIndex = this->vertex_name;
  obj->param.world = this->get_actor().get_world_matrix();
  obj->param.view = camera::view();
  obj->param.proj = camera::projection();
  renderer::draw3d(obj);
}
} // namespace sinen