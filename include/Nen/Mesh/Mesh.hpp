#pragma once
#include "../Render/Renderer.hpp"
#include "../Vertex/VertexArray.hpp"
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
            Obj() = default;
            ~Obj() = default;
            /**
             * @brief ファイルからobj形式のメッシュを読み込む
             * 
             * @param renderer レンダラ
             * @param filePath ファイルのパス（相対）
             * @param registerName 登録名
             * @return true 読み込み成功
             * @return false 読み込み失敗
             */
            bool LoadFromFile(std::shared_ptr<class Renderer> renderer, std::string_view filePath, std::string_view registerName);
            void Register();

        private:
            std::string_view name;
            VertexArray vArray;
            std::shared_ptr<class Renderer> mRenderer;
        };
    };
}