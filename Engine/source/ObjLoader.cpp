#include <memory>
#include <Engine.hpp>
#include <tiny_obj_loader.h>

namespace nen::mesh
{
    bool ObjLoader::Load(std::shared_ptr<Renderer> renderer, std::string_view filepath, std::string_view name)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string err;
        std::string base_dir;
        base_dir = ".";
#ifdef _WIN32
        base_dir += "\\";
#else
        base_dir += "/";
#endif
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.data(),
                                    base_dir.c_str());
        if (!warn.empty())
        {
            std::cout << "WARN: " << warn << std::endl;
            return false;
        }
        if (!err.empty())
        {
            std::cerr << err << std::endl;
            return false;
        }

        VertexArray vArray;
        uint32_t indexCount = 0;
        for (const auto &shape : shapes)
        {
            for (const auto &index : shape.mesh.indices)
            {
                Vertex v{};
                v.pos = Vector3f(
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]);

                v.norm = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };

                v.uv = Vector2f(
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]);

                vArray.vertices.push_back(v);
                vArray.indices.push_back(vArray.indices.size());
            }
        }
        vArray.indexCount = vArray.indices.size();
        renderer->AddVertexArray(vArray, name);
        return true;
    }
}