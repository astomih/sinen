#ifdef EMSCRIPTEN
#include <Engine.hpp>
#include "../../include/ES/ESRenderer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <SDL_image.h>
#include <Effekseer.h>
#include <EffekseerRendererGL.h>
#include <Engine/include/Effect.hpp>

#define _countof(array) (sizeof(array) / sizeof(array[0]))

namespace nen
{
	namespace es
	{
		void ESRenderer::initialize(::SDL_Window *window, ::SDL_GLContext context)
		{
			mWindow = window;
			// Create an OpenGL context
			prepare();
		}

		void ESRenderer::prepare()
		{
			if (!loadShader())
			{
				std::cout << "failed to loads shader" << std::endl;
			}
			createSpriteVerts();
			createBoxVerts();
		}

		void ESRenderer::render()
		{
			auto color = mRenderer->GetClearColor();
			glClearColor(color.x, color.y, color.z, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			static int time = 0;
			if (time % 120 == 0)
			{
				// Play an effect
				mEffectManager->handle = mEffectManager->GetManager()->Play(mEffectManager->GetEffect(u"Laser01.efk"), 0, 0, 0);
				time = 0;
			}

			if (time % 120 == 119)
			{
				// Stop effects
				mEffectManager->GetManager()->StopEffect(mEffectManager->handle);
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
					glBindVertexArrayOES(m_VertexArrays[i->vertexIndex].vertexID);
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
				glDrawElements(GL_TRIANGLES, m_VertexArrays[i->vertexIndex].indexCount, GL_UNSIGNED_INT, nullptr);
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
					glBindVertexArrayOES(m_VertexArrays[i->vertexIndex].vertexID);
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
				glDrawElements(GL_TRIANGLES, m_VertexArrays[i->vertexIndex].indexCount, GL_UNSIGNED_INT, nullptr);
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
			SDL_GL_SwapWindow(mWindow);
		}

		void ESRenderer::registerTexture(std::shared_ptr<Texture> texture, const TextureType &type)
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

		bool ESRenderer::loadShader()
		{
			// Create sprite shader
			mSpriteShader = new gl::ShaderGL();
			if (!mSpriteShader->Load("sprite.vert", "sprite.frag"))
			{
				return false;
			}
			mAlphaShader = new gl::ShaderGL();
			if (!mAlphaShader->Load("sprite.vert", "alpha.frag"))
			{
				return false;
			}
			return true;
		}

		void ESRenderer::AddVertexArray(const gl::VertexArrayForGL &vArray, std::string_view name)
		{
			m_VertexArrays.insert(std::pair<std::string, gl::VertexArrayForGL>(name.data(), vArray));
		}
		void ESRenderer::createSpriteVerts()
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

		void ESRenderer::createBoxVerts()
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
			mRenderer->AddVertexArray(vArray, "BOX");
		}
	}
}
#endif
