#include "../../manager/get_system.hpp"
#include "../../render/render_system.hpp"
#include "../../texture/texture_system.hpp"
#include "../../window/window_system.hpp"
#include <SDL.h>
#include <SDL_image.h>

#if defined(EMSCRIPTEN) || defined(MOBILE)
#include <GLES3/gl3.h>
#include <SDL_opengles2.h>
#endif

#ifndef MOBILE
#include <GL/glew.h>
#endif

#include <cstdint>
#include <vector>

#include "../../texture/texture_system.hpp"
#include "gl_renderer.hpp"
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

namespace nen {
gl_renderer::gl_renderer() {}
gl_renderer::~gl_renderer() = default;

void gl_renderer::initialize() {
  auto &w = get_window();
  mContext = SDL_GL_CreateContext((SDL_Window *)w.GetSDLWindow());
  SDL_GL_MakeCurrent((SDL_Window *)w.GetSDLWindow(), mContext);
  if (!SDL_GL_SetSwapInterval(1)) {
    SDL_GL_SetSwapInterval(0);
  }
  prev_window_x = w.Size().x;
  prev_window_y = w.Size().y;
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
  ImGui_ImplSDL2_InitForOpenGL((SDL_Window *)get_window().GetSDLWindow(),
                               mContext);
#if defined EMSCRIPTEN || defined MOBILE
  ImGui_ImplOpenGL3_Init("#version 300 es");
#else
  ImGui_ImplOpenGL3_Init("#version 330 core");
#endif
}

void gl_renderer::shutdown() {}

void gl_renderer::render() {
  auto &w = get_window();
  if (w.Size().x != prev_window_x || w.Size().y != prev_window_y) {
    glViewport(0, 0, w.Size().x, w.Size().y);
    prev_window_x = w.Size().x;
    prev_window_y = w.Size().y;
  }
  auto color = get_renderer().get_clear_color();
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glViewport(0, 0, get_window().size.x, -get_window().size.y);
  glClearColor(color.r, color.g, color.b, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
  glEnable(GL_BLEND);
  glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
  disable_vertex_attrib_array();
  draw_2d();
  enable_vertex_attrib_array();
  draw_instancing_2d();
  glDisable(GL_BLEND);
  glFlush();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(m_VertexArrays["SPRITE"].vao);
  glBindTexture(GL_TEXTURE_2D, rendertexture);
  m_render_texture_shader.active(0);
  shader_parameter param;
  m_render_texture_shader.update_ubo(0, sizeof(shader_parameter), &param);
  disable_vertex_attrib_array();
  glDrawElements(GL_TRIANGLES, m_VertexArrays["SPRITE"].indices.size(),
                 GL_UNSIGNED_INT, nullptr);
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame((SDL_Window *)w.GetSDLWindow());
  ImGui::NewFrame();
  if (get_renderer().is_show_imgui()) {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(
        ImVec2(get_window().Size().x, get_window().Size().y), ImGuiCond_Always);

    // Draw ImGUI widgets.
    ImGui::Begin(" ", nullptr,
                 ImGuiWindowFlags_HorizontalScrollbar |
                     ImGuiWindowFlags_MenuBar);
    for (auto &i : get_renderer().get_imgui_function()) {
      i();
    }
    ImGui::End();
  }

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  m_drawer_3ds.clear();
  m_drawer_2ds.clear();
  SDL_GL_SwapWindow((SDL_Window *)w.GetSDLWindow());
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
  create_texture(get_renderer().get_skybox_texture());
  auto &va = m_VertexArrays["BOX"];
  glBindVertexArray(va.vao);
  mAlphaShader.active(0);
  shader_parameter param;
  matrix4 w = matrix4::identity;
  w[0][0] = 5;
  w[1][1] = 5;
  w[2][2] = 5;
  param.proj = get_camera().projection;
  param.view = matrix4::lookat(vector3(0, 0, 0),
                               get_camera().target - get_camera().position,
                               get_camera().up);
  mAlphaShader.active(0);
  mAlphaShader.update_ubo(0, sizeof(shader_parameter), &param);
  glBindTexture(GL_TEXTURE_2D,
                mTextureIDs[get_renderer().get_skybox_texture().handle]);
  disable_vertex_attrib_array();
  glDrawElements(GL_TRIANGLES, va.indices.size(), GL_UNSIGNED_INT, nullptr);
}

void gl_renderer::draw_3d() {
  for (auto &i : m_drawer_3ds) {
    auto &va = m_VertexArrays[i->vertexIndex];
    glBindVertexArray(va.vao);
    if (i->shader_data.vertName == "default" &&
        i->shader_data.fragName == "default") {
      mSpriteShader.active(0);
      mSpriteShader.update_ubo(0, sizeof(shader_parameter), &i->param);
    } else {
      for (auto &j : m_user_pipelines) {
        if (j.first == i->shader_data) {
          j.second.active(0);
          j.second.update_ubo(0, sizeof(shader_parameter), &i->param);
        }
      }
    }
    glBindTexture(GL_TEXTURE_2D, mTextureIDs[i->texture_handle.handle]);
    disable_vertex_attrib_array();
    glDrawElements(GL_TRIANGLES, m_VertexArrays[i->vertexIndex].indices.size(),
                   GL_UNSIGNED_INT, nullptr);
  }
}
void gl_renderer::draw_instancing_3d() {
  for (auto &i : m_instancing_3d) {
    mSpriteInstanceShader.active(0);
    mSpriteInstanceShader.update_ubo(0, sizeof(shader_parameter),
                                     &i.ins.object->param);

    auto &va = m_VertexArrays[i.ins.object->vertexIndex];
    glBindVertexArray(i.vao);

    glBindTexture(GL_TEXTURE_2D,
                  mTextureIDs[i.ins.object->texture_handle.handle]);
    enable_vertex_attrib_array();
    glDrawElementsInstanced(GL_TRIANGLES, va.indices.size(), GL_UNSIGNED_INT,
                            nullptr, i.ins.data.size());
  }
}

void gl_renderer::draw_2d() {
  for (auto &i : m_drawer_2ds) {
    glBindVertexArray(m_VertexArrays[i->vertexIndex].vao);
    glBindTexture(GL_TEXTURE_2D, mTextureIDs[i->texture_handle.handle]);
    if (i->shader_data.vertName == "default" &&
        i->shader_data.fragName == "default") {
      mAlphaShader.active(0);
      mAlphaShader.update_ubo(0, sizeof(shader_parameter), &i->param);
    } else {
      for (auto &j : m_user_pipelines) {
        if (j.first == i->shader_data) {
          j.second.active(0);
          j.second.update_ubo(0, sizeof(shader_parameter), &i->param);
        }
      }
    }
    disable_vertex_attrib_array();
    glDrawElements(GL_TRIANGLES, m_VertexArrays[i->vertexIndex].indices.size(),
                   GL_UNSIGNED_INT, nullptr);
  }
}

void gl_renderer::draw_instancing_2d() {
  for (auto &i : m_instancing_2d) {
    mAlphaInstanceShader.active(0);
    mAlphaInstanceShader.update_ubo(0, sizeof(shader_parameter),
                                    &i.ins.object->param);

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

void gl_renderer::draw2d(std::shared_ptr<class draw_object> sprite) {
  create_texture(sprite->texture_handle);
  add_sprite2d(sprite);
}

void gl_renderer::draw3d(std::shared_ptr<class draw_object> sprite) {
  create_texture(sprite->texture_handle);
  add_sprite3d(sprite);
}

void gl_renderer::load_shader(const shader &shaderInfo) {
  gl_shader pipeline;
  pipeline.load(shaderInfo.vertName, shaderInfo.fragName);
  shader_parameter param;
  pipeline.create_ubo(0, sizeof(shader_parameter), &param);
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
  prepare_render_texture();
}

void gl_renderer::prepare_render_texture() {

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
}

void gl_renderer::create_texture(texture handle) {
  ::SDL_Surface &surf = get_texture().get(handle.handle);
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
  shader_parameter param{};
  if (!m_render_texture_shader.load("render_texture.vert",
                                    "render_texture.frag")) {
    return false;
  }
  m_render_texture_shader.create_ubo(0, sizeof(shader_parameter), &param);
  if (!mSpriteShader.load("shader.vert", "shader.frag")) {
    return false;
  }
  mSpriteShader.create_ubo(0, sizeof(shader_parameter), &param);
  if (!mAlphaShader.load("shader.vert", "alpha.frag")) {
    return false;
  }
  mAlphaShader.create_ubo(0, sizeof(shader_parameter), &param);
  if (!mSpriteInstanceShader.load("shader_instance.vert", "shader.frag")) {
    return false;
  }
  mSpriteInstanceShader.create_ubo(0, sizeof(shader_parameter), &param);
  if (!mAlphaInstanceShader.load("shader_instance.vert", "alpha.frag")) {
    return false;
  }
  mAlphaInstanceShader.create_ubo(0, sizeof(shader_parameter), &param);
  return true;
}

} // namespace nen