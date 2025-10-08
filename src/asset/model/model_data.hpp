#ifndef SINEN_MODEL_DATA_HPP
#define SINEN_MODEL_DATA_HPP
#include <asset/model/mesh.hpp>
#include <asset/model/model.hpp>

namespace sinen {
std::pair<px::Ptr<px::Buffer>, px::Ptr<px::Buffer>>
createVertexIndexBuffer(const Mesh &mesh);
px::Ptr<px::Buffer> createAnimationVertexBuffer(const Mesh &mesh);
} // namespace sinen
#endif // SINEN_MODEL_DATA_HPP