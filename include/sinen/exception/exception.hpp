#ifndef SINEN_EXCEPTION_HPP
#define SINEN_EXCEPTION_HPP

#include <stdexcept>
#include <string>
namespace sinen {
/**
 * @brief Exception class
 *
 */
class exception : public std::runtime_error {
public:
  /**
   * @brief Construct a new exception object
   *
   * @param str Exception message
   */
  exception(const std::string &str) : std::runtime_error(str) {}
};
} // namespace sinen
#endif // !SINEN_EXCEPTION_HPP
