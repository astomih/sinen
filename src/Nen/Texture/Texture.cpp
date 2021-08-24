#include <SDL.h>
#include <SDL_image.h>
#include <Nen.hpp>
#include "SurfaceHandle.hpp"
namespace nen
{

	void SDLObjectCloser::operator()(::SDL_Surface* surface)
	{
		if (surface != nullptr)
		{
			::SDL_FreeSurface(surface);
			surface = nullptr;
		}
	}
	void SDLObjectCloser::operator()(::SDL_RWops* rw)
	{
		if (rw != nullptr)
		{
			::SDL_FreeRW(rw);
			rw = nullptr;
		}
	}
	std::unordered_map<std::string, std::unique_ptr<::SDL_Surface, SDLObjectCloser>> SurfaceHandle::surfaces = {};
	void SurfaceHandle::Register(std::string_view string, std::unique_ptr<::SDL_Surface, SDLObjectCloser> surface)
	{
		surfaces.insert({ std::string(string), std::move(surface) });
	}
	bool SurfaceHandle::Contain(std::string_view string)
	{
		return surfaces.contains(std::string(string));
	}
	SDL_Surface& SurfaceHandle::Load(std::string_view string)
	{
		return *surfaces[std::string(string)];
	}
	class Texture::Impl
	{
	};
	std::string TextureAsset::texname = "default";
	std::unordered_map<std::string, std::shared_ptr<Texture>> TextureAsset::mTexture;
	Texture::Texture()
		: impl(nullptr), width(0), height(0)
	{
	}

	Texture::~Texture() = default;

	bool Texture::Load(std::string_view fileName)
	{
		id = fileName.data();
		if (SurfaceHandle::Contain(id))
			return true;
		auto* surface = ::IMG_Load(fileName.data());
		if (surface)
		{
			width = surface->w;
			height = surface->h;
		}
		else
		{
			Logger::Error("%s", ::IMG_GetError());
			return false;
		}
		SurfaceHandle::Register(id, std::move(std::unique_ptr<::SDL_Surface, SDLObjectCloser>(surface)));
		return true;
	}
	bool Texture::LoadFromMemory(std::vector<char>& buffer, std::string_view ID)
	{
		id = ID.data();
		if (SurfaceHandle::Contain(id))
		{
			return true;
		}
		auto rw = std::unique_ptr<::SDL_RWops, SDLObjectCloser>(::SDL_RWFromMem(reinterpret_cast<void*>(buffer.data()), buffer.size()));
		if (!rw)
		{
			Logger::Error("%s", IMG_GetError());
			return false;
		}

		auto surface = std::unique_ptr<::SDL_Surface, SDLObjectCloser>(IMG_Load_RW(rw.get(), 1));
		if (!surface)
		{
			Logger::Error("%s", IMG_GetError());
			return false;
		}
		width = surface->w;
		height = surface->h;

		SurfaceHandle::Register(id, std::move(surface));

		return true;
	}

	/*
	void Texture::SetSurface(std::unique_ptr<::SDL_Surface, SDLObjectCloser> surface)
	{
		width = surface->w;
		height = surface->h;
		impl->SetSurface(std::move(surface));
	}

	const ::SDL_Surface& Texture::GetSurface()
	{
		if (impl->GetSurface())
			return *impl->GetSurface();
		impl->SetSurface(std::move(std::unique_ptr<::SDL_Surface, SDLObjectCloser>(::SDL_CreateRGBSurface(NULL, 32, 32, 32, 0, 0, 0, 0))));
		::SDL_FillRect(impl->GetSurface(), NULL, SDL_MapRGBA(impl->GetSurface()->format, 0, 0, 0, 0));
		if (impl->GetSurface())
			return *impl->GetSurface();
		Logger::Error("%s", SDL_GetError());
	}
	*/


}