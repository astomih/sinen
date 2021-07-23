#pragma once
#ifndef EMSCRIPTEN
#include <SDL.h>
#include <string>
#include <GL/glew.h>
#include "ShaderGL.h"
#include <Engine/include/Window.hpp>
#include <Engine/include/Sprite.h>
#endif
namespace nen
{
	class Renderer;

	namespace gl
	{

		struct VertexArrayForGL : public nen::VertexArray
		{
			uint32_t vertexID;
			uint32_t indexID;
		};
#ifndef EMSCRIPTEN

		class GLRenderer
		{
		public:
			GLRenderer() : mRenderer(nullptr) {}
			void initialize(struct ::SDL_Window *window, ::SDL_GLContext context);
			void prepare();
			void cleanup() {}
			void render();
			void registerTexture(std::shared_ptr<Texture>, const TextureType &type);
			void pushSprite2d(std::shared_ptr<Sprite> sprite2d)
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
			void eraseSprite2d(std::shared_ptr<Sprite> sprite2d)
			{
				auto itr = std::find(mSprite2Ds.begin(), mSprite2Ds.end(), sprite2d);
				if (itr != mSprite2Ds.end())
				{
					mSprite2Ds.erase(itr);
				}
			}

			void pushSprite3d(std::shared_ptr<Sprite> sprite3d)
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
			void eraseSprite3d(std::shared_ptr<Sprite> sprite3d)
			{
				auto itr = std::find(mSprite3Ds.begin(), mSprite3Ds.end(), sprite3d);
				if (itr != mSprite3Ds.end())
				{
					mSprite3Ds.erase(itr);
				}
			}
			void setRenderer(nen::Renderer *renderer)
			{
				mRenderer = renderer;
			}

			void AddVertexArray(const VertexArrayForGL &vArray, std::string_view name);

			void SetEffect(std::unique_ptr<class EffectGL> effect) { mEffectManager = std::move(effect); }

		private:
			std::unique_ptr<class EffectGL> mEffectManager;
			uint32_t AddVertexArray(const VertexArrayForGL &);
			bool loadShader();
			void createSpriteVerts();
			void createBoxVerts();
			nen::Renderer *mRenderer;

			ShaderGL *mSpriteShader;
			ShaderGL *mAlphaShader;
			GLuint mTextureID;
			std::unordered_map<std::string, GLuint> mTextureIDs;
			std::unordered_map<std::string, VertexArrayForGL> m_VertexArrays;
			::SDL_Window *mWindow;
			::SDL_GLContext mContext;
			std::vector<std::shared_ptr<Sprite>> mSprite2Ds;
			std::vector<std::shared_ptr<Sprite>> mSprite3Ds;
		};

#endif
	}
}