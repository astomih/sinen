#ifndef SINEN_SCRIPT_HPP
#define SINEN_SCRIPT_HPP
#include <core/data/string.hpp>
namespace sinen {
class Script {
public:
  static void load(StringView filePath, StringView baseDirPath = ".");
};
} // namespace sinen

#endif // SINEN_SCRIPT_HPP
