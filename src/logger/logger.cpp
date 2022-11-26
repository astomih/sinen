#include <string>
#ifdef __ANDROID__
#include <android/log.h>
#endif
#include <logger/logger.hpp>

namespace sinen {
std::unique_ptr<logger::interface> logger::mLogger =
    logger::default_logger::CreateConsoleLogger();

void logger::change_logger(std::unique_ptr<logger::interface> logger) {
  mLogger = std::move(logger);
}

void logger::default_logger::console_logger::debug(std::string_view string) {
#ifdef _DEBUG
  std::cout << "SINEN_DEBUG: " << string << std::endl;
#endif
}
void logger::default_logger::console_logger::info(std::string_view string) {
#ifdef __ANDROID__
  __android_log_print(ANDROID_LOG_INFO, "NEN", "INFO: %s\n", string.data());
#else
  std::cout << "SINEN_INFO: " << string << std::endl;
#endif
}
void logger::default_logger::console_logger::error(std::string_view string) {

#ifdef __ANDROID__
  __android_log_print(ANDROID_LOG_ERROR, "NEN", "ERROR: %s\n", string.data());

#else
  std::cout << "SINEN_ERROR: " << string << std::endl;
#endif
}
void logger::default_logger::console_logger::warn(std::string_view string) {
  std::cout << "SINEN_WARNING: " << string << std::endl;
}
void logger::default_logger::console_logger::fatal(std::string_view string) {
  std::cout << "SINEN_FATAL: " << string << std::endl;
}

void logger::default_logger::file_logger::debug(std::string_view string) {
#ifdef _DEBUG
  std::cout << "SINEN_DEBUG: " << string << std::endl;
#endif
}
void logger::default_logger::file_logger::info(std::string_view string) {
  std::cout << "SINEN_INFO: " << string << std::endl;
}
void logger::default_logger::file_logger::error(std::string_view string) {
  std::cout << "SINEN_ERROR: " << string << std::endl;
}
void logger::default_logger::file_logger::warn(std::string_view string) {
  std::cout << "SINEN_WARNING: " << string << std::endl;
}
void logger::default_logger::file_logger::fatal(std::string_view string) {
  std::cout << "SINEN_FATAL: " << string << std::endl;
}

} // namespace sinen
