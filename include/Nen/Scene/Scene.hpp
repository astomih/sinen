#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
namespace nen
{
	class AudioSystem;
	class Scene : public std::enable_shared_from_this<Scene>
	{
	public:
		Scene();
		virtual ~Scene() {}

		void Initialize();

		void SetRenderer(std::shared_ptr<class Renderer> renderer) { mRenderer = renderer; }

		bool isRunning() { return mIsRunning; }

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
			actor->AddedScene();
			return actor;
		}

		template <class T>
		std::shared_ptr<T> GetActor(uint32_t handle = 0)
		{
			for (auto i : mActors)
			{
				auto actor = std::dynamic_pointer_cast<T>(i);
				if (actor != nullptr && i->handle)
					return actor;
			}
			for (auto i : mPendingActors)
			{
				auto actor = std::dynamic_pointer_cast<T>(i);
				if (actor != nullptr && i->handle)
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

		std::shared_ptr<AudioSystem> GetAudioSystem() const { return mAudioSystem; }

		std::shared_ptr<class Renderer> GetRenderer() const { return mRenderer; }

		enum GameState
		{
			EGameplay,
			EPaused,
			EQuit
		};

		GameState GetState() const { return mGameState; }
		void SetState(GameState state) { mGameState = state; }

		void Quit()
		{
			mIsRunning = false;
			mGameState = EQuit;
		}
		void ExitScene() { mIsRunning = false; }

		void AddGUI(std::shared_ptr<class UIScreen> ui);
		void RemoveGUI(std::shared_ptr<class UIScreen> ui);

	protected:
		virtual void Setup();
		virtual void Update(float deltaTime);
		virtual void SystemInput(const struct InputState &) {}

		// All the actors in the game
		std::vector<std::shared_ptr<class Actor>> mActors;
		std::shared_ptr<class AudioSystem> mAudioSystem;

	private:
		void UnloadData();
		void ProcessInput();
		void UpdateScene();
		class InputSystem *mInputSystem;
		std::shared_ptr<class Renderer> mRenderer;
		std::unordered_map<std::string, class Font *> mFonts;
		// Any pending actors
		std::vector<std::shared_ptr<class Actor>> mPendingActors;
		uint32_t mTicksCount = 0;
		uint32_t mTimeBuf = 0;
		// Track if we're updating actors right now
		bool mUpdatingActors = false;
		bool mIsRunning = true;
		// Map for text localization
		std::unordered_map<std::string, std::string> mText;
		GameState mGameState = GameState::EGameplay;
	};
	void ChangeScene(std::shared_ptr<Scene> newScene);
}