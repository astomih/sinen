#ifndef SINEN_EXCEPTION_HPP
#define SINEN_EXCEPTION_HPP

#include <stdexcept>
#include <string>
namespace sinen {
class exception : public std::runtime_error {
public:
  exception(const std::string &str) : std::runtime_error(str) {}
};
} // namespace sinen
#endif // !SINEN_EXCEPTION_HPP
