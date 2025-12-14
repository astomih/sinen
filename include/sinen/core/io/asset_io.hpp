#ifndef SINEN_DATA_STREAM_HPP
#define SINEN_DATA_STREAM_HPP
// std
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
namespace sinen {

/**
 * @brief Data stream class
 *
 */
class AssetIO {
public:
  static std::vector<uint8_t> key;
  static std::string_view open(std::string_view name);

  static void *openAsIOStream(std::string_view name);

  static std::string openAsString(std::string_view name);
  static void write(std::string_view name, std::string_view data);
  static std::string getFilePath(std::string_view name);
};
} // namespace sinen
#endif // !SINEN_DATA_STREAM_HPP
