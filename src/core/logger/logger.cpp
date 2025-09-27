#include <SDL3/SDL.h>
#include <core/logger/logger.hpp>

namespace sinen {
Logger::Implements Logger::logger;
void Logger::Implements::verbose(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, format, args);
}

void Logger::Implements::debug(const char *format, ...) {
  va_list args;
  va_start(args, format);
  va_end(args);
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, format, args);
}

void Logger::Implements::info(const char *format, ...) {
  va_list args;
  va_start(args, format);
  va_end(args);
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, format, args);
}

void Logger::Implements::error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  va_end(args);
  SDL_LogError(SDL_LOG_CATEGORY_ERROR, format, args);
}

void Logger::Implements::warn(const char *format, ...) {
  va_list args;
  va_start(args, format);
  va_end(args);
  SDL_LogWarn(SDL_LOG_CATEGORY_ASSERT, format, args);
}

void Logger::Implements::critical(const char *format, ...) {
  va_list args;
  va_start(args, format);
  va_end(args);
  SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, format, args);
}

static std::function<void(Logger::priority, std::string_view)> g_func;
void SDL_LogOutputFunction(void *userdata, int category,
                           SDL_LogPriority priority, const char *message) {
  g_func(static_cast<Logger::priority>(priority), message);
}
void Logger::setOutputFunction(
    std::function<void(priority, std::string_view)> func) {
  g_func = func;
  SDL_SetLogOutputFunction(SDL_LogOutputFunction, nullptr);
}
} // namespace sinen
