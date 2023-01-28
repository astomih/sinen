#include "../../render/render_system.hpp"
#include "../../texture/texture_system.hpp"
#include "../../window/window_system.hpp"
#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif
#include <SDL.h>
#include <SDL_image.h>
#include <scene/scene.hpp>
#include <sol/sol.hpp>

#if defined(EMSCRIPTEN) || defined(ANDROID)
#define GL_GLEXT_PROTOTYPES
#include <GLES3/gl3.h>
#include <SDL_opengles2.h>
#endif

#ifndef ANDROID
#include <GL/glew.h>
#endif

#include <cstdint>
#include <vector>

#include "../../texture/texture_system.hpp"
#include "gl_renderer.hpp"
#include "gl_uniform_buffer.hpp"
#include <camera/camera.hpp>
#include <fstream>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <io/data_stream.hpp>
#include <iostream>
#include <logger/logger.hpp>
#include <sstream>
#include <window/window.hpp>

namespace sinen {
gl_renderer::gl_renderer() {}
gl_renderer::~gl_renderer() = default;

void gl_renderer::initialize() {
  mContext = SDL_GL_CreateContext(window_system::get_sdl_window());
  SDL_GL_MakeCurrent(window_system::get_sdl_window(), mContext);
  if (!SDL_GL_SetSwapInterval(1)) {
    SDL_GL_SetSwapInterval(0);
  }
  prev_window_x = window_system::size().x;
  prev_window_y = window_system::size().y;
#if !defined ANDROID
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    logger::error("GLEW Init error.");
  }
  glGetError();
#endif

  prepare();
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
  io.IniFilename = NULL;
  io.Fonts->AddFontFromFileTTF(
      data_stream::convert_file_path(asset_type::Font,
                                     "mplus/mplus-1p-medium.ttf")
          .data(),
      18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
  ImGui_ImplSDL2_InitForOpenGL(window_system::get_sdl_window(), mContext);
#if defined EMSCRIPTEN || defined MOBILE
  ImGui_ImplOpenGL3_Init("#version 300 es");
#else
  ImGui_ImplOpenGL3_Init("#version 330 core");
#endif
}

void gl_renderer::shutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui::DestroyContext();
  SDL_GL_DeleteContext(mContext);
}

void gl_renderer::render() {

  auto w = window_system::size();
  if (w.x != prev_window_x || w.y != prev_window_y) {
    glViewport(0, 0, w.x, w.y);
    prev_window_x = w.x;
    prev_window_y = w.y;
    destroy_render_texture();
    create_render_texture();
  }
  auto color = render_system::get_clear_color();
  glClearColor(color.r, color.g, color.b, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, window::size().x, window::size().y);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
  disable_vertex_attrib_array();
  draw_skybox();
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  draw_3d();
  enable_vertex_attrib_array();
  draw_instancing_3d();
  glDisable(GL_DEPTH_TEST);
  gl_shader_parameter param;

  glEnable(GL_BLEND);
  glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
  disable_vertex_attrib_array();
  draw_2d();
  enable_vertex_attrib_array();
  draw_instancing_2d();
  glFlush();
  glActiveTexture(GL_TEXTURE0);
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(m_VertexArrays["SPRITE"].vao);
    glBindTexture(GL_TEXTURE_2D, rendertexture);
    m_shaders["RenderTexture"].active(0);

    render_texture_ubo.bind(m_shaders["RenderTexture"].program(), 0);
    render_texture_ubo.update(0, sizeof(gl_shader_parameter), &param, 0);
    disable_vertex_attrib_array();
    glDrawElements(GL_TRIANGLES, m_VertexArrays["SPRITE"].indices.size(),
                   GL_UNSIGNED_INT, nullptr);
  }
  draw_ui();

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame(window_system::get_sdl_window());
  ImGui::NewFrame();
  if (renderer::is_show_imgui()) {

    // Draw ImGUI widgets.
    for (auto &i : renderer::get_imgui_function()) {
      i();
    }
  }

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  SDL_GL_SwapWindow(window_system::get_sdl_window());
  for (auto &i : m_drawer_3ds) {
    i.ubo.destroy();
  }
  m_drawer_3ds.clear();
  for (auto &i : m_drawer_2ds) {
    i.ubo.destroy();
  }
  m_drawer_2ds.clear();
  for (auto &i : m_drawer_uis) {
    i.ubo.destroy();
  }
  m_drawer_uis.clear();
  for (auto &i : m_instancing_2d) {
    glDeleteBuffers(1, &i.vbo);
    i.ubo.destroy();
    glDeleteVertexArrays(1, &i.vao);
  }
  for (auto &i : m_instancing_3d) {
    glDeleteBuffers(1, &i.vbo);
    i.ubo.destroy();
    glDeleteVertexArrays(1, &i.vao);
  }
  m_instancing_3d.clear();
  m_instancing_2d.clear();
}

