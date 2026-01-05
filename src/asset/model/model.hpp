#ifndef SINEN_MODEL_HPP
#define SINEN_MODEL_HPP
#include "node.hpp"
#include "skeletal_animation.hpp"
#include <asset/texture/material.hpp>
#include <core/buffer/buffer.hpp>
#include <core/data/hashmap.hpp>
#include <core/data/ptr.hpp>
#include <geometry/bbox.hpp>
#include <geometry/mesh.hpp>
#include <graphics/rhi/rhi.hpp>

namespace sinen {

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
  void load(const Buffer &buffer);
  void loadFromVertexArray(const Ptr<Mesh> &mesh);
  void loadSprite();
  void loadBox();
  void play(float start);
  void update(float delta_time);
  const AABB &getAABB() const;
  Ptr<void> data;
  Buffer getBoneUniformBuffer() const;

  Material getMaterial() const { return material; }

  const Ptr<Mesh> &getMesh() const { return mesh; }

  Ptr<rhi::Buffer> vertexBuffer;
  Ptr<rhi::Buffer> tangentBuffer;
  Ptr<rhi::Buffer> animationVertexBuffer;
  Ptr<rhi::Buffer> indexBuffer;

  struct BoneInfo {
    Mat4 offsetMatrix;
    Mat4 finalTransform;
    uint32_t index;
  };
  using BoneMap = Hashmap<String, BoneInfo>;
  const BoneMap &getBoneMap() { return this->boneMap; }

private:
  void loadBoneUniform(float time);
  float time = 0.0f;
  Material material;
  AABB localAABB;
  String name;
  Ptr<Mesh> mesh;
  Array<Mat4> boneMatrices;

  SkeletalAnimation skeletalAnimation;
  BoneMap boneMap;
};
} // namespace sinen
#endif // !SINEN_MODEL_HPP
