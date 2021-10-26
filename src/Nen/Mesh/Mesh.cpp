#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#include <SDL.h>
#include <SDL_rwops.h>
#include <Nen.hpp>
#include <memory>
#include <sstream>
namespace nen
{
    Assimp::Importer importer;
    C_STRUCT aiLogStream stream;

    void LoadImpl(const C_STRUCT aiScene *sc, const C_STRUCT aiNode *nd, VertexArray &vArray)
    {
        unsigned int i;
        unsigned int n = 0, t;
        C_STRUCT aiMatrix4x4 m = nd->mTransformation;
        aiTransposeMatrix4(&m);

        for (; n < nd->mNumMeshes; ++n)
        {
            const C_STRUCT aiMesh *mesh = sc->mMeshes[nd->mMeshes[n]];

            for (t = 0; t < mesh->mNumFaces; ++t)
            {
                const C_STRUCT aiFace *face = &mesh->mFaces[t];

                for (i = 0; i < face->mNumIndices; i++)
                {
                    Vertex v{};
                    int index = face->mIndices[i];
                    if (mesh->mNormals != NULL)
                    {
                        v.normal.x = mesh->mNormals[index].x;
                        v.normal.y = mesh->mNormals[index].y;
                        v.normal.z = mesh->mNormals[index].z;
                        if (mesh->HasTextureCoords(0))
                        {
                            v.uv.x = mesh->mTextureCoords[0][index].x;
                            v.uv.y = 1 - mesh->mTextureCoords[0][index].y;
                        }
                    }
                    v.position.x = mesh->mVertices[index].x;
                    v.position.y = mesh->mVertices[index].y;
                    v.position.z = mesh->mVertices[index].z;

                    vArray.vertices.push_back(v);
                    vArray.indices.push_back(vArray.indices.size());
                }
            }
        }

        for (n = 0; n < nd->mNumChildren; ++n)
        {
            LoadImpl(sc, nd->mChildren[n], vArray);
        }
    }
    bool Mesh::LoadFromFile(std::shared_ptr<Renderer> renderer, std::string_view filepath, std::string_view registerName)
    {
        static auto once = [&]()
        {
            stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT, NULL);
            aiAttachLogStream(&stream);
            stream = aiGetPredefinedLogStream(aiDefaultLogStream_FILE, "assimp_log.txt");
            aiAttachLogStream(&stream);
        };
        mRenderer = renderer;
        name = registerName;
        auto path = "Assets/Model/" + std::string(filepath);
        const C_STRUCT aiScene *scene = NULL;
        scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Fast);
        if (!scene)
        {
            Logger::Error("%s", importer.GetErrorString());
        }

        int indices = 0;
        LoadImpl(scene, scene->mRootNode, vArray);
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