void gl_renderer::enable_vertex_attrib_array() {
  glVertexAttribDivisor(4, 1);
  glVertexAttribDivisor(5, 1);
  glVertexAttribDivisor(6, 1);
  glVertexAttribDivisor(7, 1);
  for (size_t i = 0; i < 8; i++) {
    glEnableVertexAttribArray(i);
  }
}

void gl_renderer::disable_vertex_attrib_array() {
  glDisableVertexAttribArray(4);
  glDisableVertexAttribArray(5);
  glDisableVertexAttribArray(6);
  glDisableVertexAttribArray(7);
  glVertexAttribDivisor(4, 0);
  glVertexAttribDivisor(5, 0);
  glVertexAttribDivisor(6, 0);
  glVertexAttribDivisor(7, 0);
}

void gl_renderer::draw_skybox() {
  create_texture(render_system::get_skybox_texture());
  gl_uniform_buffer ubo;
  auto &va = m_VertexArrays["BOX"];
  glBindVertexArray(va.vao);
  m_shaders["Alpha"].active(0);
  gl_shader_parameter param;
  param.param.proj = scene::main_camera().projection();
  param.param.view = matrix4::lookat(vector3(0, 0, 0),
                                     scene::main_camera().target() -
                                         scene::main_camera().position(),
                                     scene::main_camera().up());
  matrix4 w = matrix4::identity;
  w[0][0] = 5;
  w[1][1] = 5;
  w[2][2] = 5;
  ubo.create(0, sizeof(drawable::parameter), &param);
  glBindTexture(GL_TEXTURE_2D,
                mTextureIDs[render_system::get_skybox_texture().handle]);
  ubo.bind(m_shaders["Alpha"].program(), 0);
  disable_vertex_attrib_array();
  glDrawElements(GL_TRIANGLES, va.indices.size(), GL_UNSIGNED_INT, nullptr);
  ubo.destroy();
}

void gl_renderer::draw_3d() {
  for (auto &i : m_drawer_3ds) {
    auto &va = m_VertexArrays[i.drawable_object->vertexIndex];
    glBindVertexArray(va.vao);
    gl_shader_parameter param;
    param.param = i.drawable_object->param;
    m_shaders["Normal"].active(0);
    i.ubo.bind(m_shaders["Normal"].program(), 0);
    i.ubo.update(0, sizeof(gl_shader_parameter), &param, 0);
    if (i.drawable_object->shade.get_parameter_size() > 0) {
      i.ubo.update(0, i.drawable_object->shade.get_parameter_size(),
                   i.drawable_object->shade.get_parameter().get(),
                   sizeof(gl_shader_parameter));
    }
    disable_vertex_attrib_array();
    glBindTexture(GL_TEXTURE_2D,
                  mTextureIDs[i.drawable_object->binding_texture.handle]);
    glDrawElements(GL_TRIANGLES, va.indices.size(), GL_UNSIGNED_INT, nullptr);
  }
}
void gl_renderer::draw_instancing_3d() {
  for (auto &i : m_instancing_3d) {
    gl_shader_parameter param;
    param.param = i.ins.object->param;
    m_shaders["NormalInstance"].active(0);
    i.ubo.bind(m_shaders["NormalInstance"].program(), 0);
    i.ubo.update(0, sizeof(gl_shader_parameter), &param, 0);
    if (i.ins.object->shade.get_parameter_size() > 0) {
      i.ubo.update(0, i.ins.object->shade.get_parameter_size(),
                   i.ins.object->shade.get_parameter().get(),
                   sizeof(gl_shader_parameter));
    }

    auto &va = m_VertexArrays[i.ins.object->vertexIndex];
    glBindVertexArray(i.vao);
    glBindTexture(GL_TEXTURE_2D,
                  mTextureIDs[i.ins.object->binding_texture.handle]);
    enable_vertex_attrib_array();
    glDrawElementsInstanced(GL_TRIANGLES, va.indices.size(), GL_UNSIGNED_INT,
                            nullptr, i.ins.data.size());
  }
}

