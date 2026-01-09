#ifndef SINEN_DATA_STREAM_HPP
#define SINEN_DATA_STREAM_HPP
#include <core/data/array.hpp>
#include <core/data/string.hpp>
namespace sinen {

/**
 * @brief Data stream class
 *
 */
class AssetIO {
public:
  static Array<uint8_t> key;
  static StringView open(StringView name);

  static void *openAsIOStream(StringView name);

  static String openAsString(StringView name);
  static void write(StringView name, StringView data);
  static String getFilePath(StringView name);
};
} // namespace sinen
#endif // !SINEN_DATA_STREAM_HPP
