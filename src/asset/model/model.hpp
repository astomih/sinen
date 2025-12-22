#ifndef SINEN_MODEL_HPP
#define SINEN_MODEL_HPP
#include "mesh.hpp"
#include <asset/texture/material.hpp>
#include <core/data/hashmap.hpp>
#include <graphics/rhi/rhi.hpp>
#include <graphics/uniform_data.hpp>

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
  Array<glm::vec3> position;
  Array<float> positionTime;
  Array<glm::quat> rotation;
  Array<float> rotationTime;
  Array<glm::vec3> scale;
  Array<float> scaleTime;
};

class Node {
public:
  String name;
  glm::mat4 transformation;
  Array<Node> children;
};

class SkeletalAnimation {
public:
  void load(const Node &root, float ticksPerSecond, float duration,
            Hashmap<String, NodeAnimation> &nodeAnimationMap);

  void update(float timeInSeconds);

  void readNodeHierarchy(float animTime, const Node &node,
                         const glm::mat4 &parentTransform);

  glm::mat4 interpolateTransform(const NodeAnimation &channel, float time);

  Array<glm::mat4> getFinalBoneMatrices() const;

  Hashmap<String, NodeAnimation> nodeAnimMap;
  Array<AnimationVertex> animationVertices;
  glm::mat4 globalInverseTransform;
  float ticksPerSecond;
  float duration;
  Node root;
  Hashmap<String, glm::mat4> finalBoneMatrices;
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
  void load(StringView str);
  void loadFromPath(StringView path);
  void loadFromVertexArray(const Mesh &mesh);
  void loadSprite();
  void loadBox();
  void play(float start);
  void update(float delta_time);
  const AABB &getAABB() const;
  Ptr<void> data;
  UniformData getBoneUniformData() const;

  Material getMaterial() const { return material; }

  const Mesh &getMesh() const { return mesh; }
  void setMesh(const Mesh &m) { mesh = mesh; }

  Ptr<rhi::Buffer> vertexBuffer;
  Ptr<rhi::Buffer> tangentBuffer;
  Ptr<rhi::Buffer> animationVertexBuffer;
  Ptr<rhi::Buffer> indexBuffer;

  using BoneMap = Hashmap<String, BoneInfo>;
  const BoneMap &getBoneMap() const { return boneMap; }

private:
  void loadBoneUniform(float time);
  float time = 0.0f;
  Array<glm::mat4> inverseBindMatrices;
  Material material;
  AABB localAABB;
  String name;
  Mesh mesh;
  UniformData boneUniformData;

  SkeletalAnimation skeletalAnimation;
  BoneMap boneMap;
};
} // namespace sinen
#endif // !SINEN_MODEL_HPP
