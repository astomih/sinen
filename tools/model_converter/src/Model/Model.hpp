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
  void LoadFromFile(std::string_view filePath);
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