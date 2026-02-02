#ifndef SINEN_NODE_HPP
#define SINEN_NODE_HPP
#include <core/data/array.hpp>
#include <core/data/string.hpp>
#include <math/matrix.hpp>
#include <math/quaternion.hpp>
#include <math/vector.hpp>
namespace sinen {
class NodeAnimation {
public:
  Array<Vec3> position;
  Array<float> positionTime;
  Array<Quat> rotation;
  Array<float> rotationTime;
  Array<Vec3> scale;
  Array<float> scaleTime;
};

class Node {
public:
  String name;
  Mat4 transformation;
  Array<Node> children;
};
} // namespace sinen
#endif