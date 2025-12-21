#ifndef SINEN_FILE_SYSTEM_HPP
#define SINEN_FILE_SYSTEM_HPP
#include <core/data/array.hpp>
#include <core/data/string.hpp>
namespace sinen {
class FileSystem {
public:
  static Array<String> enumerateDirectory(StringView path);
  static String getAppBaseDirectory();
};
} // namespace sinen
#endif // SINEN_FILE_SYSTEM_HPP
