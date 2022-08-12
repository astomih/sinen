#pragma once
#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <sinen.hpp>

#include "Animation.hpp"
namespace sinen {
vector3 fromAssimp(const aiVector3D &v);

color fromAssimp(const aiColor3D &col);

color fromAssimp(const aiColor4D &col);

VectorKey fromAssimp(const aiVectorKey &key);

QuatKey fromAssimp(const aiQuatKey &key);
} // namespace sinen