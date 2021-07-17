#include <Scene.hpp>
#include <SDL_ttf.h>
#include <algorithm>
#include <Actors.hpp>
#include <Components.hpp>
#include <iostream>
#include <Engine.hpp>
#ifndef EMSCRIPTEN
#include "imgui.h"
#include "imgui_impl_sdl.h"

#endif
namespace nen
{
	Scene::Scene()
		: mTransition(std::move(std::make_shared<Transition>()))
	{
	}

	void Scene::Initialize(std::shared_ptr<Renderer> renderer)
	{
		mRenderer = renderer;

		mAudioSystem = std::make_shared<AudioSystem>(*this);
		if (!mAudioSystem->Initialize())
		{
			SDL_Log("Failed to initialize audio system");
			mAudioSystem->Shutdown();
			mAudioSystem = nullptr;
			std::exit(1);
		}
		mInputSystem = new InputSystem();
		if (!mInputSystem->Initialize())
		{
			SDL_Log("Failed to initialize input system");
			std::exit(1);
		}
// スクリプトのインスタンスを作成
#ifndef EMSCRIPTEN
		Script::Create();
#endif
		// シーンのデータを読み込み
		LoadData();
		// デルタタイムを読み込み
		mTicksCount = SDL_GetTicks();
	}

	/*
		breaf:main loop
	*/
	void Scene::RunLoop()
	{
		// Call system events
		ProcessInput();
		//update actor, components
		UpdateScene();
		//draw sprites, meshes
		mRenderer->Draw();
	}

	void Scene::ProcessInput()
	{
		mInputSystem->PrepareForUpdate();
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
#ifndef EMSCRIPTEN
			ImGui_ImplSDL2_ProcessEvent(&event);
#endif
			switch (event.type)
			{
			case SDL_QUIT:
			{
				mIsRunning = false;
				NextScene = Scenes::None;
				mGameState = EQuit;
			}
			break;
			case SDL_KEYDOWN:
				break;
			case SDL_MOUSEBUTTONDOWN:
				break;
			default:
				break;
			}
		}

		mInputSystem->Update();
		const InputState &state = mInputSystem->GetState();

		if (state.Keyboard.GetKeyState(SDL_SCANCODE_ESCAPE) == EReleased)
		{
			mIsRunning = false;
			NextScene = Scenes::None;
			mGameState = EQuit;
		}
		SystemInput(state);

		mUpdatingActors = true;
		const Uint8 *oldstate = SDL_GetKeyboardState(NULL);
		if (mGameState == EGameplay)
		{
			for (auto actor : mActors)
			{
				if (actor->GetState() == Actor::EActive)
				{
					actor->ProcessInput(state);
				}
			}
		}
		mUpdatingActors = false;
	}

	void Scene::UpdateScene()
	{
		// calc delta time
		float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
		if (deltaTime > 0.05f)
		{
			deltaTime = 0.05f;
		}
		mTicksCount = SDL_GetTicks();

		this->Update(deltaTime);
		// All actors update
		mUpdatingActors = true;
		for (auto actor : mActors)
		{
			actor->Update(deltaTime);
		}
		mUpdatingActors = false;

		//erase dead actors
		for (auto it = mActors.begin(); it != mActors.end();)
		{
			if ((*it)->GetState() == Actor::EDead)
			{
				it = mActors.erase(it);
			}
			else
			{
				++it;
			}
		}

		//move pending actors
		for (auto pending : mPendingActors)
		{
			pending->ComputeWorldTransform();
			mActors.emplace_back(pending);
		}
		mPendingActors.clear();

		mAudioSystem->Update(deltaTime);
	}

	void Scene::LoadData()
	{
	}

	void Scene::UnloadData()
	{
		mActors.clear();
	}

	void Scene::Update(float deltaTime)
	{
	}

	Scenes Scene::Shutdown()
	{
		mInputSystem->Shutdown();
		delete mInputSystem;
		mInputSystem = nullptr;
		UnloadData();
		return NextScene;
	}

	void Scene::AddActor(std::shared_ptr<Actor> actor)
	{
		// If we're updating actors, need to add to pending
		if (mUpdatingActors)
		{
			mPendingActors.emplace_back(actor);
		}
		else
		{
			mActors.emplace_back(actor);
		}
		actor->AddedScene();
	}

	void Scene::RemoveActor(std::shared_ptr<Actor> actor)
	{
		// Is it in pending actors?
		auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
		if (iter != mPendingActors.end())
		{
			// Swap to end of vector and pop off (avoid erase copies)
			std::iter_swap(iter, mPendingActors.end() - 1);
			mPendingActors.pop_back();
		}

		// Is it in actors?
		iter = std::find(mActors.begin(), mActors.end(), actor);
		if (iter != mActors.end())
		{
			// Swap to end of vector and pop off (avoid erase copies)
			std::iter_swap(iter, mActors.end() - 1);
			mActors.pop_back();
		}
	}

	Font *Scene::GetFont(const std::string &fileName)
	{
		auto iter = mFonts.find(fileName);
		if (iter != mFonts.end())
		{
			return iter->second;
		}
		else
		{
			Font *font = new Font();
			if (font->Load(fileName))
			{
				mFonts.emplace(fileName, font);
			}
			else
			{
				font->Unload();
				delete font;
				font = nullptr;
			}
			return font;
		}
	}
}