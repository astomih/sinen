#include <Engine.hpp>
#include <Components.hpp>
#include <SDL_image.h>
namespace nen
{
	Renderer::Renderer(GraphicsAPI api)
		: transPic(nullptr), mScene(nullptr), mWindow(nullptr), vkRenderer(nullptr), glRenderer(nullptr), RendererAPI(api)
	{
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
				std::cout << "Error: glew isn't init" << std::endl;
			}
			glGetError();
			glRenderer->setRenderer(this);
			glRenderer->initialize(mWindow, context);
			break;
		default:
			break;
		}
		Window::Info::id = SDL_GetWindowID(mWindow);
		SDL_VERSION(&Window::Info::info.version);
	}

	bool Renderer::Initialize(std::shared_ptr<Scene> scene, std::shared_ptr<Transition> transition)
	{
		return true;
	}

	void Renderer::Shutdown()
	{
		if (RendererAPI == GraphicsAPI::Vulkan)
			vkRenderer->terminate();
	}

	void Renderer::UnloadData()
	{
		// Destroy textures
		for (auto i : mTextures3D)
		{
			delete i.second;
		}
		mTextures3D.clear();
	}

	void Renderer::Draw()
	{
		if (RendererAPI == GraphicsAPI::Vulkan)
			vkRenderer->render();
		if (RendererAPI == GraphicsAPI::OpenGL)
			glRenderer->render();
	}

	void Renderer::AddSprite2D(Sprite2DComponent *sprite)
	{
		// Find the insertion point in the sorted vector
		// (The first element with a higher draw order than me)
		const auto myDrawOrder = sprite->GetDrawOrder();
		auto iter = mSprite2Ds.begin();
		for (;
			 iter != mSprite2Ds.end();
			 ++iter)
		{
			if (myDrawOrder < (*iter)->GetDrawOrder())
			{
				break;
			}
		}

		// Inserts element before position of iterator
		mSprite2Ds.insert(iter, sprite);
	}

	void Renderer::RemoveSprite2D(Sprite2DComponent *sprite)
	{
		auto iter = std::find(mSprite2Ds.begin(), mSprite2Ds.end(), sprite);
		mSprite2Ds.erase(iter);
	}

	void Renderer::AddSprite3D(Sprite3DComponent *sprite)
	{
		// Find the insertion point in the sorted vector
		// (The first element with a higher draw order than me)
		const auto myDrawOrder = sprite->GetDrawOrder();
		auto iter = mSprite3Ds.begin();
		for (;
			 iter != mSprite3Ds.end();
			 ++iter)
		{
			if (myDrawOrder < (*iter)->GetDrawOrder())
			{
				break;
			}
		}

		// Inserts element before position of iterator
		mSprite3Ds.insert(iter, sprite);
	}

	void Renderer::RemoveSprite3D(Sprite3DComponent *sprite)
	{
		auto iter = std::find(mSprite3Ds.begin(), mSprite3Ds.end(), sprite);
		mSprite3Ds.erase(iter);
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
