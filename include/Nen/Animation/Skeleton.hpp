#pragma once
#include "BoneTransform.hpp"
#include <string>
#include <vector>
namespace nen
{
    class Skeleton
    {
    public:
        struct Bone
        {
            BoneTransform mLocalBindPose;
            std::string mName;
            int mParent;
        };

        bool Load(const std::string &fileName);

        size_t GetNumBones() const { return mBones.size(); }
        const Bone &GetBone(size_t idx) const { return mBones[idx]; }
        const std::vector<Bone> &GetBones() const { return mBones; }
        const std::vector<Matrix4> &GetGlobalInvBindPoses() const { return mGlobalInvBindPoses; }

    protected:
        void ComputeGlobalInvBindPose();

    private:
        std::vector<Bone> mBones;
        std::vector<Matrix4> mGlobalInvBindPoses;
    };
}