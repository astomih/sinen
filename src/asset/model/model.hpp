#ifndef SINEN_MODEL_HPP
#define SINEN_MODEL_HPP
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include "../../physics/collision.hpp"
#include "graphics/uniform_data.hpp"
#include "mesh.hpp"
#include <asset/texture/material.hpp>

#include <graphics/rhi/rhi.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

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

  std::unordered_map<std::string, NodeAnimation> nodeAnimMap;
  std::vector<AnimationVertex> animationVertices;
  glm::mat4 globalInverseTransform;
  float ticksPerSecond;
  float duration;
  Node root;
  std::unordered_map<std::string, glm::mat4> finalBoneMatrices;
  class Model *owner;
};
struct Model {
public:
  /**
   * @brief Construct a new model object
   *
   */
  Model();
  /**
   * @brief Destroy the model object
   *
   */
  ~Model() = default;
  /**
   * @brief Load the model from a file
   *
   * @brief Model format is a custom format(.sim)
   * @param str
   */
  void load(std::string_view str);
  void loadFromPath(std::string_view path);
  void loadFromVertexArray(const Mesh &mesh);
  void loadSprite();
  void loadBox();
  void play(float start);
  void update(float delta_time);
  const AABB &getAABB() const;
  std::shared_ptr<void> data;
  UniformData getBoneUniformData() const;

  Material getMaterial() const { return material; }

  const Mesh &getMesh() const { return mesh; }
  void setMesh(const Mesh &m) { mesh = mesh; }

  Ptr<rhi::Buffer> vertexBuffer;
  Ptr<rhi::Buffer> tangentBuffer;
  Ptr<rhi::Buffer> animationVertexBuffer;
  Ptr<rhi::Buffer> indexBuffer;

  using BoneMap = std::unordered_map<std::string, BoneInfo>;
  const BoneMap &getBoneMap() const { return boneMap; }

private:
  void loadBoneUniform(float time);
  float time = 0.0f;
  std::vector<glm::mat4> inverseBindMatrices;
  Material material;
  AABB localAABB;
  std::string name;
  Mesh mesh;
  UniformData boneUniformData;

  SkeletalAnimation skeletalAnimation;
  BoneMap boneMap;
};
} // namespace sinen
#endif // !SINEN_MODEL_HPP
