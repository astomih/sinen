#include <Nen.hpp>
#include "../Render/RendererHandle.hpp"
#include <wchar.h>

namespace nen
{
	Effect::Effect(std::string_view path)
		: loop(false)
	{
	}

	Effect::~Effect()
	{
	}
}