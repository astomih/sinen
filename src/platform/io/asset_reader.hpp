#ifndef SINEN_DATA_STREAM_HPP
#define SINEN_DATA_STREAM_HPP
#include <core/data/array.hpp>
#include <core/data/string.hpp>
namespace sinen {

class AssetReader {
public:
  static constexpr const char *archiveExtension() { return ".sna"; }
  static constexpr const char *archiveRootDirectory() { return "sna"; }

  static Array<uint8_t> key;
  static StringView open(StringView name);

  static void *openAsIOStream(StringView name);

  static String openAsString(StringView name);
  static String getFilePath(StringView name);
  static String getLoadPath(StringView name);

  static bool mountArchive(StringView path);
  static void unmountArchive();
  static bool isArchiveMounted();
  static bool isArchivePath(StringView path);
  static bool exists(StringView name);
  static bool archiveEntryExists(StringView path);
  static bool archiveDirectoryExists(StringView path);
  static String openArchiveEntryAsString(StringView path);
  static Array<String> enumerateArchiveDirectory(StringView path);
};
} // namespace sinen
#endif // !SINEN_DATA_STREAM_HPP
