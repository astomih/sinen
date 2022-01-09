#pragma once
#include "../Math/Vector2.hpp"
#include "../Math/Vector3.hpp"

namespace nen {
/**
 * @brief 頂点情報
 *
 */
struct vertex {
  /**
   * @brief 位置ベクトル
   *
   */
  vector3 position;

  /**
   * @brief 法線ベクトル
   *
   */
  vector3 normal;

  /**
   * @brief UV座標
   *
   */
  vector2 uv;
};
} // namespace nen