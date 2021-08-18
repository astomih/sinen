#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <GL/glew.h>
#include "GLRenderer.h"
#include <nen.hpp>
#include <SDL.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <SDL_image.h>
#include <sol/sol.hpp>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <Effekseer.h>
#include <EffekseerRendererGL.h>
#include "EffectManagerGL.h"

namespace nen::gl
{
	GLRenderer::GLRenderer()
	{
	}

	void GLRenderer::Initialize(::SDL_Window *window)
	{
		mWindow = window;
		mContext = SDL_GL_CreateContext(mWindow);
		SDL_GL_MakeCurrent(mWindow, mContext);
		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
		{
			std::cout << "ERROR: glew isn't init" << std::endl;
		}
		glGetError();

		// Create an OpenGL context
		prepare();
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplSDL2_InitForOpenGL(window, mContext);
		ImGui_ImplOpenGL3_Init("#version 130");
		mEffectManager = std::make_unique<EffectManagerGL>(this);
		mEffectManager->Init();
	}

	void GLRenderer::Render()
	{
		auto color = mRenderer->GetClearColor();
		glClearColor(color.x, color.y, color.z, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		static int time = 0;
		for (auto i : this->mRenderer->GetEffects())
		{
			if (time % 200 == 0)
			{
				auto eref = mEffectManager->GetEffect(i->GetPath());
				auto p = i->GetPosition();
				i->handle = mEffectManager->GetManager()->Play(eref, p.x, p.y, p.z);
			}
			if (time % 200 == 199)
				mEffectManager->GetManager()->StopEffect(i->handle);
		}
		time++;
		// Move the effect
		//manager->AddLocation(handle, ::Effekseer::Vector3D(0.2f, 0.0f, 0.0f));

		// Update the manager
		mEffectManager->GetManager()->Update();

		// Begin to rendering effects
		mEffectManager->GetRenderer()->BeginRendering();

		// Render effects
		mEffectManager->GetManager()->Draw();

		// Finish to rendering effects
		mEffectManager->GetRenderer()->EndRendering();

		mSpriteShader->SetActive();
		// Specify the vertex attributes
		// (For now, assume one vertex format)
		// Position is 3 floats
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), 0);
		// Normal is 3 floats
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float),
							  reinterpret_cast<void *>(sizeof(float) * 3));
		// Texture coordinates is 2 floats
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, 8 * sizeof(float),
							  reinterpret_cast<void *>(sizeof(float) * 6));
		// Enable alpha blending on the norm buffer
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		std::string vertexID;
		bool lastFrameChanged = false;
		for (auto &i : mSprite3Ds)
		{
			if (vertexID != i->vertexIndex)
			{
				glBindVertexArray(m_VertexArrays[i->vertexIndex].vertexID);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VertexArrays[i->vertexIndex].indexID);
				auto vArraySize = m_VertexArrays[i->vertexIndex].vertices.size() * sizeof(Vertex);
				glBufferSubData(GL_ARRAY_BUFFER, 0, vArraySize, m_VertexArrays[i->vertexIndex].vertices.data());

				vertexID = i->vertexIndex;
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), 0);
				// Normal is 3 floats
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float),
									  reinterpret_cast<void *>(sizeof(float) * 3));
				// Texture coordinates is 2 floats
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, 8 * sizeof(float),
									  reinterpret_cast<void *>(sizeof(float) * 6));
			}
			if ((vertexID != i->vertexIndex && i->isChangeBuffer) || (i->isChangeBuffer ^ lastFrameChanged))
			{
				const float value = 1.f;
				const Vector2f lb(i->trimStart.x, i->trimEnd.y);
				const Vector2f lt(i->trimStart.x, i->trimStart.y);
				const Vector2f rb(i->trimEnd.x, i->trimEnd.y);
				const Vector2f rt(i->trimEnd.x, i->trimStart.y);
				std::array<float, 3> norm = {1, 1, 1};

				Vertex vertices[] =
					{
						{Vector3f(-value, value, value), norm, lb},
						{Vector3f(-value, -value, value), norm, lt},
						{Vector3f(value, value, value), norm, rb},
						{Vector3f(value, -value, value), norm, rt},

					};
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			}
			lastFrameChanged = i->isChangeBuffer;
			glBindTexture(GL_TEXTURE_2D, mTextureIDs[i->textureIndex]);
			mSpriteShader->SetMatrixUniform("uWorld", i->param.world);
			mSpriteShader->SetMatrixUniform("uProj", i->param.proj);
			mSpriteShader->SetMatrixUniform("uView", i->param.view);
			glDrawElementsBaseVertex(GL_TRIANGLES, m_VertexArrays[i->vertexIndex].indexCount, GL_UNSIGNED_INT, nullptr, 0);
		}
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		// Set shader/vao as active
		mAlphaShader->SetActive();
		for (auto &i : mSprite2Ds)
		{
			if (vertexID != i->vertexIndex)
			{
				glBindVertexArray(m_VertexArrays[i->vertexIndex].vertexID);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VertexArrays[i->vertexIndex].indexID);
				auto vArraySize = m_VertexArrays[i->vertexIndex].vertices.size() * sizeof(Vertex);
				glBufferSubData(GL_ARRAY_BUFFER, 0, vArraySize, m_VertexArrays[i->vertexIndex].vertices.data());

				vertexID = i->vertexIndex;
			}

			if (i->isChangeBuffer)
			{
				const float value = 1.f;
				const Vector2f lb(i->trimStart.x, i->trimEnd.y);
				const Vector2f lt(i->trimStart.x, i->trimStart.y);
				const Vector2f rb(i->trimEnd.x, i->trimEnd.y);
				const Vector2f rt(i->trimEnd.x, i->trimStart.y);
				std::array<float, 3> norm = {1, 1, 1};

				Vertex vertices[] =
					{
						{Vector3f(-value, value, value), norm, lb},
						{Vector3f(-value, -value, value), norm, lt},
						{Vector3f(value, value, value), norm, rb},
						{Vector3f(value, -value, value), norm, rt},

					};
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			}
			mAlphaShader->SetMatrixUniform("uWorld", i->param.world);
			mAlphaShader->SetMatrixUniform("uProj", i->param.proj);
			mAlphaShader->SetMatrixUniform("uView", i->param.view);
			int num = mTextureIDs[i->textureIndex];
			glBindTexture(GL_TEXTURE_2D, num);
			glDrawElementsBaseVertex(GL_TRIANGLES, m_VertexArrays[i->vertexIndex].indexCount, GL_UNSIGNED_INT, nullptr, 0);
			if (i->isChangeBuffer)
			{
				const float value = 1.f;
				const Vector2f lb(0, 0);
				const Vector2f lt(0, 1);
				const Vector2f rb(1, 0);
				const Vector2f rt(1, 1);
				std::array<float, 3> norm = {1, 1, 1};

				Vertex vertices[] =
					{
						{Vector3f(-value, value, value), norm, lb},
						{Vector3f(-value, -value, value), norm, lt},
						{Vector3f(value, value, value), norm, rb},
						{Vector3f(value, -value, value), norm, rt},

					};
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			}
		}
		glDisable(GL_BLEND);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(mWindow);
		ImGui::NewFrame();

		// Draw ImGUI widgets.
		ImGui::Begin("Engine Info");
		ImGui::Text("%.1f fps", ImGui::GetIO().Framerate);
		if (ImGui::Button("toggleAPI"))
		{
			std::ofstream ofs("./api");
			ofs << "Vulkan";
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(mWindow);
	}
	void GLRenderer::AddVertexArray(const VertexArray &vArray, std::string_view name)
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
		AddVertexArray(vArrayGL, name);
		m_VertexArrays.insert(std::pair<std::string, VertexArrayForGL>(name.data(), vArrayGL));
	}
	void GLRenderer::ChangeBufferSprite(std::shared_ptr<class Sprite> sprite, const TextureType type)
	{
	}

	void GLRenderer::AddSprite2D(std::shared_ptr<class Sprite> sprite, std::shared_ptr<Texture> texture)
	{
		registerTexture(texture, TextureType::Image2D);
		pushSprite2d(sprite);
	}
	void GLRenderer::RemoveSprite2D(std::shared_ptr<class Sprite> sprite)
	{
		eraseSprite2d(sprite);
	}

	void GLRenderer::AddSprite3D(std::shared_ptr<class Sprite> sprite, std::shared_ptr<Texture> texture)
	{
		registerTexture(texture, TextureType::Image3D);
		pushSprite3d(sprite);
	}
	void GLRenderer::RemoveSprite3D(std::shared_ptr<class Sprite> sprite)
	{
		eraseSprite3d(sprite);
	}

	void GLRenderer::prepare()
	{
		if (!loadShader())
		{
			std::cout << "failed to loads shader" << std::endl;
		}
		createSpriteVerts();
		createBoxVerts();
	}

	void GLRenderer::registerTexture(std::shared_ptr<Texture> texture, const TextureType &type)
	{
		if (mTextureIDs.find(texture->id) == mTextureIDs.end())
		{
			::SDL_Surface surf = texture->GetSurface();
			::SDL_LockSurface(&surf);
			auto formatbuf = ::SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888);
			formatbuf->BytesPerPixel = 4;
			auto imagedata = ::SDL_ConvertSurface(&surf, formatbuf, 0);
			SDL_UnlockSurface(&surf);
			// Generate a GL texture
			GLuint textureId;
			glGenTextures(1, &textureId);
			glBindTexture(GL_TEXTURE_2D, textureId);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf.w, surf.h, 0, GL_RGBA,
						 GL_UNSIGNED_BYTE, imagedata->pixels);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			// Use linear filtering
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			mTextureIDs.emplace(texture->id, textureId);
		}
	}

	bool GLRenderer::loadShader()
	{
		// Create sprite shader
		mSpriteShader = new ShaderGL();
		if (!mSpriteShader->Load("Assets/Shader/GL/sprite.vert", "Assets/Shader/GL/sprite.frag"))
		{
			return false;
		}
		mAlphaShader = new ShaderGL();
		if (!mAlphaShader->Load("Assets/Shader/GL/sprite.vert", "Assets/Shader/GL/alpha.frag"))
		{
			return false;
		}
		return true;
	}

	void GLRenderer::AddVertexArray(const VertexArrayForGL &vArray, std::string_view name)
	{
	}
	void GLRenderer::createSpriteVerts()
	{
		const float value = 1.f;
		const Vector2f lb(0.0f, 0.0f);
		const Vector2f lt(0.f, 1.f);
		const Vector2f rb(1.0f, 0.0f);
		const Vector2f rt(1.0f, 1.0f);
		std::array<float, 3> norm = {1, 1, 1};
		VertexArray vArray;
		vArray.vertices.push_back({Vector3f(-value, value, value), norm, lb});
		vArray.vertices.push_back({Vector3f(-value, -value, value), norm, lt});
		vArray.vertices.push_back({Vector3f(value, value, value), norm, rb});
		vArray.vertices.push_back({Vector3f(value, -value, value), norm, rt});
		uint32_t indices[] = {
			0, 2, 1, 1, 2, 3 // front
		};
		vArray.indexCount = 6;
		vArray.PushIndices(indices, vArray.indexCount);

		AddVertexArray(vArray, "SPRITE");
	}

	void GLRenderer::createBoxVerts()
	{
		const float value = 1.f;
		const Vector2f lb(0.0f, 0.0f);
		const Vector2f lt(0.f, 1.f);
		const Vector2f rb(1.0f, 0.0f);
		const Vector2f rt(1.0f, 1.0f);
		std::array<float, 3> norm = {1, 1, 1};
		const std::array<float, 3> red{1.0f, 0.0f, 0.0f};
		const std::array<float, 3> green{0.0f, 1.0f, 0.0f};
		const std::array<float, 3> blue{0.0f, 0.0f, 1.0f};
		const std::array<float, 3> white{1.0f, 1, 1};
		const std::array<float, 3> black{0.0f, 0, 0};
		const std::array<float, 3> yellow{1.0f, 1.0f, 0.0f};
		const std::array<float, 3> magenta{1.0f, 0.0f, 1.0f};
		const std::array<float, 3> cyan{0.0f, 1.0f, 1.0f};

		VertexArray vArray;
		vArray.vertices.push_back({Vector3f(-value, value, value), yellow, lb});
		vArray.vertices.push_back({Vector3f(-value, -value, value), red, lt});
		vArray.vertices.push_back({Vector3f(value, value, value), white, rb});
		vArray.vertices.push_back({Vector3f(value, -value, value), magenta, rt});

		vArray.vertices.push_back({Vector3f(value, value, value), white, lb});
		vArray.vertices.push_back({Vector3f(value, -value, value), magenta, lt});
		vArray.vertices.push_back({Vector3f(value, value, -value), cyan, rb});
		vArray.vertices.push_back({Vector3f(value, -value, -value), blue, rt});

		vArray.vertices.push_back({Vector3f(-value, value, -value), green, lb});
		vArray.vertices.push_back({Vector3f(-value, -value, -value), black, lt});
		vArray.vertices.push_back({Vector3f(-value, value, value), yellow, rb});
		vArray.vertices.push_back({Vector3f(-value, -value, value), red, rt});

		vArray.vertices.push_back({Vector3f(value, value, -value), cyan, lb});
		vArray.vertices.push_back({Vector3f(value, -value, -value), blue, lt});
		vArray.vertices.push_back({Vector3f(-value, value, -value), green, rb});
		vArray.vertices.push_back({Vector3f(-value, -value, -value), black, rt});

		vArray.vertices.push_back({Vector3f(-value, value, -value), green, lb});
		vArray.vertices.push_back({Vector3f(-value, value, value), yellow, lt});
		vArray.vertices.push_back({Vector3f(value, value, -value), cyan, rb});
		vArray.vertices.push_back({Vector3f(value, value, value), white, rt});

		vArray.vertices.push_back({Vector3f(-value, -value, value), red, lb});
		vArray.vertices.push_back({Vector3f(-value, -value, -value), black, lt});
		vArray.vertices.push_back({Vector3f(value, -value, value), magenta, rb});
		vArray.vertices.push_back({Vector3f(value, -value, -value), blue, rt});

		uint32_t indices[] = {
			0, 2, 1, 1, 2, 3,	 // front
			4, 6, 5, 5, 6, 7,	 // right
			8, 10, 9, 9, 10, 11, // left

			12, 14, 13, 13, 14, 15, // back
			16, 18, 17, 17, 18, 19, // top
			20, 22, 21, 21, 22, 23, // bottom
		};
		vArray.indexCount = _countof(indices);

		vArray.PushIndices(indices, vArray.indexCount);
		AddVertexArray(vArray, "BOX");
	}
}
#endif