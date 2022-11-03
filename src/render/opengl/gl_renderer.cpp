#include "../../render/render_system.hpp"
#include "../../script/script_system.hpp"
#include "../../texture/texture_system.hpp"
#include "../../window/window_system.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <sol/sol.hpp>

#if defined(EMSCRIPTEN) || defined(MOBILE)
#include <GLES3/gl3.h>
#include <SDL_opengles2.h>
#define glGenFramebuffers glGenFramebuffersEXT
#define glBindFramebuffer glBindFramebufferEXT
#endif

#ifndef MOBILE
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
#include <io/dstream.hpp>
#include <iostream>
#include <logger/logger.hpp>
#include <sstream>
#include <window/window.hpp>

namespace sinen {
gl_renderer::gl_renderer() {}
gl_renderer::~gl_renderer() = default;

auto light_view =
    matrix4::lookat(vector3(0.5, 2, 2), vector3(0), vector3(0, 1, 0));

auto light_projection = matrix4::ortho(20, 20, 0.5, 100);
// auto light_projection = matrix4::perspective(math::to_radians(90), 1, 0.5,
// 10);

void gl_renderer::initialize() {
  mContext = SDL_GL_CreateContext(window_system::get_sdl_window());
  SDL_GL_MakeCurrent(window_system::get_sdl_window(), mContext);
  if (!SDL_GL_SetSwapInterval(1)) {
    SDL_GL_SetSwapInterval(0);
  }
  prev_window_x = window_system::size().x;
  prev_window_y = window_system::size().y;
#if !defined EMSCRIPTEN && !defined MOBILE
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
      dstream::convert_file_path("mplus/mplus-1p-medium.ttf", asset_type::Font)
          .data(),
      18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
  ImGui_ImplSDL2_InitForOpenGL(window_system::get_sdl_window(), mContext);
#if defined EMSCRIPTEN || defined MOBILE
  ImGui_ImplOpenGL3_Init("#version 300 es");
#else
  ImGui_ImplOpenGL3_Init("#version 330 core");
#endif
}

void gl_renderer::shutdown() {}

vector3 eye;
vector3 at;
float width, height;
void gl_renderer::render() {
  auto &lua = (*(sol::state *)script_system::get_state());
  lua["light_eye"] = [&](const vector3 &v) { eye = v; };
  lua["light_at"] = [&](const vector3 &v) { at = v; };
  lua["light_width"] = [&](float v) { width = v; };
  lua["light_height"] = [&](float v) { height = v; };
  light_view = matrix4::lookat(eye, at, vector3(0, 1, 0));
  light_projection = matrix4::ortho(width, height, 0.5, 10);

  auto w = window_system::size();
  if (w.x != prev_window_x || w.y != prev_window_y) {
    glViewport(0, 0, w.x, w.y);
    prev_window_x = w.x;
    prev_window_y = w.y;
  }
  auto color = render_system::get_clear_color();
  glClearColor(color.r, color.g, color.b, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  /*
  glBindFramebuffer(GL_FRAMEBUFFER, shadowframebuffer);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, 1024, 1024);
  glEnable(GL_DEPTH_TEST);
  for (auto &i : m_drawer_3ds) {
    if (!i->is_draw_depth)
      continue;
    auto &va = m_VertexArrays[i->vertexIndex];
    glBindVertexArray(va.vao);
    gl_shader_parameter param;
    param.param = i->param;
    param.projection = light_projection;
    param.view = light_view;
    m_depth_texture_shader.active(0);
    m_depth_texture_shader.update_ubo(0, sizeof(gl_shader_parameter), &param);
    disable_vertex_attrib_array();
    glDrawElements(GL_TRIANGLES, m_VertexArrays[i->vertexIndex].indices.size(),
                   GL_UNSIGNED_INT, nullptr);
  }
  for (auto &i : m_instancing_3d) {
    if (!i.ins.object->is_draw_depth)
      continue;
    gl_shader_parameter param;
    param.param = i.ins.object->param;
    param.projection = light_projection;
    param.view = light_view;
    m_depth_texture_instanced_shader.active(0);
    m_depth_texture_instanced_shader.update_ubo(0, sizeof(gl_shader_parameter),
                                                &param);

    auto &va = m_VertexArrays[i.ins.object->vertexIndex];
    glBindVertexArray(i.vao);

    enable_vertex_attrib_array();
    glDrawElementsInstanced(GL_TRIANGLES, va.indices.size(), GL_UNSIGNED_INT,
                            nullptr, i.ins.data.size());
  }
  glFlush();
  glDisable(GL_DEPTH_TEST);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, 1280, 720); // Render on the whole framebuffer, complete
  */
  glEnable(GL_BLEND);
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
  drawable::parameter param;

  glEnable(GL_BLEND);
  glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
  disable_vertex_attrib_array();
  draw_2d();
  enable_vertex_attrib_array();
  draw_instancing_2d();
  // glFlush();
  // glBindFramebuffer(GL_FRAMEBUFFER, 0);
  /*
  glBindVertexArray(m_VertexArrays["SPRITE"].vao);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rendertexture);
  glUniform1i(
      glGetUniformLocation(m_render_texture_shader.program(), "diffuseMap"), 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, shadowdepthtexture);
  glUniform1i(
      glGetUniformLocation(m_render_texture_shader.program(), "shadowMap"), 1);
  m_render_texture_shader.active(0);
  m_render_texture_shader.update_ubo(0, sizeof(shader_parameter), &param);
  disable_vertex_attrib_array();
  glDrawElements(GL_TRIANGLES, m_VertexArrays["SPRITE"].indices.size(),
                 GL_UNSIGNED_INT, nullptr);
  glActiveTexture(GL_TEXTURE0);
                 */
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
  m_drawer_3ds.clear();
  m_drawer_2ds.clear();
  SDL_GL_SwapWindow(window_system::get_sdl_window());
  for (auto &i : m_instancing_2d) {
    glDeleteBuffers(1, &i.vbo);
    glDeleteVertexArrays(1, &i.vao);
  }
  for (auto &i : m_instancing_3d) {
    glDeleteBuffers(1, &i.vbo);
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
  mAlphaShader.active(0);
  drawable::parameter param;
  matrix4 w = matrix4::identity;
  w[0][0] = 5;
  w[1][1] = 5;
  w[2][2] = 5;
  param.proj = camera::projection();
  param.view = matrix4::lookat(
      vector3(0, 0, 0), camera::target() - camera::position(), camera::up());
  mAlphaShader.active(0);
  ubo.create(0, sizeof(drawable::parameter), &param);
  glBindTexture(GL_TEXTURE_2D,
                mTextureIDs[render_system::get_skybox_texture().handle]);
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
    param.projection = light_projection;
    param.view = light_view;
    mSpriteShader.active(0);
    i.ubo.bind(mSpriteShader.program(), 0);
    i.ubo.update(0, sizeof(gl_shader_parameter), &param, 0);
    if (i.drawable_object->shader_data.get_parameter_size() > 0) {
      i.ubo.update(0, i.drawable_object->shader_data.get_parameter_size(),
                   i.drawable_object->shader_data.get_parameter().get(),
                   sizeof(gl_shader_parameter));
    }
    disable_vertex_attrib_array();
    /*
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadowdepthtexture);
    glUniform1i(glGetUniformLocation(mSpriteShader.program(), "shadowMap"), 0);
    glActiveTexture(GL_TEXTURE1);
    */
    glBindTexture(GL_TEXTURE_2D,
                  mTextureIDs[i.drawable_object->texture_handle.handle]);
    // glUniform1i(glGetUniformLocation(mSpriteShader.program(), "diffuseMap"),
    // 1);
    glDrawElements(GL_TRIANGLES, va.indices.size(), GL_UNSIGNED_INT, nullptr);
    // glActiveTexture(GL_TEXTURE0);
  }
}
void gl_renderer::draw_instancing_3d() {
  for (auto &i : m_instancing_3d) {
    gl_shader_parameter param;
    param.param = i.ins.object->param;
    param.projection = light_projection;
    param.view = light_view;
    mSpriteInstanceShader.active(0);
    i.ubo.bind(mSpriteInstanceShader.program(), 0);
    i.ubo.update(0, sizeof(gl_shader_parameter), &param, 0);
    if (i.ins.object->shader_data.get_parameter_size() > 0) {
      i.ubo.update(0, i.ins.object->shader_data.get_parameter_size(),
                   i.ins.object->shader_data.get_parameter().get(),
                   sizeof(gl_shader_parameter));
    }

    auto &va = m_VertexArrays[i.ins.object->vertexIndex];
    glBindVertexArray(i.vao);
    /*
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadowdepthtexture);
    glUniform1i(
        glGetUniformLocation(mSpriteInstanceShader.program(), "shadowMap"), 0);
    glActiveTexture(GL_TEXTURE1);
    */
    glBindTexture(GL_TEXTURE_2D,
                  mTextureIDs[i.ins.object->texture_handle.handle]);
    /*
    glUniform1i(
        glGetUniformLocation(mSpriteInstanceShader.program(), "diffuseMap"), 1);
        */
    enable_vertex_attrib_array();
    glDrawElementsInstanced(GL_TRIANGLES, va.indices.size(), GL_UNSIGNED_INT,
                            nullptr, i.ins.data.size());
    // glActiveTexture(GL_TEXTURE0);
  }
}

void gl_renderer::draw_2d() {
  for (auto &i : m_drawer_2ds) {
    glBindVertexArray(m_VertexArrays[i.drawable_object->vertexIndex].vao);
    glBindTexture(GL_TEXTURE_2D,
                  mTextureIDs[i.drawable_object->texture_handle.handle]);
    if (i.drawable_object->shader_data.vertex_shader() == "default" &&
        i.drawable_object->shader_data.fragment_shader() == "default") {
      mAlphaShader.active(0);
      i.ubo.bind(mAlphaShader.program(), 0);
      i.ubo.update(0, sizeof(drawable::parameter), &i.drawable_object->param,
                   0);
      if (i.drawable_object->shader_data.get_parameter_size() > 0) {
        i.ubo.update(0, i.drawable_object->shader_data.get_parameter_size(),
                     i.drawable_object->shader_data.get_parameter().get(),
                     sizeof(gl_shader_parameter));
      }

    } else {
      for (auto &j : m_user_pipelines) {
        if (j.first == i.drawable_object->shader_data) {
          j.second.active(0);
          i.ubo.bind(j.second.program(), 0);
          i.ubo.update(0, sizeof(drawable::parameter),
                       &i.drawable_object->param, 0);
          if (i.drawable_object->shader_data.get_parameter_size() > 0) {
            i.ubo.update(0, i.drawable_object->shader_data.get_parameter_size(),
                         i.drawable_object->shader_data.get_parameter().get(),
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
    mAlphaInstanceShader.active(0);
    i.ubo.bind(mAlphaInstanceShader.program(), 0);
    i.ubo.update(0, sizeof(drawable::parameter), &i.ins.object->param, 0);
    if (i.ins.object->shader_data.get_parameter_size() > 0) {
      i.ubo.update(0, i.ins.object->shader_data.get_parameter_size(),
                   i.ins.object->shader_data.get_parameter().get(),
                   sizeof(gl_shader_parameter));
    }

    auto &va = m_VertexArrays[i.ins.object->vertexIndex];
    glBindVertexArray(i.vao);

    glBindTexture(GL_TEXTURE_2D,
                  mTextureIDs[i.ins.object->texture_handle.handle]);
    enable_vertex_attrib_array();
    glDrawElementsInstanced(GL_TRIANGLES, va.indices.size(), GL_UNSIGNED_INT,
                            nullptr, i.ins.data.size());
  }
}

void gl_renderer::add_vertex_array(const vertex_array &vArray,
                                   std::string_view name) {
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

void gl_renderer::draw2d(std::shared_ptr<class drawable> sprite) {
  create_texture(sprite->texture_handle);
  auto iter = m_drawer_2ds.begin();
  for (; iter != m_drawer_2ds.end(); ++iter) {
    if (sprite->drawOrder < (*iter).drawable_object->drawOrder) {
      break;
    }
  }
  gl_uniform_buffer ubo;
  ubo.create(
      0, sizeof(drawable::parameter) + sprite->shader_data.get_parameter_size(),
      &sprite->param);
  if (sprite->shader_data.get_parameter_size() > 0) {
    ubo.update(0, sprite->shader_data.get_parameter_size(),
               sprite->shader_data.get_parameter().get(),
               sizeof(gl_shader_parameter));
  }
  m_drawer_2ds.insert(iter, {sprite, ubo});
}

void gl_renderer::draw3d(std::shared_ptr<class drawable> sprite) {
  create_texture(sprite->texture_handle);
  auto iter = m_drawer_3ds.begin();
  for (; iter != m_drawer_3ds.end(); ++iter) {
    if (sprite->drawOrder < (*iter).drawable_object->drawOrder) {
      break;
    }
  }
  gl_uniform_buffer ubo;
  ubo.create(
      0, sizeof(drawable::parameter) + sprite->shader_data.get_parameter_size(),
      &sprite->param);
  ubo.update(0, sprite->shader_data.get_parameter_size(),
             sprite->shader_data.get_parameter().get(),
             sizeof(drawable::parameter));
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
  create_texture(_instancing.object->texture_handle);
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
                 sizeof(drawable::parameter) +
                     _instancing.object->shader_data.get_parameter_size(),
                 &_instancing.object->param);
  ogl.ubo.update(0, _instancing.object->shader_data.get_parameter_size(),
                 _instancing.object->shader_data.get_parameter().get(),
                 sizeof(drawable::parameter));

  if (_instancing.type == object_type::_2D) {
    m_instancing_2d.emplace_back(ogl);

  } else {
    m_instancing_3d.emplace_back(ogl);
  }
}

void gl_renderer::prepare() {
  if (!load_shader()) {
    std::cout << "failed to loads shader" << std::endl;
  }
  prepare_depth_texture();
  prepare_render_texture();
}

void gl_renderer::prepare_render_texture() {

  /*
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  glGenTextures(1, &rendertexture);
  glBindTexture(GL_TEXTURE_2D, rendertexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, get_window().size.x,
               get_window().size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glGenRenderbuffers(1, &depthbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                        get_window().size.x, get_window().size.y);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, depthbuffer);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendertexture, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  */
}
void gl_renderer::prepare_depth_texture() {

  /*
  glGenFramebuffers(1, &shadowframebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, shadowframebuffer);
  glGenTextures(1, &shadowdepthtexture);
  glBindTexture(GL_TEXTURE_2D, shadowdepthtexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                  GL_COMPARE_R_TO_TEXTURE);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowdepthtexture,
                       0);
  glDrawBuffer(GL_NONE);
  */
}

void gl_renderer::create_texture(texture handle) {
  ::SDL_Surface &surf = texture_system::get(handle.handle);
  if (mTextureIDs.contains(handle.handle)) {
    if (*handle.is_need_update) {
      glDeleteTextures(1, &mTextureIDs[handle.handle]);
      mTextureIDs.erase(handle.handle);
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
  // Use linear filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  SDL_FreeFormat(formatbuf);
  SDL_FreeSurface(imagedata);
  mTextureIDs[handle.handle] = textureId;
}

bool gl_renderer::load_shader() {
  gl_shader_parameter param{};
  /*
  if (!m_depth_texture_shader.load("depth.vert", "depth.frag")) {
    return false;
  }
  m_depth_texture_shader.create_ubo(0, sizeof(gl_shader_parameter), &param);
  if (!m_depth_texture_instanced_shader.load("depth_instanced.vert",
                                             "depth.frag")) {
    return false;
  }
  m_depth_texture_instanced_shader.create_ubo(0, sizeof(gl_shader_parameter),
                                              &param);
  if (!m_render_texture_shader.load("render_texture.vert",
                                    "render_texture.frag")) {
    return false;
  }
  m_render_texture_shader.create_ubo(0, sizeof(gl_shader_parameter), &param);
  */
  if (!mSpriteShader.load("shader.vert", "shader.frag")) {
    return false;
  }
  if (!mAlphaShader.load("shader.vert", "alpha.frag")) {
    return false;
  }
  if (!mSpriteInstanceShader.load("shader_instance.vert", "shader.frag")) {
    return false;
  }
  if (!mAlphaInstanceShader.load("shader_instance.vert", "alpha.frag")) {
    return false;
  }
  return true;
}

} // namespace sinen