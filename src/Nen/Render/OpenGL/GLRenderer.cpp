#include <Nen.hpp>
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include "SDL_stdinc.h"
#include <cstdint>
#include <vector>

#include "../../Texture/SurfaceHandle.hpp"
#include "GLRenderer.h"
#include <GL/glew.h>
#include <Nen.hpp>
#include <SDL.h>
#include <SDL_image.h>
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
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    logger::Error("GLEW Init error.");
  }
  glGetError();

  prepare();
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplSDL2_InitForOpenGL((SDL_Window *)window->GetSDLWindow(), mContext);
  ImGui_ImplOpenGL3_Init("#version 330 core");
}
void GLRenderer::SetRenderer(renderer *renderer) { mRenderer = renderer; }

void GLRenderer::Render() {
  auto color = mRenderer->GetClearColor();
  glClearColor(color.r, color.g, color.b, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  std::string vertexID;
  bool lastFrameChanged = false;
  for (auto &i : mSprite3Ds) {
    i->param.view = mRenderer->GetViewMatrix();
    i->param.proj = mRenderer->GetProjectionMatrix();
    if (vertexID != i->vertexIndex) {
      glBindVertexArray(m_VertexArrays[i->vertexIndex].vao);
      vertexID = i->vertexIndex;
    }
    if (i->shader_data.vertName == "default" &&
        i->shader_data.fragName == "default") {
      mSpriteShader->SetActive(0);
      mSpriteShader->UpdateUBO(0, sizeof(shader_parameter), &i->param);
    } else {
      for (auto &j : userPipelines) {
        if (j.first == i->shader_data) {
          j.second.SetActive(0);
          j.second.UpdateUBO(0, sizeof(shader_parameter), &i->param);
        }
      }
    }

    glBindTexture(GL_TEXTURE_2D, mTextureIDs[i->textureIndex]);
    glDrawElements(GL_TRIANGLES, m_VertexArrays[i->vertexIndex].indexCount,
                   GL_UNSIGNED_INT, nullptr);
  }
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
  for (auto &i : mSprite2Ds) {
    if (vertexID != i->vertexIndex) {
      glBindVertexArray(m_VertexArrays[i->vertexIndex].vao);
      vertexID = i->vertexIndex;
    }
    if (i->shader_data.vertName == "default" &&
        i->shader_data.fragName == "default") {
      mAlphaShader->SetActive(0);
      mAlphaShader->UpdateUBO(0, sizeof(shader_parameter), &i->param);
    } else {
      for (auto &j : userPipelines) {
        if (j.first == i->shader_data) {
          j.second.SetActive(0);
          j.second.UpdateUBO(0, sizeof(shader_parameter), &i->param);
        }
      }
    }
    glBindTexture(GL_TEXTURE_2D, mTextureIDs[i->textureIndex]);
    glDrawElements(GL_TRIANGLES, m_VertexArrays[i->vertexIndex].indexCount,
                   GL_UNSIGNED_INT, nullptr);
  }
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
               GL_STATIC_DRAW);

  // VBOをVAOに登録
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float),
                        reinterpret_cast<void *>(sizeof(float) * 3));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, 8 * sizeof(float),
                        reinterpret_cast<void *>(sizeof(float) * 6));
  // IBOを作成
  glGenBuffers(1, &vArrayGL.ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vArrayGL.ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               vArrayGL.indices.size() * sizeof(uint32_t),
               vArrayGL.indices.data(), GL_STATIC_DRAW);
  // 登録
  m_VertexArrays.emplace(name.data(), vArrayGL);
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
  registerTexture(texture, texture_type::Image2D);
  pushSprite2d(sprite);
}
void GLRenderer::RemoveDrawObject2D(std::shared_ptr<class draw_object> sprite) {
  eraseSprite2d(sprite);
}

void GLRenderer::AddDrawObject3D(std::shared_ptr<class draw_object> sprite,
                                 std::shared_ptr<texture> texture) {
  registerTexture(texture, texture_type::Image3D);
  pushSprite3d(sprite);
}
void GLRenderer::RemoveDrawObject3D(std::shared_ptr<class draw_object> sprite) {
  eraseSprite3d(sprite);
}

void GLRenderer::LoadEffect(std::shared_ptr<effect> effect) {}

