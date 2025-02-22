#ifndef SINEN_MODEL_DATA_HPP
#define SINEN_MODEL_DATA_HPP
#include <model/model.hpp>
#include <paranoixa/paranoixa.hpp>
#include <physics/collision.hpp>
#include <vertex/vertex_array.hpp>
namespace sinen {
struct ModelData {
  AABB local_aabb;
  Model *parent;
  std::vector<Model *> children;
  std::string name;
  VertexArray v_array;

  px::Ptr<px::Buffer> vertexBuffer;
  px::Ptr<px::Buffer> indexBuffer;
};
inline std::shared_ptr<ModelData> GetModelData(std::shared_ptr<void> model) {
  return std::static_pointer_cast<ModelData>(model);
}
std::pair<px::Ptr<px::Buffer>, px::Ptr<px::Buffer>>
CreateVertexIndexBuffer(const VertexArray &vArray);
} // namespace sinen
#endif // SINEN_MODEL_DATA_HPP