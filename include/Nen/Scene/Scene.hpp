#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
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

		void AddActor(std::shared_ptr<class Actor> actor);
		void RemoveActor(std::shared_ptr<class Actor> actor);

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
		Uint32 mTicksCount = 0;
		Uint32 mTimeBuf = 0;
		// Track if we're updating actors right now
		bool mUpdatingActors = false;
		bool mIsRunning = true;
		// Map for text localization
		std::unordered_map<std::string, std::string> mText;
		GameState mGameState = GameState::EGameplay;
	};
}