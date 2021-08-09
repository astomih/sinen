#include <Engine.hpp>
#include <Components.hpp>
#include <SDL_image.h>
#include <SDL_net.h>
namespace nen
{
	Renderer::Renderer(GraphicsAPI api)
		: transPic(nullptr), mScene(nullptr), mWindow(nullptr),
#ifndef EMSCRIPTEN
		  vkRenderer(nullptr),
		  glRenderer(nullptr),
#endif
#ifdef EMSCRIPTEN
		  esRenderer(nullptr),
#endif
		  RendererAPI(api)
	{
		mEffectManager = std::make_unique<Effect>();
#ifndef EMSCRIPTEN
		SDL_GLContext context;
		switch (RendererAPI)
		{
		case GraphicsAPI::Vulkan:
			vkRenderer = std::make_unique<vk::VKRenderer>();
			SDL_Init(SDL_INIT_EVERYTHING);
			TTF_Init();
			IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF);
			mWindow = SDL_CreateWindow(
				std::string(Window::name + " : Vulkan").c_str(),
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				static_cast<int>(Window::Size.x),
				static_cast<int>(Window::Size.y),
				SDL_WINDOW_VULKAN);
			vkRenderer->initialize(mWindow, Window::name.c_str());
			vkRenderer->setRenderer(this);
			mEffectManager->Init(vkRenderer.get(), vkRenderer->GetBase());
			break;
		case GraphicsAPI::OpenGL:
			glRenderer = std::make_unique<gl::GLRenderer>();
			SDL_Init(SDL_INIT_EVERYTHING);
			TTF_Init();
			IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF);
			// Set OpenGL attributes
			// Use the core OpenGL profile
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			// Specify version 3.3
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
			// Request a color buffer with 8-bits per RGBA channel
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
			// Enable double buffering
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			// Force OpenGL to use hardware acceleration
			SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
			mWindow = SDL_CreateWindow(
				std::string(Window::name + " : OpenGL").c_str(),
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				static_cast<int>(Window::Size.x),
				static_cast<int>(Window::Size.y),
				SDL_WINDOW_OPENGL);

			context = SDL_GL_CreateContext(mWindow);
			SDL_GL_MakeCurrent(mWindow, context);
			glewExperimental = GL_TRUE;
			if (glewInit() != GLEW_OK)
			{
				std::cout << "ERROR: glew isn't init" << std::endl;
			}
			glGetError();
			glRenderer->setRenderer(this);
			glRenderer->initialize(mWindow, context);
			mEffectManager->Init(glRenderer.get());
			break;
		default:
			break;
		}
