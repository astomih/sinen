#pragma once
#include <Effekseer.h>
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <GL/glew.h>
#endif
#include <EffekseerRendererGL.h>

namespace nen
{
	class EffectManager
	{
	public:
		EffectManager();
		virtual ~EffectManager() {}
		virtual void Init() {}
		Effekseer::EffectRef GetEffect(const std::u16string &filePath);
		EffekseerRenderer::RendererRef GetRenderer() { return renderer; }
		Effekseer::ManagerRef GetManager() { return manager; }

	protected:
		std::unordered_map<std::u16string, Effekseer::EffectRef> effects;
		Effekseer::ManagerRef manager;
		EffekseerRenderer::RendererRef renderer;
	};
}