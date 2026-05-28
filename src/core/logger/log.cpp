#include <SDL3/SDL.h>
#include <core/logger/log.hpp>
namespace sinen {
void Log::Impl::verbose(StringView str) {
  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "%s", str.data());
}

void Log::Impl::debug(StringView str) {
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s", str.data());
}

void Log::Impl::info(StringView str) {
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", str.data());
}

void Log::Impl::error(StringView str) {
  SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s", str.data());
}

void Log::Impl::warn(StringView str) {
  SDL_LogWarn(SDL_LOG_CATEGORY_ASSERT, "%s", str.data());
}

void Log::Impl::critical(StringView str) {
  SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "%s", str.data());
}

} // namespace sinen
