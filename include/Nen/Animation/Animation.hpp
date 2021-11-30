#pragma once
#include "BoneTransform.hpp"
#include <vector>
#include <string>

namespace nen
{
    class Animation
    {
    public:
        bool Load(const std::string &fileName);

        size_t GetNumBones() const { return mNumBones; }
        size_t GetNumFrames() const { return mNumFrames; }
        float GetDuration() const { return mDuration; }
        float GetFrameDuration() const { return mFrameDuration; }

        void GetGlobalPoseAtTime(std::vector<Matrix4> &outPoses, const class Skeleton *inSkeleton, float inTime) const;

    private:
        size_t mNumBones;
        size_t mNumFrames;
        float mDuration;
        float mFrameDuration;
        std::vector<std::vector<BoneTransform>> mTracks;
    };
}