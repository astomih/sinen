#pragma once
#include <string>
#include <memory>
#include "Vulkan/VKRenderer.h"
#include "Vertex.h"
#include <unordered_map>
namespace nen
{
	struct SDL_Surface;
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
	struct BufferObject
	{
		~BufferObject();
		VkBuffer buffer;
		VkDeviceMemory memory;
	};
	struct ImageObject
	{
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
	};
#endif
	
	struct ShaderParameters
	{
		Matrix4 world;
		Matrix4 view;
		Matrix4 proj;
	};
	struct SDLObjectCloser
	{
		void operator()(::SDL_Surface *surface)
		{
			if (surface != nullptr)
			{
				::SDL_FreeSurface(surface);
				surface = nullptr;
			}
		}
		void operator()(::SDL_RWops *rw)
		{
			if (rw != nullptr)
			{
				::SDL_FreeRW(rw);
				rw = nullptr;
			}
		}
	};
	class Texture
	{
	public:
		Texture();
		~Texture();
		bool Load(std::string_view fileName);
		bool LoadFromMemory(std::vector<char> &buffer, std::string_view ID);
		void SetSurface(std::unique_ptr<::SDL_Surface, SDLObjectCloser> surface);
		const ::SDL_Surface &GetSurface();
		const int GetWidth() { return width; }
		const int GetHeight() { return height; }

		std::string id = "default";

	private:
		std::unique_ptr<::SDL_Surface, SDLObjectCloser> m_surface;
		int width = 0;
		int height = 0;
	};
	class TextureAsset
	{
	public:
		static void Store(std::string_view name, std::shared_ptr<Texture> texture)
		{
			mTexture.emplace(name, texture);
		}
		static std::shared_ptr<Texture> Load(std::string_view name)
		{
			return mTexture.at(name.data());
		}
		static std::shared_ptr<Texture> Load()
		{
			return mTexture[texname];
		}

	private:
		static std::string texname;
		static std::unordered_map<std::string, std::shared_ptr<Texture>> mTexture;
	};
}
