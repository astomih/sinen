#ifndef NEN_LOGGER
#define NEN_LOGGER
#include <iostream>
#include <memory>
#include <string_view>

namespace nen {
/* ログの種類 */
enum class log_type {
  DEBUG,
  INFO,
#ifdef ERROR
#undef ERROR
  ERROR,
#define ERROR 0
#else
  ERROR,
#endif
  WARN,
  FATAL
};

namespace detail {
template <typename... Args>
std::string StringFormatInternal(const std::string &format, Args &&...args) {
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
} // namespace detail
/* ロガーのための文字列フォーマッティングを行う */
template <typename... Args>
std::string StringFormatLogger(const std::string &format, Args &&...args) {
  return ::nen::detail::StringFormatInternal(
      format, ::nen::detail::Convert(std::forward<Args>(args))...);
}

/* ロギングを行うクラス */
class logger {
public:
  class interface_logger {
  public:
    virtual void Debug(std::string_view) {}
    virtual void Info(std::string_view) {}
    virtual void Error(std::string_view) {}
    virtual void Warn(std::string_view) {}
    virtual void Fatal(std::string_view) {}
  };
  /* ロガーの実装のインスタンスを移します */
  static void MakeLogger(std::unique_ptr<interface_logger> logger);

  /* デバッグビルド時のみに表示される情報ログ */
  template <typename... Args>
  static void Debug(std::string_view format, Args &&...args) {
    mLogger->Debug(
        StringFormatLogger(format.data(), std::forward<Args>(args)...));
  }

  /* デバッグ・リリースビルド時に表示される情報ログ */
  template <typename... Args>
  static void Info(std::string_view format, Args &&...args) {
    mLogger->Info(
        StringFormatLogger(std::string(format), std::forward<Args>(args)...));
  }

  /* エラーを表示するログ */
  template <typename... Args>
  static void Error(std::string_view format, Args &&...args) {
    mLogger->Error(
        StringFormatLogger(std::string(format), std::forward<Args>(args)...));
  }
  /* 警告を表示するログ */
  template <typename... Args>
  static void Warn(std::string_view format, Args &&...args) {
    mLogger->Warn(
        StringFormatLogger(format.data(), std::forward<Args>(args)...));
  }
  /* 致命的なエラー（Errorよりも深刻）を表示するログ */
  template <typename... Args>
  static void Fatal(std::string_view format, Args &&...args) {
    mLogger->Fatal(
        StringFormatLogger(format.data(), std::forward<Args>(args)...));
  }

private:
  static std::unique_ptr<interface_logger> mLogger;

public:
  /* Nen標準のロガー実装 */
  class default_logger {
  public:
    static std::unique_ptr<interface_logger> CreateConsoleLogger() {
      return std::move(std::make_unique<console_logger>());
    }

  private:
    /* コンソールに表示するロガー */
    class console_logger : public interface_logger {
    public:
      virtual void Debug(std::string_view) override;
      virtual void Info(std::string_view) override;
      virtual void Error(std::string_view) override;
      virtual void Warn(std::string_view) override;
      virtual void Fatal(std::string_view) override;
    };
    class file_logger : public interface_logger {
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
