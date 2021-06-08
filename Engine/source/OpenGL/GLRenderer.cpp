#include <Engine.hpp>
#include "../../include/OpenGL/GLRenderer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <SDL_image.h>
#include <sol/sol.hpp>
#include <rapidjson/document.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

namespace nen::gl
{
	void GLRenderer::initialize(::SDL_Window *window, ::SDL_GLContext context)
	{
		mWindow = window;
		// Create an OpenGL context
		prepare();
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplSDL2_InitForOpenGL(window, context);
		ImGui_ImplOpenGL3_Init("#version 130");
	}

	void GLRenderer::prepare()
	{
		if (!loadShader())
		{
			std::cout << "failed to loads shader" << std::endl;
		}
		createBoxVerts();
		createSpriteVerts();
	}

	void GLRenderer::render()
	{
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
		for (auto &i : mSprite3Ds)
		{
			if (vertexID != i->vertexIndex)
			{
				glBindVertexArray(m_VertexArrays[i->vertexIndex].vertexID);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VertexArrays[i->vertexIndex].indexID);
				auto vArraySize = m_VertexArrays[i->vertexIndex].vertices.size() * sizeof(Vertex);
				glBufferSubData(GL_ARRAY_BUFFER, 0, vArraySize, m_VertexArrays[i->vertexIndex].vertices.data());

				vertexID = i->vertexIndex;
			}
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
		ImGui::Begin("NenEngine Info");
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
		if (!mSpriteShader->Load("sprite.vert", "sprite.frag"))
		{
			return false;
		}
		mAlphaShader = new ShaderGL();
		if (!mAlphaShader->Load("sprite.vert", "alpha.frag"))
		{
			return false;
		}
		return true;
	}

	void GLRenderer::AddVertexArray(const VertexArrayForGL &vArray, std::string_view name)
	{
		m_VertexArrays.insert(std::pair<std::string, VertexArrayForGL>(name.data(), vArray));
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

		mRenderer->AddVertexArray(vArray, "SPRITE");
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

		VertexArrayForGL vArray;
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
		vArray.indexCount = 36;
		vArray.PushIndices(indices, vArray.indexCount);
		mRenderer->AddVertexArray(vArray, "BOX");
	}
}