void GLRenderer::LoadShader(const shader &shaderInfo) {
  ShaderGL pipeline;
  pipeline.Load(std::string("Assets/Shader/GL/") + shaderInfo.vertName +
                    std::string(".vert"),
                std::string("Assets/Shader/GL/") + shaderInfo.fragName +
                    std::string(".frag"));
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

void GLRenderer::prepare() {
  if (!loadShader()) {
    std::cout << "failed to loads shader" << std::endl;
  }
  createSpriteVerts();
  createBoxVerts();
}

void GLRenderer::registerTexture(std::shared_ptr<texture> texture,
                                 const texture_type &type) {
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
  mSpriteShader = new ShaderGL();
  if (!mSpriteShader->Load("Assets/Shader/GL/shader.vert",
                           "Assets/Shader/GL/shader.frag")) {
    return false;
  }

  shader_parameter param{};
  mSpriteShader->CreateUBO(0, sizeof(shader_parameter), &param);
  mAlphaShader = new ShaderGL();
  if (!mAlphaShader->Load("Assets/Shader/GL/shader.vert",
                          "Assets/Shader/GL/alpha.frag")) {
    return false;
  }
  mAlphaShader->CreateUBO(0, sizeof(shader_parameter), &param);
  return true;
}

void GLRenderer::createSpriteVerts() {
  const float value = 1.f;
  const vector2 lb(0.0f, 0.0f);
  const vector2 lt(0.f, 1.f);
  const vector2 rb(1.0f, 0.0f);
  const vector2 rt(1.0f, 1.0f);
  vector3 norm{1, 1, 1};
  vertex_array vArray;
  vArray.vertices.push_back({vector3(-value, value, 0.5f), norm, lb});
  vArray.vertices.push_back({vector3(-value, -value, 0.5f), norm, lt});
  vArray.vertices.push_back({vector3(value, value, 0.5f), norm, rb});
  vArray.vertices.push_back({vector3(value, -value, 0.5f), norm, rt});
  uint32_t indices[] = {
      0, 2, 1, 1, 2, 3 // front
  };
  vArray.indexCount = 6;
  vArray.PushIndices(indices, vArray.indexCount);

  AddVertexArray(vArray, "SPRITE");
}

void GLRenderer::createBoxVerts() {
  const float value = 1.f;
  const vector2 lb(0.0f, 0.0f);
  const vector2 lt(0.f, 1.f);
  const vector2 rb(1.0f, 0.0f);
  const vector2 rt(1.0f, 1.0f);
  vector3 norm{1, 1, 1};
  const vector3 red{1.0f, 0.0f, 0.0f};
  const vector3 green{0.0f, 1.0f, 0.0f};
  const vector3 blue{0.0f, 0.0f, 1.0f};
  const vector3 white{1.0f, 1, 1};
  const vector3 black{0.0f, 0, 0};
  const vector3 yellow{1.0f, 1.0f, 0.0f};
  const vector3 magenta{1.0f, 0.0f, 1.0f};
  const vector3 cyan{0.0f, 1.0f, 1.0f};

  vertex_array vArray;
  vArray.vertices.push_back({vector3(-value, value, value), yellow, lb});
  vArray.vertices.push_back({vector3(-value, -value, value), red, lt});
  vArray.vertices.push_back({vector3(value, value, value), white, rb});
  vArray.vertices.push_back({vector3(value, -value, value), magenta, rt});

  vArray.vertices.push_back({vector3(value, value, value), white, lb});
  vArray.vertices.push_back({vector3(value, -value, value), magenta, lt});
  vArray.vertices.push_back({vector3(value, value, -value), cyan, rb});
  vArray.vertices.push_back({vector3(value, -value, -value), blue, rt});

  vArray.vertices.push_back({vector3(-value, value, -value), green, lb});
  vArray.vertices.push_back({vector3(-value, -value, -value), black, lt});
  vArray.vertices.push_back({vector3(-value, value, value), yellow, rb});
  vArray.vertices.push_back({vector3(-value, -value, value), red, rt});

  vArray.vertices.push_back({vector3(value, value, -value), cyan, lb});
  vArray.vertices.push_back({vector3(value, -value, -value), blue, lt});
  vArray.vertices.push_back({vector3(-value, value, -value), green, rb});
  vArray.vertices.push_back({vector3(-value, -value, -value), black, rt});

  vArray.vertices.push_back({vector3(-value, value, -value), green, lb});
  vArray.vertices.push_back({vector3(-value, value, value), yellow, lt});
  vArray.vertices.push_back({vector3(value, value, -value), cyan, rb});
  vArray.vertices.push_back({vector3(value, value, value), white, rt});

  vArray.vertices.push_back({vector3(-value, -value, value), red, lb});
  vArray.vertices.push_back({vector3(-value, -value, -value), black, lt});
  vArray.vertices.push_back({vector3(value, -value, value), magenta, rb});
  vArray.vertices.push_back({vector3(value, -value, -value), blue, rt});

  uint32_t indices[] = {
      0,  2,  1,  1,  2,  3,  // front
      4,  6,  5,  5,  6,  7,  // right
      8,  10, 9,  9,  10, 11, // left

      12, 14, 13, 13, 14, 15, // back
      16, 18, 17, 17, 18, 19, // top
      20, 22, 21, 21, 22, 23, // bottom
  };
  vArray.indexCount = sizeof(indices) / sizeof(uint32_t);

  vArray.PushIndices(indices, vArray.indexCount);
  AddVertexArray(vArray, "BOX");
}
} // namespace nen::gl
#endif
