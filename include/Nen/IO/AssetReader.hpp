#pragma once
#include "AssetType.hpp"
#include <string>
#include <cstddef>

namespace nen
{

    /**
     * @brief 汎用のAssets内のファイルの読み込みを行うクラス
     * 
     */
    class AssetReader
    {
    public:
        /**
         * @brief アセットを読み込む
         * 
         * @param assetType アセットの種類。EffectならAssets/Effect内にあるファイルを読み込む。
         * @param name Assets/〇〇(任意のディレクトリ)にあるファイル。
         * @return std::string_view 
         */
        static std::string_view Load(const AssetType &assetType, std::string_view name);

        /**
         * @brief SDL_RWopsとしてアセットを読み込む
         * 
         * @param assetType アセットの種類。EffectならAssets/Effect内にあるファイルを読み込む。
         * @param name Assets/〇〇(任意のディレクトリ)にあるファイル。
         * @return void* SDL_RWops
         */
        static void *LoadAsRWops(const AssetType &assetType, std::string_view name);

        /**
         * @brief std::stringとしてアセットを読み込む
         * 
         * @param assetType アセットの種類。EffectならAssets/Effect内にあるファイルを読み込む。
         * @param name Assets/〇〇(任意のディレクトリ)にあるファイル。
         * @return std::string 
         */
        static std::string LoadAsString(const AssetType &assetType, std::string_view name);

        static void ConvertFilePath(std::string &filePath, std::string_view name, const AssetType &assetType);
    };
}