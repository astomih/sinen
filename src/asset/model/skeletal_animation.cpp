#include "model.hpp"
#include <core/logger/logger.hpp>
#include <math/math.hpp>
namespace sinen {
void SkeletalAnimation::load(const Node &root, float ticksPerSecond,
                             float duration,
                             Hashmap<String, NodeAnimation> &nodeAnimationMap) {
  this->root = root;
  this->ticksPerSecond = ticksPerSecond;
  this->duration = duration;
  this->globalInverseTransform = Mat4::invert(root.transformation);
  this->nodeAnimMap = nodeAnimationMap;
}
void SkeletalAnimation::update(float timeInSeconds) {
  float ticksPerSecond =
      this->ticksPerSecond != 0 ? this->ticksPerSecond : 25.0f;
  float timeInTicks = timeInSeconds * ticksPerSecond;
  float animationTime = Math::fmod(timeInTicks, duration);

  readNodeHierarchy(animationTime, root, Mat4(1.0f));
}
void SkeletalAnimation::readNodeHierarchy(float animTime, const Node &node,
                                          const Mat4 &parentTransform) {
  String nodeName = node.name;

  auto nodeTransform = node.transformation;
  if (nodeAnimMap.contains(nodeName)) {
    nodeTransform = interpolateTransform(nodeAnimMap[nodeName], animTime);
  }

  auto globalTransform = parentTransform * nodeTransform;

  const auto &boneMap = owner->getBoneMap();
  if (boneMap.contains(nodeName)) {
    finalBoneMatrices[nodeName] = globalInverseTransform * globalTransform *
                                  boneMap.at(nodeName).offsetMatrix;
  }

  for (const auto &child : node.children) {
    readNodeHierarchy(animTime, child, globalTransform);
  }
}

uint32_t getIndex(float time, const Array<float> &keys) {
  auto count = keys.size();
  for (uint32_t i = 0; i < count - 1; ++i) {
    if (time < keys[i + 1]) {
      return i;
    }
  }
  return count - 1;
}

Vec3 interpolateVector3(const Array<Vec3> &keys, const Array<float> &keyTimes,
                        float time, bool isScaling = false) {
  auto count = keys.size();
  if (count == 0) {
    if (isScaling) {
      return Vec3(1, 1, 1);
    }
    return Vec3(0, 0, 0);
  }
  if (count == 1) {
    return keys[0];
  }

  uint32_t i = getIndex(time, keyTimes);
  float t = float((time - keyTimes[i]) / (keyTimes[i + 1] - keyTimes[i]));
  auto k0 = keys[i];
  auto k1 = keys[i + 1];
  return Vec3::lerp(k0, k1, t);
};
Quaternion interpolateQuat(const Array<Quaternion> &keys,
                           const Array<float> &keyTimes, float time) {
  auto count = keys.size();
  if (count == 0) {
    return Quaternion(0, 0, 0, 1);
  }
  if (count == 1) {
    return keys[0];
  }

  uint32_t i = getIndex(time, keyTimes);
  float t = float((time - keyTimes[i]) / (keyTimes[i + 1] - keyTimes[i]));
  auto k0 = keys[i];
  auto k1 = keys[i + 1];
  auto q = Quaternion::slerp(k0, k1, t);
  q = Quaternion::normalize(q);
  return q;
};

Mat4 SkeletalAnimation::interpolateTransform(const NodeAnimation &channel,
                                             float time) {

  auto scaling =
      interpolateVector3(channel.scale, channel.scaleTime, time, true);
  auto rotation = interpolateQuat(channel.rotation, channel.rotationTime, time);
  auto translation =
      interpolateVector3(channel.position, channel.positionTime, time);

  auto t = Mat4::create_translation(translation);
  auto r = Mat4::create_from_quaternion(rotation);
  auto s = Mat4::create_scale(scaling);

  auto m = t * r * s;
  return m;
}
Array<Mat4> SkeletalAnimation::getFinalBoneMatrices() const {
  Array<Mat4> result(finalBoneMatrices.size(), Mat4(1.0f));
  for (const auto &[name, mat] : finalBoneMatrices) {
    if (finalBoneMatrices.contains(name)) {
      result[owner->getBoneMap().at(name).index] = mat;
    } else {
      Logger::error(
          "[Warning] bone \"%s\" has no finalTransform. Using identity", name);
    }
  }
  return result;
}
} // namespace sinen