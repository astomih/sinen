#include <Nen.hpp>
#include "../Render/RendererHandle.hpp"

namespace nen
{
	Effect::Effect(std::u16string_view path)
		: path(path), loop(false)
	{
		auto renderer = RendererHandle::GetRenderer();
		if(renderer)
		{
			
		}
	}

	Effect::~Effect()
	{
	}
}