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
class Renderer;

namespace es {

struct VertexArrayForES : public nen::VertexArray {
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

  void Initialize(std::shared_ptr<Window> window) override;
  void Render() override;
  void AddVertexArray(const VertexArray &vArray,
                      std::string_view name) override;
  void UpdateVertexArray(const VertexArray &vArray,
                         std::string_view name) override;
  void AddDrawObject2D(std::shared_ptr<class DrawObject> sprite,
                       std::shared_ptr<Texture> texture) override;
  void RemoveDrawObject2D(std::shared_ptr<class DrawObject> sprite) override;

  void AddDrawObject3D(std::shared_ptr<class DrawObject> sprite,
                       std::shared_ptr<Texture> texture) override;
  void RemoveDrawObject3D(std::shared_ptr<class DrawObject> sprite) override;

  void SetRenderer(class Renderer *renderer) override;

  void LoadEffect(std::shared_ptr<class Effect> effect) override;

  void LoadShader(const Shader &shaderInfo) override;
  void UnloadShader(const Shader &shaderInfo) override;

  void prepare();
  void cleanup() {}
  void registerTexture(std::shared_ptr<class Texture>, const TextureType &type);
  void pushSprite2d(std::shared_ptr<DrawObject> sprite2d) {
    auto iter = mSprite2Ds.begin();
    for (; iter != mSprite2Ds.end(); ++iter) {
      if (sprite2d->drawOrder < (*iter)->drawOrder) {
        break;
      }
    }
    mSprite2Ds.insert(iter, sprite2d);
  }
  void eraseSprite2d(std::shared_ptr<DrawObject> sprite2d) {
    auto itr = std::find(mSprite2Ds.begin(), mSprite2Ds.end(), sprite2d);
    if (itr != mSprite2Ds.end()) {
      mSprite2Ds.erase(itr);
    }
  }

  void pushSprite3d(std::shared_ptr<DrawObject> sprite3d) {
    auto iter = mSprite3Ds.begin();
    for (; iter != mSprite3Ds.end(); ++iter) {
      if (sprite3d->drawOrder < (*iter)->drawOrder) {
        break;
      }
    }
    mSprite3Ds.insert(iter, sprite3d);
  }
  void eraseSprite3d(std::shared_ptr<DrawObject> sprite3d) {
    auto itr = std::find(mSprite3Ds.begin(), mSprite3Ds.end(), sprite3d);
    if (itr != mSprite3Ds.end()) {
      mSprite3Ds.erase(itr);
    }
  }
  void setRenderer(nen::Renderer *renderer) { mRenderer = renderer; }

  std::shared_ptr<Window> GetWindow() { return mWindow; }

private:
  Renderer *mRenderer;
  std::shared_ptr<Window> mWindow;
  bool loadShader();
  void createSpriteVerts();
  void createBoxVerts();

  ShaderES *mSpriteShader;
  ShaderES *mAlphaShader;
  std::vector<std::pair<Shader, ShaderES>> userPipelines;
  GLuint mTextureID;
  std::unordered_map<std::string, GLuint> mTextureIDs;
  std::unordered_map<std::string, VertexArrayForES> m_VertexArrays;
  ::SDL_GLContext mContext;
  std::vector<std::shared_ptr<DrawObject>> mSprite2Ds;
  std::vector<std::shared_ptr<DrawObject>> mSprite3Ds;
};
} // namespace es
} // namespace nen

#endif
