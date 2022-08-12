#pragma once
#include "../Animation/Animation.hpp"
#include "../Animation/Node.hpp"
#include "../Mesh/Mesh.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace sinen {
class Model {
public:
  /**
   * @brief ファイルから読み込む
   *
   * @param filePath
   */
  void LoadFromFile(std::string_view filePath);

  /**
   * @brief アニメーションを更新
   *
   * @param time アニメーション速度
   * @param index アニメーション番号
   */
  void UpdateAnimation(const double time, const size_t index);
  std::shared_ptr<Node> node;
  std::map<std::string, std::shared_ptr<Node>> node_index;
  std::vector<std::shared_ptr<Node>> node_list;
  bool has_anim;
  std::vector<Animation> animation;
  std::shared_ptr<class renderer> mRenderer;
  std::string_view name;

private:
  void NormalizeMeshWeight();
};
} // namespace sinen