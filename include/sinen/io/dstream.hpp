#pragma once
#include "asset_type.hpp"
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace nen {

class dstream {
public:
  static std::vector<uint8_t> key;
  static std::string_view open(const asset_type &assetType,
                               std::string_view name);

  static void *open_as_rwops(const asset_type &assetType,
                             std::string_view name);

  static std::string open_as_string(const asset_type &assetType,
                                    std::string_view name);
  static void write(const asset_type &assetType, std::string_view name,
                    std::string_view data);

  static void convert_file_path(std::string &filePath, std::string_view name,
                                const asset_type &assetType);
  static std::string convert_file_path(std::string_view name,
                                       const asset_type &assetType);
};
} // namespace nen