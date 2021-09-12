#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include "../Input/InputSystem.hpp"
namespace nen
{
	class SoundSystem;
	class Scene
	{
	public:
		Scene();
		virtual ~Scene() {}

		enum class GameState
		{
			Gameplay,
			Paused,
			Quit
		};

		void Initialize();

		void SetRenderer(std::shared_ptr<class Renderer> renderer) { mRenderer = renderer; }

		bool isRunning() { return mGameState != GameState::Quit; }

		void RunLoop();

		void Shutdown();

		template <class T>
		std::shared_ptr<T> AddActor()
		{
			auto actor = std::make_shared<T>(*this);
			if (mUpdatingActors)
			{
				mPendingActors.emplace_back(actor);
			}
			else
			{
				mActors.emplace_back(actor);
			}
			return actor;
		}

		template <class T>
		std::shared_ptr<T> GetActor(uint32_t handle = 0)
		{
			for (const auto &i : mActors)
			{
				auto actor = std::dynamic_pointer_cast<T>(i);
				if (actor && i->handle == handle)
					return actor;
			}
			for (const auto &i : mPendingActors)
			{
				auto actor = std::dynamic_pointer_cast<T>(i);
				if (actor && i->handle == handle)
					return actor;
			}
			return nullptr;
		}

		template <class T>
		void RemoveActor(std::shared_ptr<T> actor)
		{
			auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
			if (iter != mPendingActors.end())
			{
				std::iter_swap(iter, mPendingActors.end() - 1);
				mPendingActors.pop_back();
			}

			iter = std::find(mActors.begin(), mActors.end(), actor);
			if (iter != mActors.end())
			{
				std::iter_swap(iter, mActors.end() - 1);
				mActors.pop_back();
			}
		}

		std::shared_ptr<SoundSystem> GetAudioSystem() const { return mAudioSystem; }

		std::shared_ptr<class Renderer> GetRenderer() const { return mRenderer; }

		GameState GetState() const { return mGameState; }
		void SetState(GameState state) { mGameState = state; }

		void Quit()
		{
			mGameState = GameState::Quit;
		}

		void AddGUI(std::shared_ptr<class UIScreen> ui);
		void RemoveGUI(std::shared_ptr<class UIScreen> ui);

		const InputState &GetInput() { return mInputSystem->GetState(); }
		const InputSystem &GetInputSystem() { return *mInputSystem; }

	protected:
		virtual void Setup();
		virtual void Update(float deltaTime);

		// All the actors in the game
		std::vector<std::shared_ptr<class Actor>> mActors;
		std::shared_ptr<class SoundSystem> mAudioSystem;

	private:
		void UnloadData();
		void ProcessInput();
		void UpdateScene();
		class InputSystem *mInputSystem;
		std::shared_ptr<class Renderer> mRenderer;
		std::vector<std::shared_ptr<class Actor>> mPendingActors;
		GameState mGameState = GameState::Gameplay;
		uint32_t mTicksCount = 0;
		bool mUpdatingActors = false;
	};
	void ChangeScene(std::shared_ptr<Scene> newScene);
}