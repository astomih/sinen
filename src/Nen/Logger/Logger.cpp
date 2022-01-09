#include <Nen.hpp>
#ifdef __ANDROID__
#include <android/log.h>
#endif

namespace nen {
std::unique_ptr<logger::interface_logger> logger::mLogger = nullptr;

void logger::MakeLogger(std::unique_ptr<logger::interface_logger> logger) {
  mLogger = std::move(logger);
}

void logger::default_logger::console_logger::Debug(std::string_view string) {
#ifdef _DEBUG
  std::cout << "DEBUG: " << string << std::endl;
#endif
}
void logger::default_logger::console_logger::Info(std::string_view string) {
#ifdef __ANDROID__
  __android_log_print(ANDROID_LOG_INFO, "NEN", "INFO: %s\n", string.data());
#else
  std::cout << "INFO: " << string << std::endl;
#endif
}
void logger::default_logger::console_logger::Error(std::string_view string) {

#ifdef __ANDROID__
  __android_log_print(ANDROID_LOG_ERROR, "NEN", "ERROR: %s\n", string.data());

#else
  std::cout << "ERROR: " << string << std::endl;
#endif
}
void logger::default_logger::console_logger::Warn(std::string_view string) {
  std::cout << "WARN: " << string << std::endl;
}
void logger::default_logger::console_logger::Fatal(std::string_view string) {
  std::cout << "FATAL: " << string << std::endl;
}

void logger::default_logger::file_logger::Debug(std::string_view string) {
#ifdef _DEBUG
  std::cout << "DEBUG: " << string << std::endl;
#endif
}
void logger::default_logger::file_logger::Info(std::string_view string) {
  std::cout << "INFO: " << string << std::endl;
}
void logger::default_logger::file_logger::Error(std::string_view string) {
  std::cout << "ERROR: " << string << std::endl;
}
void logger::default_logger::file_logger::Warn(std::string_view string) {
  std::cout << "WARN: " << string << std::endl;
}
void logger::default_logger::file_logger::Fatal(std::string_view string) {
  std::cout << "FATAL: " << string << std::endl;
}

} // namespace nen