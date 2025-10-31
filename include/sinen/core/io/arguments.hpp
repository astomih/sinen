#ifndef SINEN_ARGUMENTS_HPP
#define SINEN_ARGUMENTS_HPP
#include <string>
#include <vector>

namespace sinen {
class Arguments {
public:
  static int getArgc();
  static std::vector<std::string> getArgv();
};
} // namespace sinen

#endif // SINEN_ARGUMENTS_HPP
