#include <SDL.h>
#include <logger/logger.hpp>

namespace sinen {
logger::implements logger::m_logger;
void logger::implements::verbose(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, format, args);
}

void logger::implements::debug(const char *format, ...) {
  va_list args;
  va_start(args, format);
  va_end(args);
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, format, args);
}

void logger::implements::info(const char *format, ...) {
  va_list args;
  va_start(args, format);
  va_end(args);
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, format, args);
}

void logger::implements::error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  va_end(args);
  SDL_LogError(SDL_LOG_CATEGORY_ERROR, format, args);
}

void logger::implements::warn(const char *format, ...) {
  va_list args;
  va_start(args, format);
  va_end(args);
  SDL_LogWarn(SDL_LOG_CATEGORY_ASSERT, format, args);
}

void logger::implements::critical(const char *format, ...) {
  va_list args;
  va_start(args, format);
  va_end(args);
  SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, format, args);
}

static std::function<void(logger::priority, std::string_view)> g_func;
void SDL_LogOutputFunction(void *userdata, int category,
                           SDL_LogPriority priority, const char *message) {
  g_func(static_cast<logger::priority>(priority), message);
}
void logger::set_output_function(
    std::function<void(priority, std::string_view)> func) {
  g_func = func;
  SDL_LogSetOutputFunction(SDL_LogOutputFunction, nullptr);
}
} // namespace sinen
