#include <SDL.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <iostream>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "../Render/RendererHandle.hpp"
#include <Nen.hpp>

namespace nen
{
	Scene::Scene()
	{
	}

	void Scene::Initialize()
	{
		mRenderer = RendererHandle::GetRenderer();
		// シーンのデータを読み込み
		Setup();
		Logger::Info("Scene setup.");
		// デルタタイムを読み込み
		mTicksCount = SDL_GetTicks();
	}

	/*
		メインループ
	*/
	void Scene::RunLoop()
	{
		ProcessInput();
		UpdateScene();
		//draw sprites, meshes
		mRenderer->Draw();
		mInputSystem->PrepareForUpdate();
		mInputSystem->Update();
	}

	void Scene::ProcessInput()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			GetRenderer()->GetWindow()->ProcessInput(event);
			switch (event.type)
			{
			case SDL_QUIT:
			{
				mGameState = GameState::Quit;
			}
			break;
			default:
				break;
			}
		}

		const InputState &state = mInputSystem->GetState();

		if (state.Keyboard.GetKeyState(KeyCode::ESCAPE) == ButtonState::Released)
		{
			mGameState = GameState::Quit;
		}
		if (state.Keyboard.GetKeyState(KeyCode::F3) == ButtonState::Pressed)
		{
			GetRenderer()->toggleShowImGui();
		}
		if (mGameState == GameState::Quit)
			return;
		mUpdatingActors = true;

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
			actor->UpdateActor(deltaTime);
		}
		mUpdatingActors = false;

		//erase dead actors
		for (auto it = mActors.begin(); it != mActors.end();)
		{
			if ((*it)->GetState() == Actor::State::Dead)
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

		mSoundSystem->Update(deltaTime);
	}

	void Scene::Setup()
	{
	}

	void Scene::UnloadData()
	{
		mActors.clear();
	}

	void Scene::Update(float deltaTime)
	{
	}

	void Scene::Shutdown()
	{
		UnloadData();
	}

	void Scene::AddGUI(std::shared_ptr<UIScreen> ui)
	{
		GetRenderer()->AddGUI(ui);
	}
	void Scene::RemoveGUI(std::shared_ptr<UIScreen> ui)
	{
		GetRenderer()->RemoveGUI(ui);
	}

}
