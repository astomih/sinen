#include "CustomTextureLoader.h"
#include <nen.hpp>
#include <EffekseerRenderer/EffekseerRendererGL.MaterialLoader.h>
#include <EffekseerRenderer/EffekseerRendererGL.RendererImplemented.h>
#include <EffekseerRenderer/GraphicsDevice.h>
#include <fstream>
#include <SDL.h>
#include <SDL_image.h>

Effekseer::TextureRef CustomTextureLoader::Load(const EFK_CHAR* path, Effekseer::TextureType textureType)
{
	std::array<char, 260> path8;
	Effekseer::ConvertUtf16ToUtf8(path8.data(), static_cast<int32_t>(path8.size()), path);
	SDL_Surface* surf = IMG_Load(path8.data());
	if (!surf)
	{
		nen::Logger::Error("Failed to load \"%s\". IMG_Error: %s", std::string(path8.data()), IMG_GetError());
	}
	::SDL_LockSurface(surf);
	auto formatbuf = ::SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888);
	formatbuf->BytesPerPixel = 4;
	auto imagedata = ::SDL_ConvertSurface(surf, formatbuf, 0);
	SDL_UnlockSurface(surf);
	// Generate a GL texture
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, imagedata->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	std::string pathStr = path8.data();
	auto backend = static_cast<EffekseerRendererGL::Backend::GraphicsDevice*>(graphicsDevice_)
		->CreateTexture(texture, false, [texture, pathStr]() -> void
			{
				glDeleteTextures(1, &texture);
			});
	auto textureData = Effekseer::MakeRefPtr<Effekseer::Texture>();
	textureData->SetBackend(backend);
	nen::Logger::Info("Loaded \"%s\".", pathStr);
	SDL_FreeSurface(surf);
	SDL_FreeSurface(imagedata);
	SDL_FreeFormat(formatbuf);
	return textureData;
}