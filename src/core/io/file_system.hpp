#ifndef SINEN_FILE_SYSTEM_HPP
#define SINEN_FILE_SYSTEM_HPP
#include <string>
#include <vector>
namespace sinen {
class FileSystem {
public:
  static std::vector<std::string> enumerateDirectory(const std::string &path);
  static std::string getAppBaseDirectory();
};
} // namespace sinen
#endif // SINEN_FILE_SYSTEM_HPP
