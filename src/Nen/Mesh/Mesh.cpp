#include <rapidjson/document.h>
#include <SDL.h>
#include <SDL_rwops.h>
#include <Nen.hpp>
#include <memory>
#include <sstream>
#include <fstream>
namespace nen
{

    bool Mesh::LoadFromFile(std::shared_ptr<Renderer> renderer, std::string_view filepath, std::string_view registerName)
    {
        mRenderer = renderer;
        name = registerName;
        auto path = "Assets/Model/" + std::string(filepath);
        std::ifstream file(path);
        if (!file.is_open())
        {
            Logger::Error("File not found: Mesh %s", path.c_str());
            return false;
        }

        std::stringstream fileStream;
        fileStream << file.rdbuf();
        std::string contents = fileStream.str();
        rapidjson::StringStream jsonStr(contents.c_str());
        rapidjson::Document doc;
        doc.ParseStream(jsonStr);

        if (!doc.IsObject())
        {
            SDL_Log("Mesh %s is not valid json", path.c_str());
            return false;
        }

        int ver = doc["version"].GetInt();

        // Check the version
        if (ver != 1)
        {
            SDL_Log("Mesh %s not version 1", path.c_str());
            return false;
        }

        //mShaderName = doc["shader"].GetString();

        // Skip the vertex format/shader for now
        // (This is changed in a later chapter's code)
        size_t vertSize = 8;

        // Load textures
        /*
        const rapidjson::Value &textures = doc["textures"];
        if (!textures.IsArray() || textures.Size() < 1)
        {
            SDL_Log("Mesh %s has no textures, there should be at least one", fileName.c_str());
            return false;
        }

        mSpecPower = static_cast<float>(doc["specularPower"].GetDouble());

        for (rapidjson::SizeType i = 0; i < textures.Size(); i++)
        {
            // Is this texture already loaded?
            std::string texName = textures[i].GetString();
            Texture *t = renderer->GetTexture(texName);
            if (t == nullptr)
            {
                // Try loading the texture
                t = renderer->GetTexture(texName);
                if (t == nullptr)
                {
                    // If it's still null, just use the default texture
                    t = renderer->GetTexture("Assets/Default.png");
                }
            }
            mTextures.emplace_back(t);
        }
        */
        // Load in the vertices
        const rapidjson::Value &vertsJson = doc["vertices"];
        if (!vertsJson.IsArray() || vertsJson.Size() < 1)
        {
            SDL_Log("Mesh %s has no vertices", path.c_str());
            return false;
        }

        //mRadius = 0.0f;
        for (rapidjson::SizeType i = 0; i < vertsJson.Size(); i++)
        {
            // For now, just assume we have 8 elements
            const rapidjson::Value &vert = vertsJson[i];
            if (!vert.IsArray() || vert.Size() != 8)
            {
                SDL_Log("Unexpected vertex format for %s", path.c_str());
                return false;
            }

            Vertex v;
            v.position = Vector3(vert[0].GetDouble(), vert[1].GetDouble(), vert[2].GetDouble());
            v.normal = Vector3(vert[3].GetDouble(), vert[4].GetDouble(), vert[5].GetDouble());
            v.uv = Vector2(vert[6].GetDouble(), vert[7].GetDouble());
            vArray.vertices.push_back(v);

            //mRadius = Math::Max(mRadius, pos.LengthSq());

            // Add the floats
        }

        // We were computing length squared earlier
        //mRadius = Math::Sqrt(mRadius);

        // Load in the indices
        const rapidjson::Value &indJson = doc["indices"];
        if (!indJson.IsArray() || indJson.Size() < 1)
        {
            SDL_Log("Mesh %s has no indices", path.c_str());
            return false;
        }

        for (rapidjson::SizeType i = 0; i < indJson.Size(); i++)
        {
            const rapidjson::Value &ind = indJson[i];
            if (!ind.IsArray() || ind.Size() != 3)
            {
                SDL_Log("Invalid indices for %s", path.c_str());
                return false;
            }

            vArray.indices.push_back(ind[0].GetUint());
            vArray.indices.push_back(ind[1].GetUint());
            vArray.indices.push_back(ind[2].GetUint());
        }
        vArray.indexCount = vArray.indices.size();
        return true;
    }

    void Mesh::Register()
    {
        if (!vArray.vertices.empty())
            mRenderer->AddVertexArray(vArray, name);
        else
            Logger::Warn("Not loaded %s", name.data());
    }
}