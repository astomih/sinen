#ifndef SINEN_GL_RENDERER_HPP
#define SINEN_GL_RENDERER_HPP
// std
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
// external
#include <SDL.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#if defined(EMSCRIPTEN) || defined(MOBILE)
#include <GLES3/gl3.h>
#endif
// internal
#include "gl_drawable.hpp"
#include "gl_shader.hpp"
#include "gl_uniform_buffer.hpp"
#include <drawable/drawable.hpp>
#include <render/renderer.hpp>
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
  gl_uniform_buffer ubo;
};

class gl_shader_parameter {
public:
  drawable::parameter param;
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
  void add_model(const model &m);
  void update_model(const model &m);
  void draw2d(std::shared_ptr<class drawable> sprite);
  void draw3d(std::shared_ptr<class drawable> sprite);

  void load_shader(const shader &shaderInfo);
  void unload_shader(const shader &shaderInfo);

  void add_instancing(const instancing &_instancing);

  void prepare();
  void cleanup() {}
  void create_texture(texture tex);
  void destroy_texture(texture tex);
  void remove_sprite2d(std::shared_ptr<drawable> sprite2d) {
    for (auto it = m_drawer_2ds.begin(); it != m_drawer_2ds.end(); ++it) {
      if ((*it).drawable_object == sprite2d) {
        m_drawer_2ds.erase(it);
        return;
      }
    }
  }

  void remove_sprite3d(std::shared_ptr<drawable> sprite3d) {
    for (auto it = m_drawer_3ds.begin(); it != m_drawer_3ds.end(); ++it) {
      if ((*it).drawable_object == sprite3d) {
        m_drawer_3ds.erase(it);
        return;
      }
    }
  }

private:
  bool load_shader();
  void draw_skybox();
  void draw_3d();
  void draw_2d();
  void draw_instancing_2d();
  void draw_instancing_3d();
  void create_render_texture();
  void destroy_render_texture();
  void enable_vertex_attrib_array();
  void disable_vertex_attrib_array();

  uint32_t framebuffer;
  uint32_t rendertexture;
  uint32_t depthbuffer;

  std::string vertexID;

  std::unordered_map<std::string, gl_shader> m_shaders;
  std::vector<std::pair<shader, gl_shader>> m_user_pipelines;
  GLuint mTextureID;
  std::unordered_map<handle_t, GLuint> mTextureIDs;
  std::unordered_map<std::string, gl_vertex_array> m_VertexArrays;
  ::SDL_GLContext mContext;
  std::vector<gl_drawable> m_drawer_2ds;
  std::vector<gl_drawable> m_drawer_3ds;
  std::vector<gl_instancing> m_instancing_2d;
  std::vector<gl_instancing> m_instancing_3d;
  gl_uniform_buffer render_texture_ubo;
  float prev_window_x;
  float prev_window_y;
};
} // namespace sinen
#endif // !SINEN_GL_RENDE
