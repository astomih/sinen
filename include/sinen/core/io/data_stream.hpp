#ifndef SINEN_DATA_STREAM_HPP
#define SINEN_DATA_STREAM_HPP
// std
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
// internal
#include "asset_type.hpp"
namespace sinen {

/**
 * @brief Data stream class
 *
 */
class DataStream {
public:
  static std::vector<uint8_t> key;
  static std::string_view Open(const AssetType &type, std::string_view name);

  static void *OpenAsRWOps(const AssetType &type, std::string_view name);

  static std::string OpenAsString(const AssetType &type, std::string_view name);
  static void Write(const AssetType &type, std::string_view name,
                    std::string_view data);

  static void ConvertFilePath(const AssetType &type, std::string &filePath,
                              std::string_view name);
  static std::string ConvertFilePath(const AssetType &type,
                                     std::string_view name);
};
} // namespace sinen
#endif // !SINEN_DATA_STREAM_HPP
