#ifndef NEN_LOGGER
#define NEN_LOGGER
#include <iostream>
#include <memory>
#include <string_view>

namespace nen {
namespace detail {
template <typename... Args>
std::string string_format_internal(const std::string &format, Args &&...args) {
  int str_len =
      std::snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args)...);

  size_t buffer_size = str_len + sizeof(char);
  std::unique_ptr<char[]> buffer(new char[buffer_size]);
  std::snprintf(buffer.get(), buffer_size, format.c_str(), args...);
  return std::string(buffer.get(), buffer.get() + str_len);
}
template <typename T> auto Convert(T &&value) {
  if constexpr (std::is_same<std::remove_cv_t<std::remove_reference_t<T>>,
                             std::string>::value) {
    return std::forward<T>(value).c_str();
  } else {
    return std::forward<T>(value);
  }
}
template <typename... Args>
std::string string_format_logger(const std::string &format, Args &&...args) {
  return ::nen::detail::string_format_internal(
      format, ::nen::detail::Convert(std::forward<Args>(args))...);
}
} // namespace detail

class logger {
public:
  class interface {
  public:
    virtual void Debug(std::string_view) {}
    virtual void Info(std::string_view) {}
    virtual void Error(std::string_view) {}
    virtual void Warn(std::string_view) {}
    virtual void Fatal(std::string_view) {}
  };
  static void MakeLogger(std::unique_ptr<interface> logger);
  template <typename... Args>
  static void debug(std::string_view format, Args &&...args) {
    mLogger->Debug(detail::string_format_logger(format.data(),
                                                std::forward<Args>(args)...));
  }
  template <typename... Args>
  static void info(std::string_view format, Args &&...args) {
    mLogger->Info(detail::string_format_logger(std::string(format),
                                               std::forward<Args>(args)...));
  }
  template <typename... Args>
  static void error(std::string_view format, Args &&...args) {
    mLogger->Error(detail::string_format_logger(std::string(format),
                                                std::forward<Args>(args)...));
  }
  template <typename... Args>
  static void warn(std::string_view format, Args &&...args) {
    mLogger->Warn(detail::string_format_logger(format.data(),
                                               std::forward<Args>(args)...));
  }
  template <typename... Args>
  static void fatal(std::string_view format, Args &&...args) {
    mLogger->Fatal(detail::string_format_logger(format.data(),
                                                std::forward<Args>(args)...));
  }

private:
  static std::unique_ptr<interface> mLogger;

public:
  class default_logger {
  public:
    static std::unique_ptr<interface> CreateConsoleLogger() {
      return std::move(std::make_unique<console_logger>());
    }

  private:
    /* コンソールに表示するロガー */
    class console_logger : public interface {
    public:
      virtual void Debug(std::string_view) override;
      virtual void Info(std::string_view) override;
      virtual void Error(std::string_view) override;
      virtual void Warn(std::string_view) override;
      virtual void Fatal(std::string_view) override;
    };
    class file_logger : public interface {
    public:
      virtual void Debug(std::string_view) override;
      virtual void Info(std::string_view) override;
      virtual void Error(std::string_view) override;
      virtual void Warn(std::string_view) override;
      virtual void Fatal(std::string_view) override;
    };
  };
};
} // namespace nen

#endif // NEN_LOGGER
