#pragma once
#include "../Math/Vector3.hpp"
#include "../Math/Vector2.hpp"
namespace nen
{
    /**
     * @brief 頂点情報
     * 
     */
    struct Vertex
    {
        /**
         * @brief 位置ベクトル
         * 
         */
        Vector3 position;

        /**
         * @brief 法線ベクトル
         * 
         */
        Vector3 normal;

        /**
         * @brief UV座標
         * 
         */
        Vector2 uv;
    };
}