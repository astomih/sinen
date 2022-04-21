#pragma once
#include "shader_type.hpp"
#include <string>

namespace nen {
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
} // namespace nen