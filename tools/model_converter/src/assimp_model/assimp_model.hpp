#ifndef SINEN_ANIM_MODEL_HPP
#define SINEN_ANIM_MODEL_HPP
#include "../assimp_animation/animation.hpp"
#include "../assimp_animation/node.hpp"
#include "../assimp_mesh/mesh.hpp"
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace sinen {
class assimp_model {
public:
  bool load_from_file(std::string_view filePath);
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
#endif // SINEN_ANIM_MODEL_HPP