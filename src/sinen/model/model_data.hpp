#ifndef SINEN_MODEL_DATA_HPP
#define SINEN_MODEL_DATA_HPP
#include "assimp/anim.h"
#include "assimp/matrix4x4.h"
#include "math/quaternion.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <model/model.hpp>
#include <model/vertex_array.hpp>
#include <paranoixa/paranoixa.hpp>
#include <physics/collision.hpp>
#include <render/renderer.hpp>

namespace sinen {
struct BoneInfo {
  aiMatrix4x4 offsetMatrix;
  aiMatrix4x4 finalTransform;
};

class SkeletalAnimation {
public:
  std::unordered_map<std::string, BoneInfo> boneMap;
  std::unordered_map<std::string, aiNodeAnim *> nodeAnimMap;
  aiMatrix4x4 globalInverseTransform;
  const aiScene *scene = nullptr;

  void Load(const aiScene *scn) {
    scene = scn;
    globalInverseTransform = scene->mRootNode->mTransformation;
    globalInverseTransform.Inverse();

    for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes;
         ++meshIndex) {
      aiMesh *mesh = scene->mMeshes[meshIndex];
      for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones;
           ++boneIndex) {
        aiBone *bone = mesh->mBones[boneIndex];
        boneMap[bone->mName.C_Str()].offsetMatrix = bone->mOffsetMatrix;
      }
    }

    if (scene->mNumAnimations > 0) {
      aiAnimation *anim = scene->mAnimations[0];
      for (unsigned int i = 0; i < anim->mNumChannels; ++i) {
        aiNodeAnim *channel = anim->mChannels[i];
        nodeAnimMap[channel->mNodeName.C_Str()] = channel;
      }
    }
  }

  void Update(float timeInSeconds) {
    aiAnimation *anim = scene->mAnimations[0];
    float ticksPerSecond =
        anim->mTicksPerSecond != 0 ? anim->mTicksPerSecond : 25.0f;
    float timeInTicks = timeInSeconds * ticksPerSecond;
    float animationTime = fmod(timeInTicks, anim->mDuration);

    ReadNodeHierarchy(animationTime, scene->mRootNode, aiMatrix4x4());
  }

  void ReadNodeHierarchy(float animTime, aiNode *node,
                         const aiMatrix4x4 &parentTransform) {
    std::string nodeName = node->mName.C_Str();

    aiMatrix4x4 nodeTransform = node->mTransformation;
    if (nodeAnimMap.count(nodeName)) {
      nodeTransform = InterpolateTransform(nodeAnimMap[nodeName], animTime);
    }

    aiMatrix4x4 globalTransform = parentTransform * nodeTransform;

    if (boneMap.count(nodeName)) {
      boneMap[nodeName].finalTransform = globalInverseTransform *
                                         globalTransform *
                                         boneMap[nodeName].offsetMatrix;
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
      ReadNodeHierarchy(animTime, node->mChildren[i], globalTransform);
    }
  }

  aiMatrix4x4 InterpolateTransform(aiNodeAnim *channel, float time) {
    auto Interpolate = [](auto keys, unsigned int count, float time,
                          auto getter) {
      if (count == 1)
        return getter(keys[0]);
      for (unsigned int i = 0; i < count - 1; ++i) {
        if (time < keys[i + 1].mTime) {
          float t =
              (time - keys[i].mTime) / (keys[i + 1].mTime - keys[i].mTime);
          return getter(keys[i]) * (1 - t) + getter(keys[i + 1]) * t;
        }
      }
      return getter(keys[count - 1]);
    };

    aiVector3D scaling =
        Interpolate(channel->mScalingKeys, channel->mNumScalingKeys, time,
                    [](auto &k) { return k.mValue; });
    aiQuaternion rotation;
    if (channel->mNumRotationKeys == 1)
      rotation = channel->mRotationKeys[0].mValue;
    else {
      for (unsigned int i = 0; i < channel->mNumRotationKeys - 1; ++i) {
        if (time < channel->mRotationKeys[i + 1].mTime) {
          float t = (time - channel->mRotationKeys[i].mTime) /
                    (channel->mRotationKeys[i + 1].mTime -
                     channel->mRotationKeys[i].mTime);
          aiQuaternion::Interpolate(rotation, channel->mRotationKeys[i].mValue,
                                    channel->mRotationKeys[i + 1].mValue, t);
          break;
        }
      }
    }
    aiVector3D translation =
        Interpolate(channel->mPositionKeys, channel->mNumPositionKeys, time,
                    [](auto &k) { return k.mValue; });

    aiMatrix4x4 scaleMat;
    aiMatrix4x4::Scaling(scaling, scaleMat);
    aiMatrix4x4 rotMat(rotation.GetMatrix());
    aiMatrix4x4 translationMat;
    aiMatrix4x4::Translation(translation, translationMat);

    return scaleMat * rotMat * translationMat;
  }

  std::vector<aiMatrix4x4> GetFinalBoneMatrices() const {
    std::vector<aiMatrix4x4> result;
    for (const auto &[name, info] : boneMap) {
      result.push_back(info.finalTransform);
    }
    return result;
  }
};
struct ModelData {
  AABB local_aabb;
  Model *parent;
  std::vector<Model *> children;
  std::string name;
  VertexArray v_array;
  UniformData boneUniformData;

  px::Ptr<px::Buffer> vertexBuffer;
  px::Ptr<px::Buffer> indexBuffer;

  const aiScene *scene = nullptr;
  Assimp::Importer importer;
  SkeletalAnimation skeletalAnimation;
};
inline std::shared_ptr<ModelData> GetModelData(std::shared_ptr<void> model) {
  return std::static_pointer_cast<ModelData>(model);
}
std::pair<px::Ptr<px::Buffer>, px::Ptr<px::Buffer>>
CreateVertexIndexBuffer(const VertexArray &vArray);
} // namespace sinen
#endif // SINEN_MODEL_DATA_HPP