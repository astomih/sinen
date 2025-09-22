#ifndef SINEN_SCRIPT_HPP
#define SINEN_SCRIPT_HPP
#include <string>
namespace sinen {
class Script {
public:
  static void Load(const std::string &filePath, const std::string &baseDirPath);
};
} // namespace sinen

#endif // SINEN_SCRIPT_HPP
