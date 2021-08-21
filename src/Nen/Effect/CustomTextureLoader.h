#pragma once
#include <Effekseer.h>
class CustomTextureLoader : public Effekseer::TextureLoader
{
private:
	::Effekseer::Backend::GraphicsDevice* graphicsDevice_ = nullptr;

public:
	CustomTextureLoader(::Effekseer::Backend::GraphicsDevice* graphicsDevice) : graphicsDevice_(graphicsDevice) {}

	~CustomTextureLoader() = default;

public:
	Effekseer::TextureRef Load(const EFK_CHAR* path, Effekseer::TextureType textureType) override;

	void Unload(Effekseer::TextureRef data) override {}
};
