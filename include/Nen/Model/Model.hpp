#pragma once
#include <vector>
#include <map>
#include <memory>
#include <string>
#include "../Animation/Animation.hpp"
#include "../Mesh/Mesh.hpp"
#include "../Animation/Node.hpp"
namespace nen
{
    class Model
    {
    public:
        /**
         * @brief ファイルから読み込む
         * 
         * @param renderer 
         * @param filePath 
         * @param registerName 
         */
        void LoadFromFile(std::shared_ptr<class Renderer> renderer, std::string_view filePath, std::string_view registerName);

        /**
         * @brief アニメーションを更新
         * 
         * @param time アニメーション速度
         * @param index アニメーション番号
         */
        void UpdateAnimation(const double time, const size_t index);

        // 親子関係にあるノード
        std::shared_ptr<Node> node;

        // 名前からノードを探す用(アニメーションで使う)
        std::map<std::string, std::shared_ptr<Node>> node_index;

        // 親子関係を解除した状態(全ノードの行列を更新する時に使う)
        std::vector<std::shared_ptr<Node>> node_list;

        bool has_anim;
        std::vector<Animation> animation;

        Mesh mesh;
        void *scene;

        std::shared_ptr<class Renderer> mRenderer;
        std::string_view name;

    private:
        void UpdateNodeMatrix(const float time, const Animation &animation);
        void UpdateNodeDerivedMatrix(const std::shared_ptr<Node> &node, const Matrix4 &parent_matrix);
        void UpdateMesh();
    };
}