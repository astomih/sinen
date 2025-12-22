#ifndef SINEN_LOGGER
#define SINEN_LOGGER
#include <core/data/string.hpp>
#include <functional>
#include <memory>

namespace sinen {

class Logger {
private:
public:
  enum class priority : int { verbose = 2, debug, info, warn, error, critical };
  static void setOutputFunction(std::function<void(priority, StringView)>);
  template <typename... Args>
  static void verbose(StringView format, Args &&...args) {
    logger.verbose(
        stringFormatLogger(String(format), std::forward<Args>(args)...)
            .c_str());
  }
  template <typename... Args>
  static void debug(StringView format, Args &&...args) {
    logger.debug(stringFormatLogger(String(format), std::forward<Args>(args)...)
                     .c_str());
  }
  template <typename... Args>
  static void info(StringView format, Args &&...args) {
    logger.info(stringFormatLogger(String(format), std::forward<Args>(args)...)
                    .c_str());
  }
  template <typename... Args>
  static void error(StringView format, Args &&...args) {
    logger.error(stringFormatLogger(String(format), std::forward<Args>(args)...)
                     .c_str());
  }
  template <typename... Args>
  static void warn(StringView format, Args &&...args) {
    logger.warn(stringFormatLogger(String(format), std::forward<Args>(args)...)
                    .c_str());
  }
  template <typename... Args>
  static void critical(StringView format, Args &&...args) {
    logger.critical(
        stringFormatLogger(String(format), std::forward<Args>(args)...)
            .c_str());
  }

private:
  class Implements {
  public:
    void verbose(const char *, ...);
    void debug(const char *, ...);
    void info(const char *, ...);
    void error(const char *, ...);
    void warn(const char *, ...);
    void critical(const char *, ...);
  };
  static Implements logger;
  template <typename... Args>
  static String stringFormatInternal(const String &format, Args &&...args) {
    int strLen =
        std::snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args)...);

    size_t bufferSize = strLen + sizeof(char);
    std::unique_ptr<char[]> buffer(new char[bufferSize]);
    std::snprintf(buffer.get(), bufferSize, format.c_str(), args...);
    return String(buffer.get(), buffer.get() + strLen);
  }
  template <typename T> static auto convert(T &&value) {
    if constexpr (std::is_same<std::remove_cv_t<std::remove_reference_t<T>>,
                               String>::value) {
      return std::forward<T>(value).c_str();
    } else {
      return std::forward<T>(value);
    }
  }
  template <typename... Args>
  static String stringFormatLogger(const String &format, Args &&...args) {
    return stringFormatInternal(format, convert(std::forward<Args>(args))...);
  }
};
} // namespace sinen

#endif // SINEN_LOGGER
