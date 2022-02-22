#pragma once
#include "AssetType.hpp"
#include <cstddef>
#include <string>

namespace nen {

/**
 * @brief 汎用のAssets内のファイルの読み込みを行うクラス
 *
 */
class asset_reader {
public:
  /**
   * @brief アセットを読み込む
   *
   * @brief asset_type
   * アセットの種類。EffectならAssets/Effect内にあるファイルを読み込む。
   * @param name Assets/〇〇(任意のディレクトリ)にあるファイル。
   * @return std::string_view
   */
  static std::string_view Load(const asset_type &assetType,
                               std::string_view name);

  /**
   * @brief SDL_RWopsとしてアセットを読み込む
   *
   * @param asset_type
   * アセットの種類。EffectならAssets/Effect内にあるファイルを読み込む。
   * @param name Assets/〇〇(任意のディレクトリ)にあるファイル。
   * @return void* SDL_RWops
   */
  static void *LoadAsRWops(const asset_type &assetType, std::string_view name);

  /**
   * @brief std::stringとしてアセットを読み込む
   *
   * @param asset_type
   * アセットの種類。EffectならAssets/Effect内にあるファイルを読み込む。
   * @param name Assets/〇〇(任意のディレクトリ)にあるファイル。
   * @return std::string
   */
  static std::string LoadAsString(const asset_type &assetType,
                                  std::string_view name);

  static void ConvertFilePath(std::string &filePath, std::string_view name,
                              const asset_type &assetType);
  static std::string ConvertFilePath(std::string_view name,
                                     const asset_type &assetType);
};
} // namespace nen