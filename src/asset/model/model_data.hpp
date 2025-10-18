#ifndef SINEN_MODEL_DATA_HPP
#define SINEN_MODEL_DATA_HPP
#include <asset/model/mesh.hpp>
#include <asset/model/model.hpp>

namespace sinen {
std::pair<px::Ptr<px::Buffer>, px::Ptr<px::Buffer>>
createVertexIndexBuffer(const std::vector<Vertex> &vertices,
                        const std::vector<uint32_t> &indices);
px::Ptr<px::Buffer>
createAnimationVertexBuffer(const std::vector<AnimationVertex> &vertices);
px::Ptr<px::Buffer> createBuffer(size_t size, void *data,
                                 px::BufferUsage usage);
} // namespace sinen
#endif // SINEN_MODEL_DATA_HPP