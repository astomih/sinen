#ifndef SINEN_GL_RENDERER_HPP
#define SINEN_GL_RENDERER_HPP
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
#include <drawable/drawable.hpp>
#include <memory>
#include <render/renderer.hpp>
#include <string>
#include <texture/texture.hpp>
#include <vertex/vertex_array.hpp>

namespace sinen {
struct gl_vertex_array : public vertex_array {
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
  uint32_t vao;
};

class gl_shader_parameter {
public:
  drawable::parameter param;
  matrix4 view;
  matrix4 projection;
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
  void draw2d(std::shared_ptr<class drawable> sprite);
  void draw3d(std::shared_ptr<class drawable> sprite);

  void load_shader(const shader &shaderInfo);
  void unload_shader(const shader &shaderInfo);

  void add_instancing(const instancing &_instancing);

  void prepare();
  void cleanup() {}
  void create_texture(texture tex);
  void add_sprite2d(std::shared_ptr<drawable> sprite2d) {
    auto iter = m_drawer_2ds.begin();
    for (; iter != m_drawer_2ds.end(); ++iter) {
      if (sprite2d->drawOrder < (*iter)->drawOrder) {
        break;
      }
    }
    m_drawer_2ds.insert(iter, sprite2d);
  }
  void remove_sprite2d(std::shared_ptr<drawable> sprite2d) {
    auto itr = std::find(m_drawer_2ds.begin(), m_drawer_2ds.end(), sprite2d);
    if (itr != m_drawer_2ds.end()) {
      m_drawer_2ds.erase(itr);
    }
  }

  void add_sprite3d(std::shared_ptr<drawable> sprite3d) {
    auto iter = m_drawer_3ds.begin();
    for (; iter != m_drawer_3ds.end(); ++iter) {
      if (sprite3d->drawOrder < (*iter)->drawOrder) {
        break;
      }
    }
    m_drawer_3ds.insert(iter, sprite3d);
  }
  void remove_sprite3d(std::shared_ptr<drawable> sprite3d) {
    auto itr = std::find(m_drawer_3ds.begin(), m_drawer_3ds.end(), sprite3d);
    if (itr != m_drawer_3ds.end()) {
      m_drawer_3ds.erase(itr);
    }
  }

private:
  bool load_shader();
  void draw_skybox();
  void draw_3d();
  void draw_2d();
  void draw_instancing_2d();
  void draw_instancing_3d();
  void prepare_render_texture();
  void prepare_depth_texture();
  void enable_vertex_attrib_array();
  void disable_vertex_attrib_array();

  uint32_t framebuffer;
  uint32_t rendertexture;
  uint32_t depthbuffer;

  uint32_t shadowframebuffer;
  uint32_t shadowdepthtexture;

  std::string vertexID;

  gl_shader mSpriteShader;
  gl_shader mAlphaShader;
  gl_shader mSpriteInstanceShader;
  gl_shader mAlphaInstanceShader;
  gl_shader m_render_texture_shader;
  gl_shader m_depth_texture_shader;
  gl_shader m_depth_texture_instanced_shader;
  std::vector<std::pair<shader, gl_shader>> m_user_pipelines;
  GLuint mTextureID;
  std::unordered_map<handle_t, GLuint> mTextureIDs;
  std::unordered_map<std::string, gl_vertex_array> m_VertexArrays;
  ::SDL_GLContext mContext;
  std::vector<std::shared_ptr<drawable>> m_drawer_2ds;
  std::vector<std::shared_ptr<drawable>> m_drawer_3ds;
  std::vector<gl_instancing> m_instancing_2d;
  std::vector<gl_instancing> m_instancing_3d;
  float prev_window_x;
  float prev_window_y;
};
} // namespace sinen
#endif // !SINEN_GL_RENDE