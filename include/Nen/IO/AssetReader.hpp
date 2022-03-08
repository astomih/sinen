#pragma once
#include "AssetType.hpp"
#include <cstddef>
#include <string>

namespace nen {

class data_io {
public:
  static std::string_view Load(const asset_type &assetType,
                               std::string_view name);

  static void *LoadAsRWops(const asset_type &assetType, std::string_view name);

  static std::string LoadAsString(const asset_type &assetType,
                                  std::string_view name);
  static void write(const asset_type &assetType, std::string_view name,
                    std::string_view data);

  static void ConvertFilePath(std::string &filePath, std::string_view name,
                              const asset_type &assetType);
  static std::string ConvertFilePath(std::string_view name,
                                     const asset_type &assetType);
};
} // namespace nen