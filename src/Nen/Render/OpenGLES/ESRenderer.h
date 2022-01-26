#pragma once
#if defined(EMSCRIPTEN) || defined(MOBILE)
#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#include "ShaderES.h"
#include <DrawObject/DrawObject.hpp>
#include <GLES3/gl3.h>
#include <Render/Renderer.hpp>
#include <SDL.h>
#include <Texture/Texture.hpp>
#include <Vertex/VertexArray.hpp>

namespace nen {
class renderer;

namespace es {

struct VertexArrayForES : public nen::vertex_array {
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

class ESRenderer : public IRenderer {
public:
  ESRenderer();
  ~ESRenderer() override {}

  void Initialize(std::shared_ptr<window>) override;
  void Render() override;
  void AddVertexArray(const vertex_array &vArray,
                      std::string_view name) override;
  void UpdateVertexArray(const vertex_array &vArray,
                         std::string_view name) override;
  void AddDrawObject2D(std::shared_ptr<class draw_object> sprite,
                       std::shared_ptr<texture>) override;
  void RemoveDrawObject2D(std::shared_ptr<class draw_object> sprite) override;

  void AddDrawObject3D(std::shared_ptr<class draw_object> sprite,
                       std::shared_ptr<texture>) override;
  void RemoveDrawObject3D(std::shared_ptr<class draw_object> sprite) override;

  void SetRenderer(class renderer *) override;

  void LoadEffect(std::shared_ptr<class effect>) override;

  void LoadShader(const shader &shaderInfo) override;
  void UnloadShader(const shader &shaderInfo) override;

  void prepare();
  void cleanup() {}
  void registerTexture(std::shared_ptr<class texture>, const texture_type &);
  void pushSprite2d(std::shared_ptr<draw_object> sprite2d) {
    auto iter = mSprite2Ds.begin();
    for (; iter != mSprite2Ds.end(); ++iter) {
      if (sprite2d->drawOrder < (*iter)->drawOrder) {
        break;
      }
    }
    mSprite2Ds.insert(iter, sprite2d);
  }
  void eraseSprite2d(std::shared_ptr<draw_object> sprite2d) {
    auto itr = std::find(mSprite2Ds.begin(), mSprite2Ds.end(), sprite2d);
    if (itr != mSprite2Ds.end()) {
      mSprite2Ds.erase(itr);
    }
  }

  void pushSprite3d(std::shared_ptr<draw_object> sprite3d) {
    auto iter = mSprite3Ds.begin();
    for (; iter != mSprite3Ds.end(); ++iter) {
      if (sprite3d->drawOrder < (*iter)->drawOrder) {
        break;
      }
    }
    mSprite3Ds.insert(iter, sprite3d);
  }
  void eraseSprite3d(std::shared_ptr<draw_object> sprite3d) {
    auto itr = std::find(mSprite3Ds.begin(), mSprite3Ds.end(), sprite3d);
    if (itr != mSprite3Ds.end()) {
      mSprite3Ds.erase(itr);
    }
  }
  void setRenderer(nen::renderer *_renderer) { mRenderer = _renderer; }

  std::shared_ptr<window> GetWindow() { return mWindow; }

private:
  renderer *mRenderer;
  std::shared_ptr<window> mWindow;
  bool loadShader();

  ShaderES *mSpriteShader;
  ShaderES *mAlphaShader;
  std::vector<std::pair<shader, ShaderES>> userPipelines;
  GLuint mTextureID;
  std::unordered_map<std::string, GLuint> mTextureIDs;
  std::unordered_map<std::string, VertexArrayForES> m_VertexArrays;
  ::SDL_GLContext mContext;
  std::vector<std::shared_ptr<draw_object>> mSprite2Ds;
  std::vector<std::shared_ptr<draw_object>> mSprite3Ds;
};
} // namespace es
} // namespace nen

#endif
