#include <core/def/assert.hpp>

#include <SDL3/SDL_assert.h>

namespace sinen {
void Assert::report(bool condition, const char *cond) noexcept {
  do {
    while (!(condition)) {
      SDL_AssertData sdlAssertData = {false, 0, cond, NULL, 0, NULL, NULL};
      const SDL_AssertState sdlAssertState = SDL_ReportAssertion(
          &sdlAssertData, SDL_FUNCTION, SDL_ASSERT_FILE, SDL_LINE);
      if (sdlAssertState == SDL_ASSERTION_RETRY) {
        continue; /* go again. */
      } else if (sdlAssertState == SDL_ASSERTION_BREAK) {
        SDL_AssertBreakpoint();
      }
      break; /* not retrying. */
    }
  } while (SDL_NULL_WHILE_LOOP_CONDITION);
}
} // namespace sinen