#endif
#ifdef EMSCRIPTEN
		SDL_GLContext context;
		esRenderer = std::make_unique<es::ESRenderer>();
		SDL_Init(SDL_INIT_EVERYTHING);
		if (TTF_Init() == -1)
			std::cout << "SDL2_TTF failed initialize." << std::endl;
		IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		// Request a color buffer with 8-bits per RGBA channel
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		// Enable double buffering
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		mWindow = SDL_CreateWindow(
			std::string(Window::name + " : ES").c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			static_cast<int>(Window::Size.x),
			static_cast<int>(Window::Size.y),
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		std::cout << "INFO: Window created" << std::endl;

		context = SDL_GL_CreateContext(mWindow);
		SDL_GL_MakeCurrent(mWindow, context);
		esRenderer->setRenderer(this);
		esRenderer->initialize(mWindow, context);
		std::cout << "INFO: Renderer Initialized" << std::endl;
		mEffectManager->Init(esRenderer.get());
		std::cout << "INFO: Effekseer Initialized" << std::endl;
#endif
		if (SDLNet_Init() != 0)
			std::cout << "net init error." << std::endl;
		
		Window::Info::id = SDL_GetWindowID(mWindow);
		SDL_VERSION(&Window::Info::info.version);
	}

	bool Renderer::Initialize(std::shared_ptr<Scene> scene, std::shared_ptr<Transition> transition)
	{
		return true;
	}

	void Renderer::Shutdown()
	{
#ifndef EMSCRIPTEN
		if (RendererAPI == GraphicsAPI::Vulkan)
			vkRenderer->terminate();
#endif
	}

	void Renderer::UnloadData()
	{
		// Destroy textures
		for (auto i : mTextures3D)
		{
			delete i.second;
			i.second = nullptr;
		}
		mTextures3D.clear();
	}

	void Renderer::Draw()
	{
#ifndef EMSCRIPTEN
		if (RendererAPI == GraphicsAPI::Vulkan)
			vkRenderer->render();
		if (RendererAPI == GraphicsAPI::OpenGL)
			glRenderer->render();
#endif
#ifdef EMSCRIPTEN
		esRenderer->render();
#endif
	}

	void Renderer::AddSprite2D(std::shared_ptr<nen::Sprite> sprite, std::shared_ptr<Texture> texture)
	{
#ifndef EMSCRIPTEN
		if (this->RendererAPI == GraphicsAPI::Vulkan)
		{
			auto t = std::make_shared<vk::SpriteVK>();
			if (sprite->isChangeBuffer)
				t->buffer = GetVK().CreateBuffer(
					sizeof(Vertex), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			t->sprite = sprite;
			GetVK().registerImageObject(texture);
			t->mTexture = texture;
			GetVK().registerTexture(t, texture->id, TextureType::Image2D);
		}
		else if (this->RendererAPI == GraphicsAPI::OpenGL)
		{
			GetGL().registerTexture(texture, TextureType::Image2D);
			GetGL().pushSprite2d(sprite);
		}
#endif
#ifdef EMSCRIPTEN
		GetES().registerTexture(texture, TextureType::Image2D);
		GetES().pushSprite2d(sprite);
#endif

		// Find the insertion point in the sorted vector
		// (The first element with a higher draw order than me)
		const auto myDrawOrder = sprite->drawOrder;
		auto iter = mSprite2Ds.begin();
		for (;
			 iter != mSprite2Ds.end();
			 ++iter)
		{
			if (myDrawOrder < (*iter)->drawOrder)
			{
				break;
			}
		}

		// Inserts element before position of iterator
		mSprite2Ds.insert(iter, sprite);
	}

	void Renderer::RemoveSprite2D(std::shared_ptr<Sprite> sprite)
	{
#ifndef EMSCRIPTEN
		if (this->RendererAPI == GraphicsAPI::Vulkan)
		{
			auto &sprites = GetVK().GetSprite2Ds();
			auto iter = sprites.begin();
			for (;
				 iter != sprites.end();)
			{
				if (sprite == (*iter)->sprite)
				{
					GetVK().unregisterTexture((*iter), TextureType::Image2D);
					iter = sprites.begin();
					if (iter == sprites.end())
						break;
				}
				iter++;
			}
		}
		else
		{
			GetGL().eraseSprite2d(sprite);
		}
#endif
#ifdef EMSCRIPTEN
		GetES().eraseSprite2d(sprite);
#endif
		auto iter = std::find(mSprite2Ds.begin(), mSprite2Ds.end(), sprite);
		if (iter != mSprite2Ds.end())
			mSprite2Ds.erase(iter);
	}

	void Renderer::AddSprite3D(std::shared_ptr<Sprite> sprite, std::shared_ptr<Texture> texture)
	{
#ifndef EMSCRIPTEN
		if (this->RendererAPI == GraphicsAPI::Vulkan)
		{
			auto t = std::make_shared<vk::SpriteVK>();
			t->sprite = sprite;
			GetVK().registerImageObject(texture);
			t->mTexture = texture;
			GetVK().registerTexture(t, texture->id, TextureType::Image3D);
		}
		else
		{
			GetGL().registerTexture(texture, TextureType::Image3D);
			GetGL().pushSprite3d(sprite);
		}
#endif
#ifdef EMSCRIPTEN
		GetES().registerTexture(texture, TextureType::Image3D);
		GetES().pushSprite3d(sprite);
#endif

		// Find the insertion point in the sorted vector
		// (The first element with a higher draw order than me)
		const auto myDrawOrder = sprite->drawOrder;
		auto iter = mSprite3Ds.begin();
		for (;
			 iter != mSprite3Ds.end();
			 ++iter)
		{
			if (myDrawOrder < (*iter)->drawOrder)
			{
				break;
			}
		}

		// Inserts element before position of iterator
		mSprite3Ds.insert(iter, sprite);
	}

	void Renderer::RemoveSprite3D(std::shared_ptr<Sprite> sprite)
	{
#ifndef EMSCRIPTEN
		if (this->RendererAPI == GraphicsAPI::Vulkan)
		{
			auto &sprites = GetVK().GetSprite3Ds();
			auto iter = sprites.begin();
			for (;
				 iter != sprites.end();
				 ++iter)
			{
				if (sprite == (*iter)->sprite)
				{
					GetVK().unregisterTexture((*iter), TextureType::Image3D);
					iter = sprites.begin();
					if (iter == sprites.end())
						break;
				}
			}
		}
		else
		{
			GetGL().eraseSprite3d(sprite);
		}
#endif
#ifdef EMSCRIPTEN
		GetES().eraseSprite3d(sprite);
#endif

		auto iter = std::find(mSprite3Ds.begin(), mSprite3Ds.end(), sprite);
		if (iter != mSprite3Ds.end())
			mSprite3Ds.erase(iter);
	}

	void Renderer::ChangeBufferSprite(std::shared_ptr<Sprite> sprite, TextureType type)
	{
#ifndef EMSCRIPTEN
		if (this->RendererAPI == GraphicsAPI::Vulkan)
		{

			if (type == TextureType::Image2D)
			{
				auto &sprites = GetVK().GetSprite2Ds();
				for (auto &i : sprites)
				{
					if (sprite == i->sprite)
					{
						if (sprite->isChangeBuffer && i->buffer.buffer == 0)
						{
							i->buffer = GetVK().CreateBuffer(
								sizeof(Vertex) * 4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
							break;
						}
					}
				}
			}
			if (type == TextureType::Image3D)
			{
				auto &sprites = GetVK().GetSprite3Ds();
				for (auto &i : sprites)
				{
					if (sprite == i->sprite)
					{
						if (sprite->isChangeBuffer && i->buffer.buffer == 0)
						{
							i->buffer = GetVK().CreateBuffer(
								sizeof(Vertex) * 4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
							break;
						}
					}
				}
			}
		}
#endif
	}

	void Renderer::AddEffectComp(EffectComponent *effect)
	{
		mEffectComp.emplace_back(effect);
	}
	void Renderer::RemoveEffectComp(EffectComponent *effect)
	{
		auto iter = std::find(mEffectComp.begin(), mEffectComp.end(), effect);
		mEffectComp.erase(iter);
	}
	Texture *Renderer::GetTexture(std::string_view fileName)
	{
		Texture *tex = nullptr;
		auto iter = mTextures3D.find(fileName.data());
		if (iter != mTextures3D.end())
		{
			tex = iter->second;
		}
		else
		{
			tex = new Texture();
			if (tex->Load(fileName))
			{
				mTextures3D.emplace(fileName, tex);
			}
			else
			{
				delete tex;
				tex = nullptr;
			}
		}
		return tex;
	}

	void Renderer::AddVertexArray(const VertexArray &vArray, std::string_view name)
	{
#ifndef EMSCRIPTEN
		if (RendererAPI == GraphicsAPI::Vulkan)
		{
			vk::VertexArrayForVK vArrayVK;
			vArrayVK.indexCount = vArray.indexCount;
			vArrayVK.indices = vArray.indices;
			vArrayVK.vertices = vArray.vertices;
			auto vArraySize = vArray.vertices.size() * sizeof(Vertex);
			vArrayVK.vertexBuffer = GetVK().CreateBuffer(vArraySize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			vArrayVK.indexBuffer = GetVK().CreateBuffer(vArray.indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

			//Write vertex data
			GetVK().MapMemory(vArrayVK.vertexBuffer.memory, vArrayVK.vertices.data(), vArraySize);
			// Write index data
			GetVK().MapMemory(vArrayVK.indexBuffer.memory, vArrayVK.indices.data(), sizeof(uint32_t) * vArrayVK.indices.size());
			// Insert VertexArray array
			GetVK().AddVertexArray(vArrayVK, name);
		}
		else
		{
			gl::VertexArrayForGL vArrayGL;
			vArrayGL.indexCount = vArray.indexCount;
			vArrayGL.indices = vArray.indices;
			vArrayGL.vertices = vArray.vertices;

			// Create vertex array
			glGenVertexArrays(1, &vArrayGL.vertexID);
			glBindVertexArray(vArrayGL.vertexID);

			// Create vertex buffer
			glGenBuffers(1, &vArrayGL.vertexID);
			glBindBuffer(GL_ARRAY_BUFFER, vArrayGL.vertexID);
			auto vArraySize = vArrayGL.vertices.size() * sizeof(Vertex);
			glBufferData(GL_ARRAY_BUFFER, vArraySize, vArrayGL.vertices.data(), GL_STATIC_DRAW);

			// Create index buffer
			glGenBuffers(1, &vArrayGL.indexID);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vArrayGL.indexID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, vArrayGL.indices.size() * sizeof(uint32_t), vArrayGL.indices.data(), GL_STATIC_DRAW);
			GetGL().AddVertexArray(vArrayGL, name);
		}
#endif
#ifdef EMSCRIPTEN
		gl::VertexArrayForGL vArrayGL;
		vArrayGL.indexCount = vArray.indexCount;
		vArrayGL.indices = vArray.indices;
		vArrayGL.vertices = vArray.vertices;

		// Create vertex array
		glGenVertexArraysOES(1, &vArrayGL.vertexID);
		glBindVertexArrayOES(vArrayGL.vertexID);

		// Create vertex buffer
		glGenBuffers(1, &vArrayGL.vertexID);
		glBindBuffer(GL_ARRAY_BUFFER, vArrayGL.vertexID);
		auto vArraySize = vArrayGL.vertices.size() * sizeof(Vertex);
		glBufferData(GL_ARRAY_BUFFER, vArraySize, vArrayGL.vertices.data(), GL_STATIC_DRAW);

		// Create index buffer
		glGenBuffers(1, &vArrayGL.indexID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vArrayGL.indexID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, vArrayGL.indices.size() * sizeof(uint32_t), vArrayGL.indices.data(), GL_STATIC_DRAW);
		GetES().AddVertexArray(vArrayGL, name);

#endif
	}

	Texture *Renderer::GetTextureFromMemory(const unsigned char *const buffer, const std::string &key)
	{
		Texture *tex = nullptr;
		auto iter = mTextures3D.find(key);
		if (iter != mTextures3D.end())
		{
			tex = iter->second;
		}
		else
		{
			tex = new Texture();
			/*
			//if (tex->LoadFromMemory(buffer))
			{
				mTextures3D.emplace(key, tex);
			}
			//else
			{
				delete tex;
				tex = nullptr;
			}
			*/
		}
		return tex;
	}

	Effect *Renderer::GetEffect(const std::u16string &fileName)
	{
		return nullptr;
	}
}
