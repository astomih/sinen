#pragma once
#include <stdexcept>
#include <string>
namespace nen {
class exception : public std::runtime_error {
public:
  exception(const std::string &str) : std::runtime_error(str) {}
};
} // namespace nen