void gl_renderer::draw_2d() {
  for (auto &i : m_drawer_2ds) {
    glBindVertexArray(m_VertexArrays[i.drawable_object->vertexIndex].vao);
    glBindTexture(GL_TEXTURE_2D,
                  mTextureIDs[i.drawable_object->binding_texture.handle]);
    if (i.drawable_object->shade.vertex_shader() == "default" &&
        i.drawable_object->shade.fragment_shader() == "default") {
      m_shaders["Alpha"].active(0);
      i.ubo.bind(m_shaders["Alpha"].program(), 0);
      i.ubo.update(0, sizeof(drawable::parameter), &i.drawable_object->param,
                   0);
      if (i.drawable_object->shade.get_parameter_size() > 0) {
        i.ubo.update(0, i.drawable_object->shade.get_parameter_size(),
                     i.drawable_object->shade.get_parameter().get(),
                     sizeof(gl_shader_parameter));
      }
    } else {
      for (auto &j : m_user_pipelines) {
        if (j.first == i.drawable_object->shade) {
          j.second.active(0);
          i.ubo.bind(j.second.program(), 0);
          i.ubo.update(0, sizeof(drawable::parameter),
                       &i.drawable_object->param, 0);
          if (i.drawable_object->shade.get_parameter_size() > 0) {
            i.ubo.update(0, i.drawable_object->shade.get_parameter_size(),
                         i.drawable_object->shade.get_parameter().get(),
                         sizeof(gl_shader_parameter));
          }
        }
      }
    }
    disable_vertex_attrib_array();
    glDrawElements(
        GL_TRIANGLES,
        m_VertexArrays[i.drawable_object->vertexIndex].indices.size(),
        GL_UNSIGNED_INT, nullptr);
  }
}

void gl_renderer::draw_instancing_2d() {
  for (auto &i : m_instancing_2d) {
    m_shaders["AlphaInstance"].active(0);
    i.ubo.bind(m_shaders["AlphaInstance"].program(), 0);
    i.ubo.update(0, sizeof(drawable::parameter), &i.ins.object->param, 0);
    if (i.ins.object->shade.get_parameter_size() > 0) {
      i.ubo.update(0, i.ins.object->shade.get_parameter_size(),
                   i.ins.object->shade.get_parameter().get(),
                   sizeof(gl_shader_parameter));
    }

    auto &va = m_VertexArrays[i.ins.object->vertexIndex];
    glBindVertexArray(i.vao);

    glBindTexture(GL_TEXTURE_2D,
                  mTextureIDs[i.ins.object->binding_texture.handle]);
    enable_vertex_attrib_array();
    glDrawElementsInstanced(GL_TRIANGLES, va.indices.size(), GL_UNSIGNED_INT,
                            nullptr, i.ins.data.size());
  }
}
void gl_renderer::draw_ui() {
  for (auto &i : m_drawer_uis) {
    glBindVertexArray(m_VertexArrays[i.drawable_object->vertexIndex].vao);
    glBindTexture(GL_TEXTURE_2D,
                  mTextureIDs[i.drawable_object->binding_texture.handle]);
    if (i.drawable_object->shade.vertex_shader() == "default" &&
        i.drawable_object->shade.fragment_shader() == "default") {
      m_shaders["Alpha"].active(0);
      i.ubo.bind(m_shaders["Alpha"].program(), 0);
      i.ubo.update(0, sizeof(drawable::parameter), &i.drawable_object->param,
                   0);
      if (i.drawable_object->shade.get_parameter_size() > 0) {
        i.ubo.update(0, i.drawable_object->shade.get_parameter_size(),
                     i.drawable_object->shade.get_parameter().get(),
                     sizeof(gl_shader_parameter));
      }
    } else {
      for (auto &j : m_user_pipelines) {
        if (j.first == i.drawable_object->shade) {
          j.second.active(0);
          i.ubo.bind(j.second.program(), 0);
          i.ubo.update(0, sizeof(drawable::parameter),
                       &i.drawable_object->param, 0);
          if (i.drawable_object->shade.get_parameter_size() > 0) {
            i.ubo.update(0, i.drawable_object->shade.get_parameter_size(),
                         i.drawable_object->shade.get_parameter().get(),
                         sizeof(gl_shader_parameter));
          }
        }
      }
    }
    disable_vertex_attrib_array();
    glDrawElements(
        GL_TRIANGLES,
        m_VertexArrays[i.drawable_object->vertexIndex].indices.size(),
        GL_UNSIGNED_INT, nullptr);
  }
}

