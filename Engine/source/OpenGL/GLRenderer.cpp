#include <Engine.hpp>
#include "../../include/OpenGL/GLRenderer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <SDL_image.h>
#include <sol/sol.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/common.hpp>
#include <rapidjson/document.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

void GLRenderer::initialize(SDL_Window *window, SDL_GLContext context)
{
	mWindow = window;
	// Create an OpenGL context
	prepare();
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init("#version 130");
	if (!loadShader())
	{
		std::cout << "failed to loads shader" << std::endl;
	}
	createSpriteVerts();
}

void GLRenderer::prepare()
{
}

void GLRenderer::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Enable alpha blending on the norm buffer
	glEnable(GL_DEPTH_TEST);
	mSpriteShader->SetActive();
	glBindVertexArray(mSpriteVertexID);
	for (auto &i : mSprite3Ds)
	{
		mSpriteShader->SetMatrixUniform("uWorld", i->param.world);
		mSpriteShader->SetMatrixUniform("uProj", i->param.proj);
		mSpriteShader->SetMatrixUniform("uView", i->param.view);
		int num = 0;
		num = mTextureIDs[i->textureIndex];
		glBindTexture(GL_TEXTURE_2D, num);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

	// Set shader/vao as active
	mAlphaShader->SetActive();
	glBindVertexArray(mSpriteVertexID);
	for (auto &i : mSprite2Ds)
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
		if (i->isChangeBuffer)
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		mAlphaShader->SetMatrixUniform("uWorld", i->param.world);
		mAlphaShader->SetMatrixUniform("uProj", i->param.proj);
		mAlphaShader->SetMatrixUniform("uView", i->param.view);
		int num = mTextureIDs[i->textureIndex];
		glBindTexture(GL_TEXTURE_2D, num);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
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

	// ImGui ウィジェットを描画する.
	ImGui::Begin("Information");
	ImGui::Text("Hello,ImGui world");
	ImGui::Text("Framerate(avg) %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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
		SDL_Surface surf = texture->GetSurface();
		SDL_LockSurface(&surf);
		auto formatbuf = SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888);
		formatbuf->BytesPerPixel = 4;
		auto imagedata = SDL_ConvertSurface(&surf, formatbuf, 0);
		SDL_UnlockSurface(&surf);
		// Generate a GL texture
		GLuint textureId;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf.w, surf.h, 0, GL_RGBA,
					 GL_UNSIGNED_BYTE, imagedata->pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
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

	uint32_t indices[] =
		{
			0, 2, 1, 1, 2, 3};
	vArray.PushIndices(indices, 6);
	// Create vertex array
	glGenVertexArrays(1, &mSpriteVertexID);
	glBindVertexArray(mSpriteVertexID);

	// Create vertex buffer
	glGenBuffers(1, &mSpriteVertexID);
	glBindBuffer(GL_ARRAY_BUFFER, mSpriteVertexID);
	glBufferData(GL_ARRAY_BUFFER, 4 * 8 * sizeof(float), vArray.vertices.data(), GL_DYNAMIC_DRAW);

	// Create index buffer
	glGenBuffers(1, &mSpriteIndexID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mSpriteIndexID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), vArray.indices.data(), GL_STATIC_DRAW);

	// Specify the vertex attributes
	// (For now, assume one vertex format)
	// Position is 3 floats
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	// Normal is 3 floats
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
						  reinterpret_cast<void *>(sizeof(float) * 3));
	// Texture coordinates is 2 floats
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
						  reinterpret_cast<void *>(sizeof(float) * 6));
}
