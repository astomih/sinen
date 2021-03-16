#pragma once
#include <SDL.h>
#include <string>
#include <GL/glew.h>
#include "ShaderGL.h"
#include <Engine/include/Window.hpp>

class SpriteGL
{
public:
	SpriteGL() : trimStart(Vector2f(0.f, 1.f)), trimEnd(Vector2f(1.f, 0.f)) {}
	ShaderParameters param;
	std::string textureIndex;
	Vector2f trimStart;
	Vector2f trimEnd;
	int drawOrder = 100;
	bool isChangeBuffer = false;
};

class GLRenderer
{
public:
	GLRenderer() : mRenderer(nullptr) {}
	void initialize(struct SDL_Window *window, SDL_GLContext context);
	void prepare();
	void cleanup() {}
	void render();
	void registerTexture(std::shared_ptr<Texture>, const TextureType &type);
	void pushSprite2d(std::shared_ptr<SpriteGL> sprite2d)
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

	void pushSprite3d(std::shared_ptr<SpriteGL> sprite3d) { mSprite3Ds.push_back(sprite3d); }
	void setRenderer(class Renderer *renderer)
	{
		mRenderer = renderer;
	}

private:
	bool loadShader();
	void createSpriteVerts();
	unsigned int mSpriteVertexID;
	unsigned int mSpriteIndexID;
	class Renderer *mRenderer;

	ShaderGL *mSpriteShader;
	ShaderGL *mAlphaShader;
	GLuint mTextureID;
	std::unordered_map<std::string, GLuint> mTextureIDs;
	SDL_Window *mWindow;
	SDL_GLContext mContext;
	std::vector<std::shared_ptr<SpriteGL>> mSprite2Ds;
	std::vector<std::shared_ptr<SpriteGL>> mSprite3Ds;
};
