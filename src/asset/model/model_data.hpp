#ifndef SINEN_MODEL_DATA_HPP
#define SINEN_MODEL_DATA_HPP
#include "assimp/anim.h"
#include <asset/model/mesh.hpp>
#include <asset/model/model.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <graphics/graphics.hpp>
#include <paranoixa/paranoixa.hpp>
#include <physics/collision.hpp>

namespace sinen {
struct BoneInfo {
  glm::mat4 offsetMatrix;
  glm::mat4 finalTransform;
  uint32_t index;
};

class NodeAnimation {
public:
  std::vector<glm::vec3> position;
  std::vector<float> positionTime;
  std::vector<glm::quat> rotation;
  std::vector<float> rotationTime;
  std::vector<glm::vec3> scale;
  std::vector<float> scaleTime;
};

class Node {
public:
  std::string name;
  glm::mat4 transformation;
  std::vector<Node> children;
};

class SkeletalAnimation {
public:
  void load(const Node &root, float ticksPerSecond, float duration,
            std::unordered_map<std::string, NodeAnimation> &nodeAnimationMap);

  void update(float timeInSeconds);

  void readNodeHierarchy(float animTime, const Node &node,
                         const glm::mat4 &parentTransform);

  glm::mat4 interpolateTransform(const NodeAnimation &channel, float time);

  std::vector<glm::mat4> getFinalBoneMatrices() const;

  std::unordered_map<std::string, BoneInfo> boneMap;
  std::unordered_map<std::string, NodeAnimation> nodeAnimMap;
  glm::mat4 globalInverseTransform;
  float ticksPerSecond;
  float duration;
  Node root;
};
struct ModelData {
  AABB localAABB;
  std::string name;
  Mesh mesh;
  UniformData boneUniformData;

  px::Ptr<px::Buffer> vertexBuffer;
  px::Ptr<px::Buffer> animationVertexBuffer;
  px::Ptr<px::Buffer> indexBuffer;

  SkeletalAnimation skeletalAnimation;
};
inline std::shared_ptr<ModelData> getModelData(std::shared_ptr<void> model) {
  return std::static_pointer_cast<ModelData>(model);
}
std::pair<px::Ptr<px::Buffer>, px::Ptr<px::Buffer>>
createVertexIndexBuffer(const Mesh &mesh);
px::Ptr<px::Buffer> createAnimationVertexBuffer(const Mesh &mesh);
} // namespace sinen
#endif // SINEN_MODEL_DATA_HPP