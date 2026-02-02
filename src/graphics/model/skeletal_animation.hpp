#ifndef SINEN_SKELETAL_ANIMATION_HPP
#define SINEN_SKELETAL_ANIMATION_HPP
#include "node.hpp"
#include <core/data/hashmap.hpp>
#include <math/geometry/skinned_vertex.hpp>
#include <math/geometry/vertex.hpp>
namespace sinen {
class SkeletalAnimation {
public:
  void load(const Node &root, float ticksPerSecond, float duration,
            Hashmap<String, NodeAnimation> &nodeAnimationMap);

  void update(float timeInSeconds);

  void readNodeHierarchy(float animTime, const Node &node,
                         const Mat4 &parentTransform);

  Mat4 interpolateTransform(const NodeAnimation &channel, float time);

  Array<Mat4> getFinalBoneMatrices() const;

  Hashmap<String, NodeAnimation> nodeAnimMap;
  Array<SkinnedVertex> skinnedVertices;
  Mat4 globalInverseTransform;
  float ticksPerSecond;
  float duration;
  Node root;
  Hashmap<String, Mat4> finalBoneMatrices;
  class Model *owner;
};
} // namespace sinen
#endif