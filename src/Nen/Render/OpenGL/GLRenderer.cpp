
#include <Nen.hpp>

#include <SDL.h>
#include <SDL_image.h>

#if defined(EMSCRIPTEN) || defined(MOBILE)
#include <GLES3/gl3.h>
#include <SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif

#include "SDL_stdinc.h"
#include <cstdint>
#include <vector>

#include "../../Texture/SurfaceHandle.hpp"
#include "GLRenderer.h"
#include <fstream>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <iostream>
#include <sstream>

namespace nen::gl {
GLRenderer::GLRenderer() {}

void GLRenderer::Initialize(std::shared_ptr<window> window) {
  mWindow = window;
  mContext = SDL_GL_CreateContext((SDL_Window *)mWindow->GetSDLWindow());
  SDL_GL_MakeCurrent((SDL_Window *)mWindow->GetSDLWindow(), mContext);
  prev_window_x = mWindow->Size().x;
  prev_window_y = mWindow->Size().y;
#if !defined EMSCRIPTEN && !defined MOBILE
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    logger::Error("GLEW Init error.");
  }
  glGetError();
#endif

  prepare();
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplSDL2_InitForOpenGL((SDL_Window *)window->GetSDLWindow(), mContext);
#if defined EMSCRIPTEN || defined MOBILE
  ImGui_ImplOpenGL3_Init("#version 300 es");
#else
  ImGui_ImplOpenGL3_Init("#version 330 core");
#endif
}
void GLRenderer::SetRenderer(renderer *renderer) { mRenderer = renderer; }

