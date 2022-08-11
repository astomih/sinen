#ifndef SINEN_SHADER_HPP
#define SINEN_SHADER_HPP
#include "shader_type.hpp"
#include <string>

namespace sinen {
class shader {
public:
  shader();
  ~shader() = default;

  std::string vertName;
  std::string fragName;
  bool operator==(const shader &info) const {
    return this->vertName == info.vertName && this->fragName == info.fragName;
  }
};
} // namespace sinen
#endif // !SINEN_SHADER_HPP