void gl_renderer::add_vertex_array(const vertex_array &vArray,
                                   std::string_view name) {
  if (m_VertexArrays.contains(name.data())) {
    return;
  }
  gl_vertex_array vArrayGL;
  vArrayGL.indexCount = vArray.indexCount;
  vArrayGL.indices = vArray.indices;
  vArrayGL.vertices = vArray.vertices;
  vArrayGL.materialName = vArray.materialName;

  // Create vao
  glGenVertexArrays(1, &vArrayGL.vao);
  glBindVertexArray(vArrayGL.vao);

  // Create vbo
  glGenBuffers(1, &vArrayGL.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vArrayGL.vbo);
  auto vArraySize = vArrayGL.vertices.size() * sizeof(vertex);
  glBufferData(GL_ARRAY_BUFFER, vArraySize, vArrayGL.vertices.data(),
               GL_DYNAMIC_DRAW);

  // Prepare vertex attribute
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float),
                        reinterpret_cast<void *>(sizeof(float) * 3));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float),
                        reinterpret_cast<void *>(sizeof(float) * 6));
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float),
                        reinterpret_cast<void *>(sizeof(float) * 8));
  // Create IBO
  glGenBuffers(1, &vArrayGL.ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vArrayGL.ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               vArrayGL.indices.size() * sizeof(uint32_t),
               vArrayGL.indices.data(), GL_DYNAMIC_DRAW);
  m_VertexArrays.emplace(std::string(name), vArrayGL);
}
void gl_renderer::update_vertex_array(const vertex_array &vArray,
                                      std::string_view name) {
  gl_vertex_array vArrayGL;
  vArrayGL.indexCount = vArray.indexCount;
  vArrayGL.indices = vArray.indices;
  vArrayGL.vertices = vArray.vertices;
  vArrayGL.materialName = vArray.materialName;
  // Update vbo
  glBindBuffer(GL_ARRAY_BUFFER, m_VertexArrays[name.data()].vbo);
  auto vArraySize = vArrayGL.vertices.size() * sizeof(vertex);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vArraySize, vArrayGL.vertices.data());
  // Update ibo
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VertexArrays[name.data()].ibo);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
                  vArrayGL.indices.size() * sizeof(uint32_t),
                  vArrayGL.indices.data());
}
void gl_renderer::add_model(const model &m) {
  gl_vertex_array vArrayGL;
  vArrayGL.indexCount = m.all_indices().size();
  vArrayGL.indices = m.all_indices();
  vArrayGL.vertices = m.all_vertex();
  vArrayGL.materialName = m.v_array.materialName;

  // Create vao
  glGenVertexArrays(1, &vArrayGL.vao);
  glBindVertexArray(vArrayGL.vao);

  // Create vbo
  glGenBuffers(1, &vArrayGL.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vArrayGL.vbo);
  auto vArraySize = vArrayGL.vertices.size() * sizeof(vertex);
  glBufferData(GL_ARRAY_BUFFER, vArraySize, vArrayGL.vertices.data(),
               GL_DYNAMIC_DRAW);

  // Prepare vertex attribute
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float),
                        reinterpret_cast<void *>(sizeof(float) * 3));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float),
                        reinterpret_cast<void *>(sizeof(float) * 6));
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float),
                        reinterpret_cast<void *>(sizeof(float) * 8));
  // Create IBO
  glGenBuffers(1, &vArrayGL.ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vArrayGL.ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               vArrayGL.indices.size() * sizeof(uint32_t),
               vArrayGL.indices.data(), GL_DYNAMIC_DRAW);
  m_VertexArrays.emplace(std::string(m.name), vArrayGL);
}
void gl_renderer::update_model(const model &m) {
  gl_vertex_array vArrayGL;
  vArrayGL.indexCount = m.all_indices().size();
  vArrayGL.indices = m.all_indices();
  vArrayGL.vertices = m.all_vertex();
  vArrayGL.materialName = m.v_array.materialName;
  // Update vbo
  glBindBuffer(GL_ARRAY_BUFFER, m_VertexArrays[m.name.data()].vbo);
  auto vArraySize = vArrayGL.vertices.size() * sizeof(vertex);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vArraySize, vArrayGL.vertices.data());
  // Update ibo
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VertexArrays[m.name.data()].ibo);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
                  vArrayGL.indices.size() * sizeof(uint32_t),
                  vArrayGL.indices.data());
}
void gl_renderer::draw2d(std::shared_ptr<class drawable> sprite) {
  create_texture(sprite->binding_texture);
  auto iter = m_drawer_2ds.begin();
  for (; iter != m_drawer_2ds.end(); ++iter) {
    if (sprite->drawOrder < (*iter).drawable_object->drawOrder) {
      break;
    }
  }
  gl_uniform_buffer ubo;
  ubo.create(0,
             sizeof(gl_shader_parameter) + sprite->shade.get_parameter_size(),
             &sprite->param);
  ubo.update(0, sprite->shade.get_parameter_size(),
             sprite->shade.get_parameter().get(), sizeof(gl_shader_parameter));
  m_drawer_2ds.insert(iter, {sprite, ubo});
}
void gl_renderer::drawui(std::shared_ptr<class drawable> sprite) {
  create_texture(sprite->binding_texture);
  auto iter = m_drawer_uis.begin();
  for (; iter != m_drawer_uis.end(); ++iter) {
    if (sprite->drawOrder < (*iter).drawable_object->drawOrder) {
      break;
    }
  }
  gl_uniform_buffer ubo;
  ubo.create(0,
             sizeof(gl_shader_parameter) + sprite->shade.get_parameter_size(),
             &sprite->param);
  ubo.update(0, sprite->shade.get_parameter_size(),
             sprite->shade.get_parameter().get(), sizeof(gl_shader_parameter));
  m_drawer_uis.insert(iter, {sprite, ubo});
}

