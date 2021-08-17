#pragma once
#include <Effekseer.h>
#include <EffekseerRendererGL.h>
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <EffekseerRendererVulkan.h>
#endif
#if defined(EMSCRIPTEN) || defined(MOBILE)
#include "ES/ESRenderer.h"
#endif

#if !defined(EMSCRIPTEN) && !defined(MOBILE)
namespace nen::vk
{
	class EffectVK
	{
	public:
		EffectVK(Effekseer::RefPtr<EffekseerRenderer::SingleFrameMemoryPool> SFMemoryPool,
				 Effekseer::RefPtr<EffekseerRenderer::CommandList> commandlist,
				 EffekseerRenderer::RendererRef rendererRef,
				 Effekseer::ManagerRef managerRef,
				 Effect &effect);
		Effekseer::RefPtr<EffekseerRenderer::SingleFrameMemoryPool> GetMemoryPool() { return sfMemoryPool; }
		Effekseer::RefPtr<EffekseerRenderer::CommandList> GetCommandList() { return commandList; }
		EffekseerRenderer::RendererRef GetRenderer() { return renderer; }
		Effekseer::ManagerRef GetManager() { return manager; }
		Effekseer::EffectRef GetEffect(const std::u16string &name);

		int handle;

	private:
		class Effect &owner;
		Effekseer::RefPtr<EffekseerRenderer::SingleFrameMemoryPool> sfMemoryPool;
		Effekseer::RefPtr<EffekseerRenderer::CommandList> commandList;
		EffekseerRenderer::RendererRef renderer;
		Effekseer::ManagerRef manager;
	};
}

namespace nen::gl
{
	class EffectGL
	{
	public:
		EffectGL(EffekseerRenderer::RendererRef rendererRef,
				 Effekseer::ManagerRef managerRef,
				 class Effect &effect);
		EffekseerRenderer::RendererRef GetRenderer() { return renderer; }
		Effekseer::ManagerRef GetManager() { return manager; }
		Effekseer::EffectRef GetEffect(const std::u16string &name);
		int handle;

	private:
		class Effect &owner;
		EffekseerRenderer::RendererRef renderer;
		Effekseer::ManagerRef manager;
	};
}
#endif

#if defined(EMSCRIPTEN) || defined(MOBILE)
namespace nen
{
	class Effect;
}
#endif
namespace nen::es
{
	class ESRenderer;
	class EffectGL
	{
	public:
		EffectGL(EffekseerRenderer::RendererRef rendererRef,
				 Effekseer::ManagerRef managerRef,
				 ::nen::Effect &effect);
		EffekseerRenderer::RendererRef GetRenderer() { return renderer; }
		Effekseer::ManagerRef GetManager() { return manager; }
		Effekseer::EffectRef GetEffect(const std::u16string &name);
		int handle;

	private:
		class ::nen::Effect &owner;
		EffekseerRenderer::RendererRef renderer;
		Effekseer::ManagerRef manager;
	};

}

#if defined(EMSCRIPTEN) || defined(MOBILE)
#endif
namespace nen
{
	class Effect
	{
	public:
		Effect();
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
		void Init(class nen::vk::VKRenderer *vkrenderer, class nen::vk::VKBase *vkbase);
		void Init(class nen::gl::GLRenderer *glrenderer);
#endif
#if defined(EMSCRIPTEN) || defined(MOBILE)
		void Init(class es::ESRenderer *esrenderer);
#endif

		Effekseer::EffectRef GetEffectRef(const std::u16string &filePath);
		void Remove();
		void Draw();

	private:
		std::unordered_map<std::u16string, Effekseer::EffectRef> effects;
		Effekseer::ManagerRef manager;
		int handle;
	};
}