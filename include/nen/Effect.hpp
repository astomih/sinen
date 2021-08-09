#pragma once
#include <Effekseer.h>
#include <EffekseerRendererGL.h>
#ifndef EMSCRIPTEN
#include <EffekseerRendererVulkan.h>
#endif
#ifdef EMSCRIPTEN
#include "ES/ESRenderer.h"
#endif

#ifndef EMSCRIPTEN
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

#ifdef EMSCRIPTEN
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

#ifdef EMSCRIPTEN
#endif
namespace nen
{
	class Effect
	{
	public:
		Effect();
#ifndef EMSCRIPTEN
		void Init(class nen::vk::VKRenderer *vkrenderer, class nen::vk::VKBase *vkbase);
		void Init(class nen::gl::GLRenderer *glrenderer);
#endif
#ifdef EMSCRIPTEN
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