#include <core/data/string.hpp>
#include <graphics/drawable/drawable.hpp>

#include <list>
#include <vector>

namespace sinen {
class MainSystem {
public:
  enum class State { running, paused, quit };
  MainSystem() = default;
  ~MainSystem() = default;
  /**
   * @brief Initialize scene system
   *
   * @return true success
   * @return false failed
   */
  static bool initialize(int argc, char *argv[]);
  /**
   * @brief Terminate scene system
   *
   */
  static void shutdown();
  static void setup();
  /**
   * @brief Set the state object
   *
   * @param state
   */
  static void setState(const State &state) { mGameState = state; }
  /**
   * @brief Get the state object
   *
   * @return const scene::state&
   */
  static const State &getState() { return mGameState; }
  static void setRunScript(bool is_run) { isRunScript = is_run; }

  static bool isRunning() { return mGameState != State::quit; }
  static bool isResetNext() { return isReset; }
  static String getCurrentName() { return mSceneName; }
  static String getBasePath() { return basePath; }
  static void change(StringView sceneFileName, StringView basePath);

  static void processInput();
  static void updateScene();
  static bool isReset;
  static bool isRunScript;

  static inline float deltaTime() { return mDeltaTime; }

  static inline int argc = 0;
  static inline std::vector<std::string> argv;

private:
  static State mGameState;
  static uint32_t mPrevTick;
  static String mSceneName;
  static String basePath;
  static float mDeltaTime;
};
} // namespace sinen
