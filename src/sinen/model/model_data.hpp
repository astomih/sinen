#ifndef SINEN_MODEL_DATA_HPP
#define SINEN_MODEL_DATA_HPP
#include <model/model.hpp>
#include <model/vertex_array.hpp>
#include <paranoixa/paranoixa.hpp>
#include <physics/collision.hpp>

namespace sinen {
struct Vector4 {
  float x, y, z, w;
  Vector4() : x(0), y(0), z(0), w(0) {}
  Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};
struct Bone {
  std::vector<Vector4> boneIDs;
  std::vector<Vector4> boneWeights;
};
struct ModelData {
  AABB local_aabb;
  Model *parent;
  std::vector<Model *> children;
  std::string name;
  VertexArray v_array;

  Bone bone;

  px::Ptr<px::Buffer> vertexBuffer;
  px::Ptr<px::Buffer> indexBuffer;
};
inline std::shared_ptr<ModelData> GetModelData(std::shared_ptr<void> model) {
  return std::static_pointer_cast<ModelData>(model);
}
std::pair<px::Ptr<px::Buffer>, px::Ptr<px::Buffer>>
CreateVertexIndexBuffer(const VertexArray &vArray, const Bone &bone);
} // namespace sinen
#endif // SINEN_MODEL_DATA_HPP