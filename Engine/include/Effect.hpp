#pragma once
#include <Effekseer.h>
#include <EffekseerRendererGL.h>
#ifndef EMSCRIPTEN
#include <EffekseerRendererVulkan.h>
#endif
#ifdef EMSCRIPTEN
#include <ESRenderer.h>
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
class CustomFileReader : public Effekseer::FileReader
{
	uint8_t *fileData;
	size_t fileSize;
	int currentPosition;

public:
	CustomFileReader(uint8_t *fileData, size_t fileSize) : fileData(fileData), fileSize(fileSize), currentPosition(0) {}
	~CustomFileReader() { free(fileData); }
	size_t Read(void *buffer, size_t size)
	{
		if (currentPosition + size > fileSize)
		{
			size = fileSize - currentPosition;
		}
		memcpy(buffer, fileData + currentPosition, size);
		currentPosition += size;
		return size;
	}
	void Seek(int position) { currentPosition = position; }
	int GetPosition() { return currentPosition; }
	size_t GetLength() { return fileSize; }
};

class CustomFileInterface : public Effekseer::FileInterface
{
public:
	Effekseer::FileReader *OpenRead(const EFK_CHAR *path, bool isRequired)
	{
		// Request to load file
		int loaded = EM_ASM_INT({ return Module._loadBinary(UTF16ToString($0), $1) != null; }, path, isRequired);
		if (!loaded)
		{
			return nullptr;
		}

		uint8_t *fileData = nullptr;
		int fileSize = 0;

		// Copy data from arraybuffer
		EM_ASM_INT(
			{
				var buffer = Module._loadBinary(UTF16ToString($0), $3);
				var memptr = _malloc(buffer.byteLength);
				HEAP8.set(new Uint8Array(buffer), memptr);
				setValue($1, memptr, "i32");
				setValue($2, buffer.byteLength, "i32");
			},
			path,
			&fileData,
			&fileSize,
			isRequired);

		if (fileData == nullptr)
		{
			return nullptr;
		}

		return new CustomFileReader(fileData, fileSize);
	}

	Effekseer::FileReader *OpenRead(const EFK_CHAR *path) override { return OpenRead(path, true); }

	Effekseer::FileReader *TryOpenRead(const EFK_CHAR *path) override { return OpenRead(path, false); }

	Effekseer::FileWriter *OpenWrite(const EFK_CHAR *path) override { return nullptr; }
};
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
		CustomFileInterface fileInterface;
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