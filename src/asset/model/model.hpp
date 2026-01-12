#ifndef SINEN_MODEL_HPP
#define SINEN_MODEL_HPP
#include "node.hpp"
#include "skeletal_animation.hpp"
#include <asset/texture/texture.hpp>
#include <core/buffer/buffer.hpp>
#include <core/data/hashmap.hpp>
#include <core/data/ptr.hpp>
#include <graphics/rhi/rhi.hpp>
#include <math/geometry/bbox.hpp>
#include <math/geometry/mesh.hpp>

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

  bool hasBaseColorTexture() const { return baseColor.has_value(); }
  Texture getBaseColorTexture() const { return baseColor.value(); }

  bool hasNormalTexture() const { return normal.has_value(); }
  Texture getNormalTexture() const { return normal.value(); }

  bool hasDiffuseRoughnessTexture() const {
    return diffuseRoughness.has_value();
  }
  Texture getDiffuseRoughnessTexture() const {
    return diffuseRoughness.value();
  }

  bool hasMetalnessTexture() const { return metalness.has_value(); }
  Texture getMetalnessTexture() const { return metalness.value(); }

  bool hasEmissiveTexture() const { return emissive.has_value(); }
  Texture getEmissiveTexture() const { return emissive.value(); }

  bool hasLightMapTexture() const { return lightMap.has_value(); }
  Texture getLightMapTexture() const { return lightMap.value(); }

private:
  void loadBoneUniform(float time);
  float time = 0.0f;
  AABB localAABB;
  String name;
  Ptr<Mesh> mesh;
  Array<Mat4> boneMatrices;

  SkeletalAnimation skeletalAnimation;
  BoneMap boneMap;
  std::optional<Texture> baseColor;
  std::optional<Texture> normal;
  std::optional<Texture> diffuseRoughness;
  std::optional<Texture> metalness;
  std::optional<Texture> emissive;
  std::optional<Texture> lightMap;
};
} // namespace sinen
#endif // !SINEN_MODEL_HPP
