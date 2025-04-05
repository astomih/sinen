#ifndef SINEN_MODEL_DATA_HPP
#define SINEN_MODEL_DATA_HPP
#include "assimp/anim.h"
#include "assimp/matrix4x4.h"
#include "math/matrix4.hpp"
#include "math/quaternion.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <model/model.hpp>
#include <model/vertex_array.hpp>
#include <paranoixa/paranoixa.hpp>
#include <physics/collision.hpp>
#include <render/renderer.hpp>

namespace sinen {
struct BoneInfo {
  matrix4 offsetMatrix;
  matrix4 finalTransform;
};

class SkeletalAnimation {
public:
  std::unordered_map<std::string, BoneInfo> boneMap;
  std::unordered_map<std::string, aiNodeAnim *> nodeAnimMap;
  std::unordered_map<std::string, unsigned int> boneNameToIndex;
  matrix4 globalInverseTransform;
  const aiScene *scene = nullptr;
  aiNode *root;

  void Load(const aiScene *scn);

  void Update(float timeInSeconds);

  void ReadNodeHierarchy(float animTime, aiNode *node,
                         const matrix4 &parentTransform);

  matrix4 InterpolateTransform(aiNodeAnim *channel, float time);

  std::vector<matrix4> GetFinalBoneMatrices() const;
};
struct ModelData {
  AABB local_aabb;
  Model *parent;
  std::vector<Model *> children;
  std::string name;
  VertexArray v_array;
  UniformData boneUniformData;

  px::Ptr<px::Buffer> vertexBuffer;
  px::Ptr<px::Buffer> indexBuffer;

  const aiScene *scene = nullptr;
  Assimp::Importer importer;
  SkeletalAnimation skeletalAnimation;
};
inline std::shared_ptr<ModelData> GetModelData(std::shared_ptr<void> model) {
  return std::static_pointer_cast<ModelData>(model);
}
std::pair<px::Ptr<px::Buffer>, px::Ptr<px::Buffer>>
CreateVertexIndexBuffer(const VertexArray &vArray);
} // namespace sinen
#endif // SINEN_MODEL_DATA_HPP