void gl_renderer::draw3d(std::shared_ptr<class drawable> sprite) {
  create_texture(sprite->binding_texture);
  auto iter = m_drawer_3ds.end();
  gl_uniform_buffer ubo;
  ubo.create(0,
             sizeof(gl_shader_parameter) + sprite->shade.get_parameter_size(),
             &sprite->param);
  ubo.update(0, sprite->shade.get_parameter_size(),
             sprite->shade.get_parameter().get(), sizeof(gl_shader_parameter));
  m_drawer_3ds.insert(iter, {sprite, ubo});
}

void gl_renderer::load_shader(const shader &shaderInfo) {
  gl_shader pipeline;
  pipeline.load(shaderInfo.vertex_shader(), shaderInfo.fragment_shader());
  gl_shader_parameter param;
  m_user_pipelines.emplace_back(
      std::pair<shader, gl_shader>{shaderInfo, pipeline});
}

void gl_renderer::unload_shader(const shader &shaderInfo) {
  std::erase_if(m_user_pipelines, [&](auto &x) {
    if (x.first == shaderInfo) {
      x.second.unload();
      return true;
    };
    return false;
  });
}

void gl_renderer::add_instancing(const instancing &_instancing) {
  create_texture(_instancing.object->binding_texture);
  auto ogl = gl_instancing(_instancing);
  auto &va = m_VertexArrays[_instancing.object->vertexIndex];
  glGenVertexArrays(1, &ogl.vao);
  glBindVertexArray(ogl.vao);
  auto size = 12 * sizeof(float);
  glBindBuffer(GL_ARRAY_BUFFER, va.vbo);
  for (size_t i = 0; i < 5; i++) {
    glEnableVertexAttribArray(i);
  }
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, size, 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, size,
                        reinterpret_cast<void *>(sizeof(float) * 3));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, size,
                        reinterpret_cast<void *>(sizeof(float) * 6));
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, size,
                        reinterpret_cast<void *>(sizeof(float) * 8));
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, size,
                        reinterpret_cast<void *>(sizeof(float) * 12));
  uint32_t vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, _instancing.size, _instancing.data.data(),
               GL_DYNAMIC_DRAW);
  size = sizeof(instance_data);
  for (size_t i = 4; i < 8; i++) {
    glEnableVertexAttribArray(i);
  }
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, size,
                        reinterpret_cast<void *>(sizeof(float) * 0));
  glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, size,
                        reinterpret_cast<void *>(sizeof(float) * 4));
  glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, size,
                        reinterpret_cast<void *>(sizeof(float) * 8));
  glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, size,
                        reinterpret_cast<void *>(sizeof(float) * 12));
  glVertexAttribDivisor(0, 0);
  glVertexAttribDivisor(1, 0);
  glVertexAttribDivisor(2, 0);
  glVertexAttribDivisor(3, 0);
  glVertexAttribDivisor(4, 1);
  glVertexAttribDivisor(5, 1);
  glVertexAttribDivisor(6, 1);
  glVertexAttribDivisor(7, 1);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, va.ibo);
  ogl.vbo = vbo;
  ogl.ubo.create(0,
                 sizeof(gl_shader_parameter) +
                     _instancing.object->shade.get_parameter_size(),
                 &_instancing.object->param);
  ogl.ubo.update(0, _instancing.object->shade.get_parameter_size(),
                 _instancing.object->shade.get_parameter().get(),
                 sizeof(gl_shader_parameter));

  if (_instancing.type == object_type::_2D) {
    m_instancing_2d.emplace_back(ogl);

  } else {
    m_instancing_3d.emplace_back(ogl);
  }
}

