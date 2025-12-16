#ifndef SINEN_LOGGER
#define SINEN_LOGGER
#include <functional>
#include <iostream>
#include <memory>
#include <string_view>

namespace sinen {

class Logger {
private:
public:
  enum class priority : int { verbose = 2, debug, info, warn, error, critical };
  static void
      setOutputFunction(std::function<void(priority, std::string_view)>);
  template <typename... Args>
  static void verbose(std::string_view format, Args &&...args) {
    logger.verbose(
        stringFormatLogger(std::string(format), std::forward<Args>(args)...)
            .c_str());
  }
  template <typename... Args>
  static void debug(std::string_view format, Args &&...args) {
    logger.debug(
        stringFormatLogger(std::string(format), std::forward<Args>(args)...)
            .c_str());
  }
  template <typename... Args>
  static void info(std::string_view format, Args &&...args) {
    logger.info(
        stringFormatLogger(std::string(format), std::forward<Args>(args)...)
            .c_str());
  }
  template <typename... Args>
  static void error(std::string_view format, Args &&...args) {
    logger.error(
        stringFormatLogger(std::string(format), std::forward<Args>(args)...)
            .c_str());
  }
  template <typename... Args>
  static void warn(std::string_view format, Args &&...args) {
    logger.warn(
        stringFormatLogger(std::string(format), std::forward<Args>(args)...)
            .c_str());
  }
  template <typename... Args>
  static void critical(std::string_view format, Args &&...args) {
    logger.critical(
        stringFormatLogger(std::string(format), std::forward<Args>(args)...)
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
  static std::string stringFormatInternal(const std::string &format,
                                          Args &&...args) {
    int str_len =
        std::snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args)...);

    size_t buffer_size = str_len + sizeof(char);
    std::unique_ptr<char[]> buffer(new char[buffer_size]);
    std::snprintf(buffer.get(), buffer_size, format.c_str(), args...);
    return std::string(buffer.get(), buffer.get() + str_len);
  }
  template <typename T> static auto convert(T &&value) {
    if constexpr (std::is_same<std::remove_cv_t<std::remove_reference_t<T>>,
                               std::string>::value) {
      return std::forward<T>(value).c_str();
    } else {
      return std::forward<T>(value);
    }
  }
  template <typename... Args>
  static std::string stringFormatLogger(const std::string &format,
                                        Args &&...args) {
    return stringFormatInternal(format, convert(std::forward<Args>(args))...);
  }
};
} // namespace sinen

#endif // SINEN_LOGGER
