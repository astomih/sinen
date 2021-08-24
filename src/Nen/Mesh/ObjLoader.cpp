#include <tiny_obj_loader.h>
#include <memory>
#include <sstream>
#include <SDL.h>
#include <SDL_rwops.h>
#include <Nen.hpp>

namespace nen::mesh
{
    std::string LoadTextFile(const std::string &path)
    {
        SDL_RWops *file{SDL_RWFromFile(path.c_str(), "r")};
        size_t fileLength{static_cast<size_t>(SDL_RWsize(file))};
        void *data{SDL_LoadFile_RW(file, nullptr, 1)};
        std::string result(static_cast<char *>(data), fileLength);
        SDL_free(data);

        return result;
    }

    bool ObjLoader::Load(std::shared_ptr<Renderer> renderer, std::string_view filepath, std::string_view name)
    {
        std::string path = filepath.data();
        std::istringstream sourceStream(LoadTextFile(path));
        if (filepath.ends_with(".obj"))
        {
            for (int i = 0; i < 3;i++)
                path.pop_back();
            path += "mtl";
        }
        std::istringstream materialStream(LoadTextFile(path.data()));
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string err;
        auto materialStreamReader = std::make_unique<tinyobj::MaterialStreamReader>(materialStream);
        bool ret = tinyobj::LoadObj(
            &attrib,
            &shapes,
            &materials,
            &warn,
            &err,
            &sourceStream,
            materialStreamReader.get());
        if (!warn.empty())
        {
            nen::Logger::Warn("%s", warn);
        }
        if (!err.empty())
        {
            nen::Logger::Error("%s", err);
            return false;
        }

        VertexArray vArray;
        uint32_t indexCount = 0;
        for (const auto &shape : shapes)
        {
            for (const auto &index : shape.mesh.indices)
            {
                Vertex v{};
                v.pos = Vector3(
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]);

                v.norm = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };

                v.uv = Vector2(
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