void gl_renderer::prepare() {
  if (!load_shader()) {
    logger::error("failed to loads shader");
  }
  create_render_texture();
}

void gl_renderer::create_render_texture() {
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  glGenTextures(1, &rendertexture);
  glBindTexture(GL_TEXTURE_2D, rendertexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window::size().x, window::size().y, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glGenRenderbuffers(1, &depthbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, window::size().x,
                        window::size().y);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, depthbuffer);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         rendertexture, 0);
  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, DrawBuffers);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    logger::error("failed to create framebuffer");
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void gl_renderer::destroy_render_texture() {
  glDeleteFramebuffers(1, &framebuffer);
  glDeleteTextures(1, &rendertexture);
  glDeleteRenderbuffers(1, &depthbuffer);
}

void gl_renderer::create_texture(texture handle) {
  ::SDL_Surface &surf = texture_system::get(handle.handle);
  if (mTextureIDs.contains(handle.handle)) {
    if (*handle.is_need_update) {
      destroy_texture(handle);
    } else
      return;
  }
  ::SDL_LockSurface(&surf);
  auto formatbuf = ::SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888);
  formatbuf->BytesPerPixel = 4;
  auto imagedata = ::SDL_ConvertSurface(&surf, formatbuf, 0);
  SDL_UnlockSurface(&surf);
  // Generate a GL texture
  GLuint textureId;
  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf.w, surf.h, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, imagedata->pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  SDL_FreeFormat(formatbuf);
  SDL_FreeSurface(imagedata);
  mTextureIDs[handle.handle] = textureId;
}
void gl_renderer::destroy_texture(texture handle) {
  if (mTextureIDs.contains(handle.handle)) {
    glDeleteTextures(1, &mTextureIDs[handle.handle]);
    mTextureIDs.erase(handle.handle);
  }
}

bool gl_renderer::load_shader() {
  gl_shader_parameter param{};
  m_shaders["RenderTexture"] = gl_shader();
  m_shaders["Normal"] = gl_shader();
  m_shaders["Alpha"] = gl_shader();
  m_shaders["NormalInstance"] = gl_shader();
  m_shaders["AlphaInstance"] = gl_shader();
  if (!m_shaders["RenderTexture"].load("render_texture.vert",
                                       "render_texture.frag")) {
    return false;
  }
  render_texture_ubo.create(0, sizeof(gl_shader_parameter), &param);
  if (!m_shaders["Normal"].load("shader.vert", "shader.frag")) {
    return false;
  }
  if (!m_shaders["Alpha"].load("shader.vert", "alpha.frag")) {
    return false;
  }
  if (!m_shaders["NormalInstance"].load("shader_instance.vert",
                                        "shader.frag")) {
    return false;
  }
  if (!m_shaders["AlphaInstance"].load("shader_instance.vert", "alpha.frag")) {
    return false;
  }
  return true;
}

} // namespace sinen
