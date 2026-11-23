#pragma once

#include <string>
#include <string_view>

namespace sinen::luau_to_cpp {
struct Result {
  std::string cpp;
  std::string error;
};

Result convert(std::string_view source, std::string_view sourceName);
} // namespace sinen::luau_to_cpp