void GLRenderer::Render() {
  if (mWindow->Size().x != prev_window_x ||
      mWindow->Size().y != prev_window_y) {
    glViewport(0, 0, mWindow->Size().x, mWindow->Size().y);
    prev_window_x = mWindow->Size().x;
    prev_window_y = mWindow->Size().y;
  }
  auto color = mRenderer->GetClearColor();
  glClearColor(color.r, color.g, color.b, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  draw_3d();
  draw_instancing_3d();
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
  draw_2d();
  draw_instancing_2d();
  glDisable(GL_BLEND);
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame((SDL_Window *)mWindow->GetSDLWindow());
  ImGui::NewFrame();
  if (mRenderer->isShowImGui()) {

    // Draw ImGUI widgets.
    ImGui::Begin("Engine Info");
    ImGui::Text("%.1f fps", ImGui::GetIO().Framerate);
    if (ImGui::Button("toggleAPI")) {
      std::ofstream ofs("./api");
      ofs << "Vulkan";
    }
    ImGui::End();
  }

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  SDL_GL_SwapWindow((SDL_Window *)mWindow->GetSDLWindow());
}

void GLRenderer::draw_3d() {
  for (auto &i : mSprite3Ds) {
    auto va = m_VertexArrays[i->vertexIndex];
    glBindVertexArray(va.vao);
    if (i->shader_data.vertName == "default" &&
        i->shader_data.fragName == "default") {
      mSpriteShader.SetActive(0);
      mSpriteShader.UpdateUBO(0, sizeof(shader_parameter), &i->param);
    } else {
      for (auto &j : userPipelines) {
        if (j.first == i->shader_data) {
          j.second.SetActive(0);
          j.second.UpdateUBO(0, sizeof(shader_parameter), &i->param);
        }
      }
    }
    glBindTexture(GL_TEXTURE_2D, mTextureIDs[i->textureIndex]);
    glDrawElements(GL_TRIANGLES, m_VertexArrays[i->vertexIndex].indices.size(),
                   GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
  }
}
void GLRenderer::draw_instancing_3d() {

  for (auto &i : m_instancing_3d) {
    mSpriteInstanceShader.SetActive(0);
    mSpriteInstanceShader.UpdateUBO(0, sizeof(shader_parameter),
                                    &i.ins.object->param);

    auto va = m_VertexArrays[i.ins.object->vertexIndex];
    glBindVertexArray(va.vao);

    glBindTexture(GL_TEXTURE_2D, mTextureIDs[i.ins.object->textureIndex]);
    glDrawElementsInstanced(
        GL_TRIANGLES, m_VertexArrays[i.ins.object->vertexIndex].indices.size(),
        GL_UNSIGNED_INT, nullptr, i.ins.data.size());
    glBindVertexArray(0);
  }
}

void GLRenderer::draw_2d() {
  for (auto &i : mSprite2Ds) {
    glBindVertexArray(m_VertexArrays[i->vertexIndex].vao);
    if (i->shader_data.vertName == "default" &&
        i->shader_data.fragName == "default") {
      mAlphaShader.SetActive(0);
      mAlphaShader.UpdateUBO(0, sizeof(shader_parameter), &i->param);
    } else {
      for (auto &j : userPipelines) {
        if (j.first == i->shader_data) {
          j.second.SetActive(0);
          j.second.UpdateUBO(0, sizeof(shader_parameter), &i->param);
        }
      }
    }
    glBindTexture(GL_TEXTURE_2D, mTextureIDs[i->textureIndex]);
    glDrawElements(GL_TRIANGLES, m_VertexArrays[i->vertexIndex].indices.size(),
                   GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
  }
}

void GLRenderer::draw_instancing_2d() {
  for (auto &i : m_instancing_2d) {
    mAlphaInstanceShader.SetActive(0);
    mAlphaInstanceShader.UpdateUBO(0, sizeof(shader_parameter),
                                   &i.ins.object->param);

    auto va = m_VertexArrays[i.ins.object->vertexIndex];
    glBindVertexArray(va.vao);

    glBindTexture(GL_TEXTURE_2D, mTextureIDs[i.ins.object->textureIndex]);
    glDrawElementsInstanced(
        GL_TRIANGLES, m_VertexArrays[i.ins.object->vertexIndex].indices.size(),
        GL_UNSIGNED_INT, nullptr, i.ins.data.size());
    glBindVertexArray(0);
  }
}

void GLRenderer::AddVertexArray(const vertex_array &vArray,
                                std::string_view name) {
  gl::VertexArrayForGL vArrayGL;
  vArrayGL.indexCount = vArray.indexCount;
  vArrayGL.indices = vArray.indices;
  vArrayGL.vertices = vArray.vertices;
  vArrayGL.materialName = vArray.materialName;

  // VAOを作成
  glGenVertexArrays(1, &vArrayGL.vao);
  glBindVertexArray(vArrayGL.vao);

  // VBOを作成
  glGenBuffers(1, &vArrayGL.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vArrayGL.vbo);
  auto vArraySize = vArrayGL.vertices.size() * sizeof(vertex);
  glBufferData(GL_ARRAY_BUFFER, vArraySize, vArrayGL.vertices.data(),
               GL_DYNAMIC_DRAW);
  size_t size = sizeof(vertex);
  // VBOをVAOに登録
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, size, 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, size,
                        reinterpret_cast<void *>(sizeof(float) * 3));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, size,
                        reinterpret_cast<void *>(sizeof(float) * 6));
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, size,
                        reinterpret_cast<void *>(sizeof(float) * 8));
  // IBOを作成
  glGenBuffers(1, &vArrayGL.ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vArrayGL.ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               vArrayGL.indices.size() * sizeof(uint32_t),
               vArrayGL.indices.data(), GL_STATIC_DRAW);
  m_VertexArrays.emplace(std::string(name), vArrayGL);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void GLRenderer::UpdateVertexArray(const vertex_array &vArray,
                                   std::string_view name) {
  gl::VertexArrayForGL vArrayGL;
  vArrayGL.indexCount = vArray.indexCount;
  vArrayGL.indices = vArray.indices;
  vArrayGL.vertices = vArray.vertices;
  vArrayGL.materialName = vArray.materialName;
  // vboを更新
  glBindBuffer(GL_ARRAY_BUFFER, m_VertexArrays[name.data()].vbo);
  auto vArraySize = vArrayGL.vertices.size() * sizeof(vertex);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vArraySize, vArrayGL.vertices.data());
  // iboを更新
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VertexArrays[name.data()].ibo);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
                  vArrayGL.indices.size() * sizeof(uint32_t),
                  vArrayGL.indices.data());
}

