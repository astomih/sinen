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
class data_stream {
public:
  static std::vector<uint8_t> key;
  static std::string_view open(const asset_type &type, std::string_view name);

  static void *open_as_rwops(const asset_type &type, std::string_view name);

  static std::string open_as_string(const asset_type &type,
                                    std::string_view name);
  static void write(const asset_type &type, std::string_view name,
                    std::string_view data);

  static void convert_file_path(const asset_type &type, std::string &filePath,
                                std::string_view name);
  static std::string convert_file_path(const asset_type &type,
                                       std::string_view name);
};
} // namespace sinen
#endif // !SINEN_DATA_STREAM_HPP
