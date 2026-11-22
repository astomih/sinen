#ifndef SINEN_MODEL_HPP
#define SINEN_MODEL_HPP
#include "node.hpp"
#include "skeletal_animation.hpp"
#include <core/buffer/buffer.hpp>
#include <core/data/hashmap.hpp>
#include <core/data/ptr.hpp>
#include <gpu/gpu.hpp>
#include <graphics/texture/texture.hpp>
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
  struct LoadOptions {
    UInt32 maxTextureSize = 0;
    // For authoring pipelines that store ORM in a legacy Specular texture.
    bool packedSpecular = false;
    bool flipNormalY = false;
    // Bake the authored node pose even when the file contains animations.
    bool staticScene = false;
  };
  struct SurfaceProperties {
    float roughnessFactor = 1.0f;
    float metallicFactor = 1.0f;
    Color emissiveFactor{1.0f};
    float normalY = 1.0f;
    float alphaCutoff = 0.0f;
    bool autoAlphaMask = false;
    bool constantMetallicRoughness = false;
    String materialName;
  };
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
  void load(StringView str, const LoadOptions &options);
  void load(const Buffer &buffer);
  void load(const Buffer &buffer, StringView formatHint);
  void load(const Buffer &buffer, StringView formatHint,
            const LoadOptions &options);
  void loadFromVertexArray(const Mesh &mesh, bool optimize = true);
  // Returns false when the backend cannot provide command-local streaming.
  bool loadTransientFromVertexArray(const Mesh &mesh,
                                    const Ptr<gpu::CommandBuffer> &commands);
  void loadSprite();
  void loadBox();
  void play(float start);
  void update(float delta_time);
  const AABB &getAABB() const;
  Ptr<void> data;
  Buffer getBoneUniformBuffer() const;

  Mesh getMesh() const;
  const Array<Ptr<Model>> &getParts() const { return parts; }
  const SurfaceProperties &getSurfaceProperties() const { return surface; }
  UInt32 getMaterialCount() const { return materialCount; }
  UInt32 getTriangleCount() const;
  bool isReady() const;
  UInt32 getLodCount() const;
  UInt32 selectLod(float distance) const;
  const Array<UInt32> &getIndicesForLod(UInt32 lod) const;

  Ptr<gpu::Buffer> vertexBuffer;
  Ptr<gpu::Buffer> tangentBuffer;
  Ptr<gpu::Buffer> animationVertexBuffer;
  Ptr<gpu::Buffer> indexBuffer;
  Array<Ptr<gpu::Buffer>> lodIndexBuffers;

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
  void finalizeImport(const Ptr<void> &state);
  void updateBounds();
  void loadBoneUniform(float time);
  float time = 0.0f;
  AABB localAABB;
  String name;
  mutable Mesh mesh;
  Array<Ptr<Model>> parts;
  SurfaceProperties surface;
  UInt32 materialCount = 0;
  Array<Mat4> boneMatrices;

  SkeletalAnimation skeletalAnimation;
  BoneMap boneMap;
  Array<Ptr<Texture>> textures;
};
} // namespace sinen
#endif // !SINEN_MODEL_HPP