void GLRenderer::AddDrawObject2D(std::shared_ptr<class draw_object> sprite,
                                 std::shared_ptr<texture> texture) {
  registerTexture(texture);
  pushSprite2d(sprite);
}
void GLRenderer::RemoveDrawObject2D(std::shared_ptr<class draw_object> sprite) {
  eraseSprite2d(sprite);
}

void GLRenderer::AddDrawObject3D(std::shared_ptr<class draw_object> sprite,
                                 std::shared_ptr<texture> texture) {
  registerTexture(texture);
  pushSprite3d(sprite);
}
void GLRenderer::RemoveDrawObject3D(std::shared_ptr<class draw_object> sprite) {
  eraseSprite3d(sprite);
}

void GLRenderer::LoadShader(const shader &shaderInfo) {
  ShaderGL pipeline;
  pipeline.Load(shaderInfo.vertName, shaderInfo.fragName);
  shader_parameter param;
  pipeline.CreateUBO(0, sizeof(shader_parameter), &param);
  userPipelines.emplace_back(std::pair<shader, ShaderGL>{shaderInfo, pipeline});
}

void GLRenderer::UnloadShader(const shader &shaderInfo) {
  std::erase_if(userPipelines, [&](auto &x) {
    if (x.first == shaderInfo) {
      x.second.Unload();
      return true;
    };
    return false;
  });
}

void GLRenderer::add_instancing(instancing &_instancing) {
  registerTexture(_instancing._texture);
  auto va = m_VertexArrays[_instancing.object->vertexIndex];
  glBindVertexArray(va.vao);
  uint32_t vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, _instancing.size, _instancing.data.data(),
               GL_DYNAMIC_DRAW);
  auto size = sizeof(instance_data);
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, size,
                        reinterpret_cast<void *>(sizeof(float) * 0));
  glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, size,
                        reinterpret_cast<void *>(sizeof(float) * 4));
  glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, size,
                        reinterpret_cast<void *>(sizeof(float) * 8));
  glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, size,
                        reinterpret_cast<void *>(sizeof(float) * 12));
  glEnableVertexAttribArray(4);
  glEnableVertexAttribArray(5);
  glEnableVertexAttribArray(6);
  glEnableVertexAttribArray(7);
  glVertexAttribDivisor(4, 1);
  glVertexAttribDivisor(5, 1);
  glVertexAttribDivisor(6, 1);
  glVertexAttribDivisor(7, 1);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  auto ogl = ogl_instancing(_instancing);
  ogl.vbo = vbo;

  if (_instancing.type == object_type::_2D) {
    m_instancing_2d.emplace_back(ogl);

  } else {
    m_instancing_3d.emplace_back(ogl);
  }
}
void GLRenderer::remove_instancing(instancing &_instancing) {}

void GLRenderer::prepare() {
  if (!loadShader()) {
    std::cout << "failed to loads shader" << std::endl;
  }
}

void GLRenderer::registerTexture(std::shared_ptr<texture> texture) {
  ::SDL_Surface surf = surface_handler::Load(texture->id);
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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  mTextureIDs.emplace(texture->id, textureId);
}

bool GLRenderer::loadShader() {
  shader_parameter param{};
  if (!mSpriteShader.Load("shader.vert", "shader.frag")) {
    return false;
  }
  mSpriteShader.CreateUBO(0, sizeof(shader_parameter), &param);
  if (!mAlphaShader.Load("shader.vert", "alpha.frag")) {
    return false;
  }
  mAlphaShader.CreateUBO(0, sizeof(shader_parameter), &param);
  if (!mSpriteInstanceShader.Load("shader_instance.vert", "shader.frag")) {
    return false;
  }
  mSpriteInstanceShader.CreateUBO(0, sizeof(shader_parameter), &param);
  if (!mAlphaInstanceShader.Load("shader_instance.vert", "alpha.frag")) {
    return false;
  }
  mAlphaInstanceShader.CreateUBO(0, sizeof(shader_parameter), &param);
  return true;
}

} // namespace nen::gl