#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <Effekseer.h>
#include <EffekseerRendererGL.h>
#include "GLRenderer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "EffectManagerGL.h"
#include "../../Texture/SurfaceHandle.hpp"
#include <Nen.hpp>

namespace nen::gl
{
	GLRenderer::GLRenderer()
	{
	}

	void GLRenderer::Initialize(std::shared_ptr<Window> window)
	{
		mWindow = window;
		mContext = SDL_GL_CreateContext(mWindow->GetSDLWindow());
		SDL_GL_MakeCurrent(mWindow->GetSDLWindow(), mContext);
		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
		{
			Logger::Error("GLEW Init error.");
		}
		glGetError();

		mEffectManager = std::make_unique<EffectManagerGL>(this);
		mEffectManager->Init();
		prepare();
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplSDL2_InitForOpenGL(window->GetSDLWindow(), mContext);
		ImGui_ImplOpenGL3_Init("#version 130");
	}
	void GLRenderer::SetRenderer(Renderer *renderer)
	{
		mRenderer = renderer;
	}

	void GLRenderer::Render()
	{
		auto color = mRenderer->GetClearColor();
		glClearColor(color.r, color.g, color.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mSpriteShader->SetActive(0);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		std::string vertexID;
		bool lastFrameChanged = false;
		for (auto &i : mSprite3Ds)
		{
			if (vertexID != i->vertexIndex)
			{
				glBindVertexArray(m_VertexArrays[i->vertexIndex].vao);
				vertexID = i->vertexIndex;
			}
			glBindTexture(GL_TEXTURE_2D, mTextureIDs[i->textureIndex]);
			mSpriteShader->UpdateUBO(0, sizeof(ShaderParameters), &i->param);
			glDrawElementsBaseVertex(GL_TRIANGLES, m_VertexArrays[i->vertexIndex].indexCount, GL_UNSIGNED_INT, nullptr, 0);
		}
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		mAlphaShader->SetActive(0);
		for (auto &i : mSprite2Ds)
		{
			if (vertexID != i->vertexIndex)
			{
				glBindVertexArray(m_VertexArrays[i->vertexIndex].vao);
				vertexID = i->vertexIndex;
			}
			mAlphaShader->UpdateUBO(0, sizeof(ShaderParameters), &i->param);
			glBindTexture(GL_TEXTURE_2D, mTextureIDs[i->textureIndex]);
			glDrawElementsBaseVertex(GL_TRIANGLES, m_VertexArrays[i->vertexIndex].indexCount, GL_UNSIGNED_INT, nullptr, 0);
		}
		glDisable(GL_BLEND);
		Effekseer::Matrix44 mat;

		auto nenm = mRenderer->GetViewMatrix();
		memcpy(&mat, &nenm, sizeof(float) * 16);

		mEffectManager->GetRenderer()->SetCameraMatrix(mat);

		for (auto i : this->mRenderer->GetEffects())
		{
			auto eref = mEffectManager->GetEffect(i->GetPath());
			auto p = i->GetPosition();
			if (i->isLoop())
			{
				if (i->GetTimer().isStarted())
				{
					if (i->GetTimer().Check())
					{
						mEffectManager->GetManager()->StopEffect(i->handle);
						i->handle = mEffectManager->GetManager()->Play(eref, p.x, p.y, p.z);
						i->GetTimer().Stop();
					}
				}
				else
				{
					i->GetTimer().Start();
					if (i->first)
					{
						mEffectManager->GetManager()->StopEffect(i->handle);
						i->handle = mEffectManager->GetManager()->Play(eref, p.x, p.y, p.z);
						i->first = false;
					}
				}
			}
			else
			{
				if (i->first)
				{
					mEffectManager->GetManager()->StopEffect(i->handle);
					i->handle = mEffectManager->GetManager()->Play(eref, p.x, p.y, p.z);
					i->first = false;
				}
			}
			auto euler = Quaternion::ToEuler(i->GetRotation());
			mEffectManager->GetManager()->SetRotation(i->handle, euler.x, euler.y, euler.z);
			mEffectManager->GetManager()->SetLocation(i->handle, ::Effekseer::Vector3D(p.x, p.y, p.z));
		}
		mEffectManager->GetManager()->Update();
		mEffectManager->GetRenderer()->BeginRendering();
		mEffectManager->GetManager()->Draw();
		mEffectManager->GetRenderer()->EndRendering();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(mWindow->GetSDLWindow());
		ImGui::NewFrame();
		if (mRenderer->isShowImGui())
		{

			// Draw ImGUI widgets.
			ImGui::Begin("Engine Info");
			ImGui::Text("%.1f fps", ImGui::GetIO().Framerate);
			if (ImGui::Button("toggleAPI"))
			{
				std::ofstream ofs("./api");
				ofs << "Vulkan";
			}
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(mWindow->GetSDLWindow());
	}
	void GLRenderer::AddVertexArray(const VertexArray &vArray, std::string_view name)
	{
		gl::VertexArrayForGL vArrayGL;
		vArrayGL.indexCount = vArray.indexCount;
		vArrayGL.indices = vArray.indices;
		vArrayGL.vertices = vArray.vertices;
		vArrayGL.materialName = vArray.materialName;

		// VAOを作成
		glGenVertexArrays(1, &vArrayGL.vao);
		glBindVertexArray(vArrayGL.vao);

		// VBOを作成
		glGenBuffers(1, &vArrayGL.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vArrayGL.vbo);
		auto vArraySize = vArrayGL.vertices.size() * sizeof(Vertex);
		glBufferData(GL_ARRAY_BUFFER, vArraySize, vArrayGL.vertices.data(), GL_STATIC_DRAW);

		// VBOをVAOに登録
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float),
							  reinterpret_cast<void *>(sizeof(float) * 3));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, 8 * sizeof(float),
							  reinterpret_cast<void *>(sizeof(float) * 6));
		// IBOを作成
		glGenBuffers(1, &vArrayGL.ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vArrayGL.ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, vArrayGL.indices.size() * sizeof(uint32_t), vArrayGL.indices.data(), GL_STATIC_DRAW);
		// 登録
		m_VertexArrays.emplace(name.data(), vArrayGL);
	}
	void GLRenderer::ChangeBufferDrawObject(std::shared_ptr<class DrawObject> sprite, const TextureType type)
	{
	}

	void GLRenderer::AddDrawObject2D(std::shared_ptr<class DrawObject> sprite, std::shared_ptr<Texture> texture)
	{
		registerTexture(texture, TextureType::Image2D);
		pushSprite2d(sprite);
	}
	void GLRenderer::RemoveDrawObject2D(std::shared_ptr<class DrawObject> sprite)
	{
		eraseSprite2d(sprite);
	}

	void GLRenderer::AddDrawObject3D(std::shared_ptr<class DrawObject> sprite, std::shared_ptr<Texture> texture)
	{
		registerTexture(texture, TextureType::Image3D);
		pushSprite3d(sprite);
	}
	void GLRenderer::RemoveDrawObject3D(std::shared_ptr<class DrawObject> sprite)
	{
		eraseSprite3d(sprite);
	}

	void GLRenderer::LoadEffect(std::shared_ptr<Effect> effect)
	{
		mEffectManager->GetEffect(effect->GetPath());
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
		if (mTextureIDs.contains(texture->id))
			return;

		::SDL_Surface surf = SurfaceHandle::Load(texture->id);
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

	bool GLRenderer::loadShader()
	{
		mSpriteShader = new ShaderGL();
		if (!mSpriteShader->Load("Assets/Shader/GL/sprite.vert", "Assets/Shader/GL/sprite.frag"))
		{
			return false;
		}

		ShaderParameters param{};
		mSpriteShader->CreateUBO(0, sizeof(ShaderParameters), &param);
		mAlphaShader = new ShaderGL();
		if (!mAlphaShader->Load("Assets/Shader/GL/sprite.vert", "Assets/Shader/GL/alpha.frag"))
		{
			return false;
		}
		mAlphaShader->CreateUBO(0, sizeof(ShaderParameters), &param);
		return true;
	}

	void GLRenderer::createSpriteVerts()
	{
		const float value = 1.f;
		const Vector2 lb(0.0f, 0.0f);
		const Vector2 lt(0.f, 1.f);
		const Vector2 rb(1.0f, 0.0f);
		const Vector2 rt(1.0f, 1.0f);
		std::array<float, 3> norm = {1, 1, 1};
		VertexArray vArray;
		vArray.vertices.push_back({Vector3(-value, value, value), norm, lb});
		vArray.vertices.push_back({Vector3(-value, -value, value), norm, lt});
		vArray.vertices.push_back({Vector3(value, value, value), norm, rb});
		vArray.vertices.push_back({Vector3(value, -value, value), norm, rt});
		uint32_t indices[] =
			{
				0, 2, 1, 1, 2, 3 // front
			};
		vArray.indexCount = 6;
		vArray.PushIndices(indices, vArray.indexCount);

		AddVertexArray(vArray, "SPRITE");
	}

	void GLRenderer::createBoxVerts()
	{
		const float value = 1.f;
		const Vector2 lb(0.0f, 0.0f);
		const Vector2 lt(0.f, 1.f);
		const Vector2 rb(1.0f, 0.0f);
		const Vector2 rt(1.0f, 1.0f);
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
		vArray.vertices.push_back({Vector3(-value, value, value), yellow, lb});
		vArray.vertices.push_back({Vector3(-value, -value, value), red, lt});
		vArray.vertices.push_back({Vector3(value, value, value), white, rb});
		vArray.vertices.push_back({Vector3(value, -value, value), magenta, rt});

		vArray.vertices.push_back({Vector3(value, value, value), white, lb});
		vArray.vertices.push_back({Vector3(value, -value, value), magenta, lt});
		vArray.vertices.push_back({Vector3(value, value, -value), cyan, rb});
		vArray.vertices.push_back({Vector3(value, -value, -value), blue, rt});

		vArray.vertices.push_back({Vector3(-value, value, -value), green, lb});
		vArray.vertices.push_back({Vector3(-value, -value, -value), black, lt});
		vArray.vertices.push_back({Vector3(-value, value, value), yellow, rb});
		vArray.vertices.push_back({Vector3(-value, -value, value), red, rt});

		vArray.vertices.push_back({Vector3(value, value, -value), cyan, lb});
		vArray.vertices.push_back({Vector3(value, -value, -value), blue, lt});
		vArray.vertices.push_back({Vector3(-value, value, -value), green, rb});
		vArray.vertices.push_back({Vector3(-value, -value, -value), black, rt});

		vArray.vertices.push_back({Vector3(-value, value, -value), green, lb});
		vArray.vertices.push_back({Vector3(-value, value, value), yellow, lt});
		vArray.vertices.push_back({Vector3(value, value, -value), cyan, rb});
		vArray.vertices.push_back({Vector3(value, value, value), white, rt});

		vArray.vertices.push_back({Vector3(-value, -value, value), red, lb});
		vArray.vertices.push_back({Vector3(-value, -value, -value), black, lt});
		vArray.vertices.push_back({Vector3(value, -value, value), magenta, rb});
		vArray.vertices.push_back({Vector3(value, -value, -value), blue, rt});

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