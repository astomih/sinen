#pragma once
#include "manager/manager.hpp"
#include <cstdint>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#if defined(EMSCRIPTEN) || defined(MOBILE)
#include <GLES3/gl3.h>
#endif
#include "gl_shader.hpp"
#include <SDL.h>
#include <draw_object/draw_object.hpp>
#include <memory>
#include <render/renderer.hpp>
#include <string>
#include <texture/texture.hpp>
#include <vertex/vertex_array.hpp>

namespace nen {
struct gl_vertex_array : public nen::vertex_array {
  /**
   * @brief Vertex Array Object
   *
   */
  uint32_t vao;

  /**
   * @brief Vertex Buffer Object
   *
   */
  uint32_t vbo;

  /**
   * @brief Index Buffer Object
   *
   */
  uint32_t ibo;
};

class gl_instancing {
public:
  gl_instancing(instancing ins) : ins(ins) {}
  instancing ins;
  uint32_t vbo;
};

class gl_renderer {
public:
  gl_renderer();
  ~gl_renderer();

  void initialize();
  void shutdown();
  void render();
  void add_vertex_array(const vertex_array &vArray, std::string_view name);
  void update_vertex_array(const vertex_array &vArray, std::string_view name);
  void draw2d(std::shared_ptr<class draw_object> sprite);
  void draw3d(std::shared_ptr<class draw_object> sprite);

  void load_shader(const shader &shaderInfo);
  void unload_shader(const shader &shaderInfo);

  void add_instancing(const instancing &_instancing);

  void prepare();
  void cleanup() {}
  void registerTexture(handle_t handle);
  void add_sprite2d(std::shared_ptr<draw_object> sprite2d) {
    auto iter = mSprite2Ds.begin();
    for (; iter != mSprite2Ds.end(); ++iter) {
      if (sprite2d->drawOrder < (*iter)->drawOrder) {
        break;
      }
    }
    mSprite2Ds.insert(iter, sprite2d);
  }
  void remove_sprite2d(std::shared_ptr<draw_object> sprite2d) {
    auto itr = std::find(mSprite2Ds.begin(), mSprite2Ds.end(), sprite2d);
    if (itr != mSprite2Ds.end()) {
      mSprite2Ds.erase(itr);
    }
  }

  void add_sprite3d(std::shared_ptr<draw_object> sprite3d) {
    auto iter = mSprite3Ds.begin();
    for (; iter != mSprite3Ds.end(); ++iter) {
      if (sprite3d->drawOrder < (*iter)->drawOrder) {
        break;
      }
    }
    mSprite3Ds.insert(iter, sprite3d);
  }
  void remove_sprite3d(std::shared_ptr<draw_object> sprite3d) {
    auto itr = std::find(mSprite3Ds.begin(), mSprite3Ds.end(), sprite3d);
    if (itr != mSprite3Ds.end()) {
      mSprite3Ds.erase(itr);
    }
  }

private:
  bool load_shader();
  void draw_skybox();
  void draw_3d();
  void draw_2d();
  void draw_instancing_2d();
  void draw_instancing_3d();
  void enable_vertex_attrib_array();
  void disable_vertex_attrib_array();

  std::string vertexID;

  gl_shader mSpriteShader;
  gl_shader mAlphaShader;
  gl_shader mSpriteInstanceShader;
  gl_shader mAlphaInstanceShader;
  std::vector<std::pair<shader, gl_shader>> userPipelines;
  GLuint mTextureID;
  std::unordered_map<handle_t, GLuint> mTextureIDs;
  std::unordered_map<std::string, gl_vertex_array> m_VertexArrays;
  ::SDL_GLContext mContext;
  std::vector<std::shared_ptr<draw_object>> mSprite2Ds;
  std::vector<std::shared_ptr<draw_object>> mSprite3Ds;
  std::vector<gl_instancing> m_instancing_2d;
  std::vector<gl_instancing> m_instancing_3d;
  float prev_window_x;
  float prev_window_y;
};
} // namespace nen