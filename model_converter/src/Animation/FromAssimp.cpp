#include "FromAssimp.hpp"

namespace sinen {
vector3 fromAssimp(const aiVector3D &v) { return vector3{v.x, v.y, v.z}; }

color fromAssimp(const aiColor3D &col) {
  return color{col.r, col.g, col.b, 1.0};
}

color fromAssimp(const aiColor4D &col) {
  return color{col.r, col.g, col.b, col.a};
}

VectorKey fromAssimp(const aiVectorKey &key) {
  VectorKey v;

  v.time = key.mTime;
  v.value = fromAssimp(key.mValue);

  return v;
}

QuatKey fromAssimp(const aiQuatKey &key) {
  QuatKey v;

  v.time = key.mTime;
  v.value = quaternion{key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z};

  return v;
}
} // namespace sinen