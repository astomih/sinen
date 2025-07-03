#include "../../asset/model/model_data.hpp"
#include "../../graphics/graphics_system.hpp"
#include "graphics_system.hpp"
#include <SDL3/SDL_events.h>
#include <asset/asset.hpp>
#include <cstring>
#include <graphics/drawable/drawable_wrapper.hpp>
#include <graphics/graphics.hpp>
#include <logic/camera/camera.hpp>
#include <logic/scene/scene.hpp>
#include <platform/window/window.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include <glm/mat4x4.hpp>

namespace sinen {
bool Graphics::offscreen_rendering = false;
glm::mat4 Graphics::render_texture_user_data;

void Graphics::unload_data() { GraphicsSystem::unload_data(); }

void Graphics::render() { GraphicsSystem::render(); }
void Graphics::Draw2D(const sinen::Draw2D &draw2d) {
  auto ratio = glm::vec2(Window::Size().x / Scene::Size().x,
                         Window::Size().y / Scene::Size().y);

  auto t = glm::translate(glm::mat4(1.0f),
                          glm::vec3(draw2d.position.x * ratio.x,
                                    draw2d.position.y * ratio.y, 0.0f));
  auto quaternion = glm::angleAxis(glm::radians(draw2d.rotation),
                                   glm::vec3(0.0f, 0.0f, -1.0f));
  auto r = glm::toMat4(quaternion);

  auto s = glm::scale(glm::mat4(1.0f), glm::vec3(draw2d.scale.x * 0.5f,
                                                 draw2d.scale.y * 0.5f, 1.0f));

  draw2d.obj->param.world = t * r * s;
  draw2d.obj->material = draw2d.material;
  auto viewproj = glm::mat4(1.0f);

  auto screen_size = Scene::Size();
  viewproj[0][0] = 2.f / Window::Size().x;
  viewproj[1][1] = 2.f / Window::Size().y;
  draw2d.obj->param.proj = viewproj;
  draw2d.obj->param.view = glm::mat4(1.f);
  if (GetModelData(draw2d.model.data)->vertexBuffer == nullptr) {
    draw2d.obj->model = GraphicsSystem::sprite;
  } else
    draw2d.obj->model = draw2d.model;
  for (auto &i : draw2d.worlds) {
    auto t = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(i.position.x * ratio.x, i.position.y * ratio.y, 0.0f));
    auto quaternion =
        glm::angleAxis(glm::radians(i.rotation), glm::vec3(0.0f, 0.0f, -1.0f));
    auto r = glm::toMat4(quaternion);
    auto s = glm::scale(glm::mat4(1.0f),
                        glm::vec3(i.scale.x * 0.5f, i.scale.y * 0.5f, 1.0f));

    auto world = t * r * s;

    InstanceData insdata;
    draw2d.obj->world_to_instance_data(world, insdata);
    draw2d.obj->data.push_back(insdata);
  }
  GraphicsSystem::draw2d(draw2d.obj);
}
void Graphics::Draw3D(const sinen::Draw3D &draw3d) {
  draw3d.obj->material = draw3d.material;
  {
    const auto t = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(draw3d.position.x, draw3d.position.y, draw3d.position.z));
    const auto rotationX = glm::angleAxis(glm::radians(draw3d.rotation.x),
                                          glm::vec3(1.0f, 0.0f, 0.0f));
    const auto rotationY = glm::angleAxis(glm::radians(draw3d.rotation.y),
                                          glm::vec3(0.0f, 1.0f, 0.0f));
    const auto rotationZ = glm::angleAxis(glm::radians(draw3d.rotation.z),
                                          glm::vec3(0.0f, 0.0f, 1.0f));
    const auto r = glm::toMat4(rotationX * rotationY * rotationZ);

    const auto s =
        glm::scale(glm::mat4(1.0f),
                   glm::vec3(draw3d.scale.x, draw3d.scale.y, draw3d.scale.z));

    auto world = t * r * s;
    draw3d.obj->param.world = world;
    draw3d.obj->param.proj = Scene::GetCamera().Projection();
    draw3d.obj->param.view = Scene::GetCamera().GetView();
  }
  if (GetModelData(draw3d.model.data)->vertexBuffer == nullptr) {
    draw3d.obj->model = GraphicsSystem::box;
  } else
    draw3d.obj->model = draw3d.model;
  for (auto &i : draw3d.worlds) {
    InstanceData insdata{};
    auto t = glm::translate(
        glm::mat4(1.0f), glm::vec3(i.position.x, i.position.y, i.position.z));
    auto rotationX =
        glm::angleAxis(glm::radians(i.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    auto rotationY =
        glm::angleAxis(glm::radians(i.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    auto rotationZ =
        glm::angleAxis(glm::radians(i.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    auto r = glm::toMat4(rotationX * rotationY * rotationZ);

    auto s =
        glm::scale(glm::mat4(1.0f), glm::vec3(i.scale.x, i.scale.y, i.scale.z));

    auto world = t * r * s;

    draw3d.obj->world_to_instance_data(world, insdata);
    draw3d.obj->data.push_back(insdata);
  }
  GraphicsSystem::draw3d(draw3d.obj);
}
void Graphics::SetClearColor(const Color &color) {
  GraphicsSystem::set_clear_color(color);
}
Color Graphics::GetClearColor() { return GraphicsSystem::get_clear_color(); }
void Graphics::toggle_show_imgui() { GraphicsSystem::toggle_show_imgui(); }
bool Graphics::is_show_imgui() { return GraphicsSystem::is_show_imgui(); }
std::list<std::function<void()>> &Graphics::get_imgui_function() {
  return GraphicsSystem::get_imgui_function();
}

void Graphics::add_imgui_function(std::function<void()> function) {
  GraphicsSystem::get_imgui_function().push_back(function);
}
void *Graphics::get_texture_id() { return GraphicsSystem::get_texture_id(); }

void Graphics::BindPipeline3D(const GraphicsPipeline3D &pipeline) {
  GraphicsSystem::bind_pipeline3d(pipeline);
}
void Graphics::BindDefaultPipeline3D() {
  GraphicsSystem::bind_default_pipeline3d();
}
void Graphics::BindPipeline2D(const GraphicsPipeline2D &pipeline) {
  GraphicsSystem::bind_pipeline2d(pipeline);
}
void Graphics::BindDefaultPipeline2D() {
  GraphicsSystem::bind_default_pipeline2d();
}
void Graphics::SetUniformData(uint32_t slot, const UniformData &data) {
  GraphicsSystem::set_uniform_data(slot, data);
}
void Graphics::BeginTarget2D(const RenderTexture &texture) {
  GraphicsSystem::begin_target2d(texture);
}
void Graphics::BeginTarget3D(const RenderTexture &texture) {
  GraphicsSystem::begin_target3d(texture);
}
void Graphics::EndTarget(const RenderTexture &texture, Texture &out) {
  GraphicsSystem::end_target(texture, out);
}
} // namespace sinen
