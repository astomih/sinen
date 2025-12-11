#ifndef SINEN_MODEL_DATA_HPP
#define SINEN_MODEL_DATA_HPP
#include <asset/model/mesh.hpp>
#include <asset/model/model.hpp>

namespace sinen {
std::pair<rhi::Ptr<rhi::Buffer>, rhi::Ptr<rhi::Buffer>>
createVertexIndexBuffer(const std::vector<Vertex> &vertices,
                        const std::vector<uint32_t> &indices);
rhi::Ptr<rhi::Buffer>
createAnimationVertexBuffer(const std::vector<AnimationVertex> &vertices);
rhi::Ptr<rhi::Buffer> createBuffer(size_t size, void *data,
                                   rhi::BufferUsage usage);
} // namespace sinen
#endif // SINEN_MODEL_DATA_HPP