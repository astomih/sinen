#ifndef SINEN_FILE_SYSTEM_HPP
#define SINEN_FILE_SYSTEM_HPP
#include <core/buffer/buffer.hpp>
#include <core/data/array.hpp>
#include <core/data/string.hpp>
#include <optional>

namespace sinen {
enum class FilesystemAccess {
  Read,
  Write,
};

class Filesystem {
public:
  static Array<String> enumerateDirectory(StringView path);
  static std::optional<Buffer> read(StringView path);
  static bool write(StringView path, const Buffer &buffer);
  static String getAppBaseDirectory();
  static String getUserDirectory();
  static bool resolveSandboxPath(StringView path, FilesystemAccess access,
                                 String &resolvedPath);
  static bool isSandboxPathAllowed(StringView path, FilesystemAccess access);
};
} // namespace sinen
#endif // SINEN_FILE_SYSTEM_HPP
