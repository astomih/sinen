#ifndef SINEN_FROM_ASSIMP_HPP
#define SINEN_FROM_ASSIMP_HPP

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "animation.hpp"
namespace sinen {
vector3 fromAssimp(const aiVector3D &v);

color fromAssimp(const aiColor3D &col);

color fromAssimp(const aiColor4D &col);

VectorKey fromAssimp(const aiVectorKey &key);

QuatKey fromAssimp(const aiQuatKey &key);
} // namespace sinen
  #endif // SINEN_FROM_ASSIMP_HPP