#pragma once
#include "../Vertex/Vertex.hpp"
#include <string>
#include <memory>
#include <unordered_map>
namespace nen
{
	struct ShaderParameters
	{
		Matrix4 world;
		Matrix4 view;
		Matrix4 proj;
	};

	class Texture
	{
	public:
		Texture();
		~Texture();
		bool Load(std::string_view fileName);
		bool LoadFromMemory(std::vector<char> &buffer, std::string_view ID);

		const int GetWidth() { return width; }
		const int GetHeight() { return height; }
		void SetWidth(const int w)
		{
			if (w >= 0)
				this->width = w;
		}
		void SetHeight(const int h)
		{
			if (h >= 0)
				this->height = h;
		}

		std::string id = "default";

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
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
