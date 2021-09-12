#pragma once
#include "../Render/Renderer.hpp"
#include <memory>
#include <string_view>
namespace nen
{
    class Mesh
    {
    public:
        class Obj
        {
        public:
            /**
             * @brief ファイルからobj形式のメッシュを読み込む
             * 
             * @param renderer レンダラ
             * @param filePath ファイルのパス（相対）
             * @param registerName 登録名
             * @return true 読み込み成功
             * @return false 読み込み失敗
             */
            static bool LoadFromFile(std::shared_ptr<class Renderer> renderer, std::string_view filePath, std::string_view registerName);
        };
    };
}