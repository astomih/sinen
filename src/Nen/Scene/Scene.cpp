#include <Nen.hpp>
#include <SDL_ttf.h>
#include <algorithm>
#include <iostream>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "../Render/RendererHandle.hpp"

namespace nen
{
	Scene::Scene()
	{
	}

	void Scene::Initialize()
	{
		mRenderer = RendererHandle::GetRenderer();
		mAudioSystem = std::make_shared<AudioSystem>(*this);
		if (!mAudioSystem->Initialize())
		{
			Logger::Info("Failed to initialize audio system");
			mAudioSystem->Shutdown();
			mAudioSystem = nullptr;
			std::exit(1);
		}
		Logger::Info("Audio system Initialized.");
		mInputSystem = new InputSystem();
		if (!mInputSystem->Initialize())
		{
			Logger::Info("Failed to initialize input system");
		}
		Logger::Info("Input system initialized.");
		// スクリプトのインスタンスを作成
		Script::Create();
		Logger::Info("Script system initialized.");
		// シーンのデータを読み込み
		LoadData();
		Logger::Info("Scene data loaded.");
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
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
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

		const InputState &state = mInputSystem->GetState();

		if (state.Keyboard.GetKeyState(SDL_SCANCODE_ESCAPE) == EReleased)
		{
			mIsRunning = false;
			NextScene = Scenes::None;
			mGameState = EQuit;
		}
		SystemInput(state);
		mUpdatingActors = true;
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

		mInputSystem->PrepareForUpdate();
		mInputSystem->Update();
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
