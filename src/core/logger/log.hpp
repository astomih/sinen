#ifndef SINEN_LOG
#define SINEN_LOG
#include <core/allocator/global_allocator.hpp>
#include <core/data/string.hpp>
#include <format>

namespace sinen {

class Log {
public:
  static void verbose(StringView msg);
  static void debug(StringView msg);
  static void info(StringView msg);
  static void error(StringView msg);
  static void warn(StringView msg);
  static void critical(StringView msg);
};
class LogF {
public:
  template <class... Args>
  static void verbose(std::format_string<Args...> fmt, Args &&...args) {
    String msg;
    std::format_to(std::back_inserter(msg), fmt, std::forward<Args>(args)...);
    Log::verbose(msg);
  }
  template <class... Args>
  static void debug(std::format_string<Args...> fmt, Args &&...args) {
    String msg;
    std::format_to(std::back_inserter(msg), fmt, std::forward<Args>(args)...);
    Log::debug(msg);
  }
  template <class... Args>
  static void info(std::format_string<Args...> fmt, Args &&...args) {
    String msg;
    std::format_to(std::back_inserter(msg), fmt, std::forward<Args>(args)...);
    Log::info(msg);
  }
  template <class... Args>
  static void error(std::format_string<Args...> fmt, Args &&...args) {
    String msg;
    std::format_to(std::back_inserter(msg), fmt, std::forward<Args>(args)...);
    Log::error(msg);
  }
  template <class... Args>
  static void warn(std::format_string<Args...> fmt, Args &&...args) {
    String msg;
    std::format_to(std::back_inserter(msg), fmt, std::forward<Args>(args)...);
    Log::warn(msg);
  }
  template <class... Args>
  static void critical(std::format_string<Args...> fmt, Args &&...args) {
    String msg;
    std::format_to(std::back_inserter(msg), fmt, std::forward<Args>(args)...);
    Log::critical(msg);
  }
};
} // namespace sinen

#endif // SINEN_LOGGER
