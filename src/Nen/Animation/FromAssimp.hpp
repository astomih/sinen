#pragma once
#include <Nen.hpp>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
namespace nen
{
    Vector3 fromAssimp(const aiVector3D &v);

    Color fromAssimp(const aiColor3D &col);

    Color fromAssimp(const aiColor4D &col);

    VectorKey fromAssimp(const aiVectorKey &key);

    QuatKey fromAssimp(const aiQuatKey &key);
}