#include <Nen.hpp>
#include "../Render/RendererHandle.hpp"
#include <Effekseer.h>
#include <wchar.h>

namespace nen
{
	Effect::Effect(std::string_view path)
		: loop(false)
	{
		std::string filePath;
		AssetReader::ConvertFilePath(filePath, path, AssetType::Effect);
		char16_t efkchar[256];
		Effekseer::ConvertUtf8ToUtf16(efkchar, 256 * 16, filePath.data());
		this->path = std::u16string_view{std::u16string(efkchar)};
		auto renderer = RendererHandle::GetRenderer();
		if (renderer)
		{
		}
	}

	Effect::~Effect()
	{
	}
}