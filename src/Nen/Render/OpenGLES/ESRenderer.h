#pragma once
#if defined(EMSCRIPTEN) || defined(MOBILE)
#include <unordered_map>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif
#include <SDL.h>
#include <GLES3/gl3.h>
#include <Texture/Texture.hpp>
#include <string>
#include <Window/Window.hpp>
#include <DrawObject/DrawObject.hpp>
#include <Render/Renderer.hpp>
#include <Vertex/VertexArray.hpp>

namespace nen::es
{
	struct VertexArrayForES : public VertexArray
	{
		uint32_t vertexID;
		uint32_t indexID;
	};
	class ESRenderer : public IRenderer
	{
	public:
		ESRenderer();
		~ESRenderer() override {}
		void Initialize(struct SDL_Window *window) override;
		void Shutdown() override;
		void Render() override;
		void AddVertexArray(const VertexArray &vArray, std::string_view name) override;
		void ChangeBufferDrawObject(std::shared_ptr<class DrawObject> sprite, const TextureType type) override;

		void AddDrawObject2D(std::shared_ptr<class DrawObject> sprite, std::shared_ptr<Texture> texture) override;
		void RemoveDrawObject2D(std::shared_ptr<class DrawObject> sprite) override;
		void AddDrawObject3D(std::shared_ptr<class DrawObject> sprite, std::shared_ptr<Texture> texture) override;
		void RemoveDrawObject3D(std::shared_ptr<class DrawObject> sprite) override;

		void prepare();
		void cleanup() {}
		void registerTexture(std::shared_ptr<Texture>, const TextureType &type);
		void pushSprite2d(std::shared_ptr<DrawObject> sprite2d)
		{
			auto iter = mSprite2Ds.begin();
			for (;
				 iter != mSprite2Ds.end();
				 ++iter)
			{
				if (sprite2d->drawOrder < (*iter)->drawOrder)
				{
					break;
				}
			}
			mSprite2Ds.insert(iter, sprite2d);
		}
		void eraseSprite2d(std::shared_ptr<DrawObject> sprite2d)
		{
			auto itr = std::find(mSprite2Ds.begin(), mSprite2Ds.end(), sprite2d);
			if (itr != mSprite2Ds.end())
			{
				mSprite2Ds.erase(itr);
			}
		}

		void pushSprite3d(std::shared_ptr<DrawObject> sprite3d)
		{
			auto iter = mSprite3Ds.begin();
			for (;
				 iter != mSprite3Ds.end();
				 ++iter)
			{
				if (sprite3d->drawOrder < (*iter)->drawOrder)
				{
					break;
				}
			}
			mSprite3Ds.insert(iter, sprite3d);
		}
		void eraseSprite3d(std::shared_ptr<DrawObject> sprite3d)
		{
			auto itr = std::find(mSprite3Ds.begin(), mSprite3Ds.end(), sprite3d);
			if (itr != mSprite3Ds.end())
			{
				mSprite3Ds.erase(itr);
			}
		}

	private:
		std::unique_ptr<class EffectManagerES> mEffectManager;
		bool loadShader();
		void createSpriteVerts();
		void createBoxVerts();

		class ShaderES *mSpriteShader;
		class ShaderES *mAlphaShader;
		GLuint mTextureID;
		std::unordered_map<std::string, GLuint> mTextureIDs;
		std::unordered_map<std::string, VertexArrayForES> m_VertexArrays;
		::SDL_GLContext mContext;
		std::vector<std::shared_ptr<DrawObject>> mSprite2Ds;
		std::vector<std::shared_ptr<DrawObject>> mSprite3Ds;
	};
}

#endif
