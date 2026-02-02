#ifndef SINEN_MODEL_HPP
#define SINEN_MODEL_HPP
#include "node.hpp"
#include "skeletal_animation.hpp"
#include <asset/texture/texture.hpp>
#include <core/buffer/buffer.hpp>
#include <core/data/hashmap.hpp>
#include <core/data/ptr.hpp>
#include <gpu/gpu.hpp>
#include <math/geometry/bbox.hpp>
#include <math/geometry/mesh.hpp>

namespace sinen {
enum class TextureKey : UInt32 {
  BaseColor,
  Normal,
  DiffuseRoughness,
  Metalness,
  Emissive,
  LightMap
};

class Model {
public:
  /**
   * @brief Construct a new model object
   *
   */
  Model();

  static constexpr const char *metaTableName() { return "sn.Model"; }
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
  void loadFromVertexArray(const Mesh &mesh);
  void loadSprite();
  void loadBox();
  void play(float start);
  void update(float delta_time);
  const AABB &getAABB() const;
  Ptr<void> data;
  Buffer getBoneUniformBuffer() const;

  Mesh getMesh() const { return mesh; }

  Ptr<gpu::Buffer> vertexBuffer;
  Ptr<gpu::Buffer> tangentBuffer;
  Ptr<gpu::Buffer> animationVertexBuffer;
  Ptr<gpu::Buffer> indexBuffer;

  struct BoneInfo {
    Mat4 offsetMatrix;
    Mat4 finalTransform;
    uint32_t index;
  };
  using BoneMap = Hashmap<String, BoneInfo>;
  const BoneMap &getBoneMap() { return this->boneMap; }

  bool hasTexture(TextureKey type) const;
  Ptr<Texture> getTexture(TextureKey type) const;
  void setTexture(TextureKey type, const Ptr<Texture> &texture);

private:
  void loadBoneUniform(float time);
  float time = 0.0f;
  AABB localAABB;
  String name;
  Mesh mesh;
  Array<Mat4> boneMatrices;

  SkeletalAnimation skeletalAnimation;
  BoneMap boneMap;
  Array<Ptr<Texture>> textures;
};
} // namespace sinen
#endif // !SINEN_MODEL_HPP
