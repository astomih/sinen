#include <Nen/Nen.hpp>
#include <fstream>
#include <sstream>
#include <rapidjson/document.h>

namespace nen
{
    bool Skeleton::Load(const std::string &fileName)
    {
        std::ifstream file(fileName);
        if (!file.is_open())
        {
            Logger::Error("File not found: Skeleton %s", fileName.c_str());
            return false;
        }

        std::stringstream fileStream;
        fileStream << file.rdbuf();
        std::string contents = fileStream.str();
        rapidjson::StringStream jsonStr(contents.c_str());
        rapidjson::Document doc;
        doc.ParseStream(jsonStr);

        if (!doc.IsObject())
        {
            Logger::Error("Skeleton %s is not valid json", fileName.c_str());
            return false;
        }

        int ver = doc["version"].GetInt();

        // Check the metadata
        if (ver != 1)
        {
            Logger::Error("Skeleton %s unknown format", fileName.c_str());
            return false;
        }

        const rapidjson::Value &bonecount = doc["bonecount"];
        if (!bonecount.IsUint())
        {
            Logger::Error("Skeleton %s doesn't have a bone count.", fileName.c_str());
            return false;
        }

        size_t count = bonecount.GetUint();

        if (count > MAX_SKELETON_BONES)
        {
            Logger::Error("Skeleton %s exceeds maximum bone count.", fileName.c_str());
            return false;
        }

        mBones.reserve(count);

        const rapidjson::Value &bones = doc["bones"];
        if (!bones.IsArray())
        {
            Logger::Error("Skeleton %s doesn't have a bone array?", fileName.c_str());
            return false;
        }

        if (bones.Size() != count)
        {
            Logger::Error("Skeleton %s has a mismatch between the bone count and number of bones", fileName.c_str());
            return false;
        }

        Bone temp;

        for (rapidjson::SizeType i = 0; i < count; i++)
        {
            if (!bones[i].IsObject())
            {
                Logger::Error("Skeleton %s: Bone %d is invalid.", fileName.c_str(), i);
                return false;
            }

            const rapidjson::Value &name = bones[i]["name"];
            temp.mName = name.GetString();

            const rapidjson::Value &parent = bones[i]["parent"];
            temp.mParent = parent.GetInt();

            const rapidjson::Value &bindpose = bones[i]["bindpose"];
            if (!bindpose.IsObject())
            {
                Logger::Error("Skeleton %s: Bone %d is invalid.", fileName.c_str(), i);
                return false;
            }

            const rapidjson::Value &rot = bindpose["rot"];
            const rapidjson::Value &trans = bindpose["trans"];

            if (!rot.IsArray() || !trans.IsArray())
            {
                Logger::Error("Skeleton %s: Bone %d is invalid.", fileName.c_str(), i);
                return false;
            }

            temp.mLocalBindPose.mRotation.x = rot[0].GetDouble();
            temp.mLocalBindPose.mRotation.y = rot[1].GetDouble();
            temp.mLocalBindPose.mRotation.z = rot[2].GetDouble();
            temp.mLocalBindPose.mRotation.w = rot[3].GetDouble();

            temp.mLocalBindPose.mTranslation.x = trans[0].GetDouble();
            temp.mLocalBindPose.mTranslation.y = trans[1].GetDouble();
            temp.mLocalBindPose.mTranslation.z = trans[2].GetDouble();

            mBones.emplace_back(temp);
        }

        ComputeGlobalInvBindPose();

        return true;
    }

    void Skeleton::ComputeGlobalInvBindPose()
    {
        mGlobalInvBindPoses.resize(GetNumBones());

        mGlobalInvBindPoses[0] = mBones[0].mLocalBindPose.ToMatrix();

        for (size_t i = 1; i < mGlobalInvBindPoses.size(); i++)
        {
            Matrix4 localMat = mBones[i].mLocalBindPose.ToMatrix();
            mGlobalInvBindPoses[i] = localMat * mGlobalInvBindPoses[mBones[i].mParent];
        }

        for (size_t i = 0; i < mGlobalInvBindPoses.size(); i++)
        {
            mGlobalInvBindPoses[i].Invert();
        }
    }
}