#ifndef SINEN_ARGUMENTS_HPP
#define SINEN_ARGUMENTS_HPP
#include <core/data/array.hpp>
#include <core/data/string.hpp>

namespace sinen {
class Arguments {
public:
  static int getArgc();
  static Array<String> getArgv();
  inline static int argc = 0;
  inline static Array<String> argv;
};
} // namespace sinen

#endif // SINEN_ARGUMENTS_HPP
