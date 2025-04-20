#ifndef SINEN_MODEL_DATA_HPP
#define SINEN_MODEL_DATA_HPP
#include "assimp/anim.h"
#include <asset/model/model.hpp>
#include <asset/model/vertex_array.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <logic/physics/collision.hpp>
#include <paranoixa/paranoixa.hpp>
#include <render/renderer.hpp>

namespace sinen {
struct BoneInfo {
  glm::mat4 offsetMatrix;
  glm::mat4 finalTransform;
  uint32_t index;
};

class SkeletalAnimation {
public:
  std::unordered_map<std::string, BoneInfo> boneMap;
  std::unordered_map<std::string, aiNodeAnim *> nodeAnimMap;
  glm::mat4 globalInverseTransform;
  const aiScene *scene = nullptr;
  aiNode *root;

  void Load(const aiScene *scn);

  void Update(float timeInSeconds);

  void ReadNodeHierarchy(float animTime, aiNode *node,
                         const glm::mat4 &parentTransform);

  glm::mat4 InterpolateTransform(aiNodeAnim *channel, float time);

  std::vector<glm::mat4> GetFinalBoneMatrices() const;
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