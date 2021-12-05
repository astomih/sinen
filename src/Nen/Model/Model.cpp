#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#include <Nen.hpp>
#include "../Animation/FromAssimp.hpp"

namespace nen
{
    Animation createAnimation(const aiAnimation *anim);
    std::shared_ptr<Node> createNode(const aiNode *const n, aiMesh **mesh);
    Mesh createMesh(const aiMesh *const m);
    Bone createBone(const aiBone *b);
    void createNodeInfo(const std::shared_ptr<Node> &node,
                        std::map<std::string, std::shared_ptr<Node>> &node_index,
                        std::vector<std::shared_ptr<Node>> &node_list);

    void Model::LoadFromFile(std::shared_ptr<class Renderer> renderer, std::string_view filePath, std::string_view registerName)
    {
        mRenderer = renderer;
        name = registerName;
        Assimp::Importer importer;
#ifdef _DEBUG
        C_STRUCT aiLogStream stream;
        stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT, NULL);
        aiAttachLogStream(&stream);

        stream = aiGetPredefinedLogStream(aiDefaultLogStream_FILE, "assimp_log.txt");
        aiAttachLogStream(&stream);
#endif

        auto path = AssetReader::LoadAsString(AssetType::Model, filePath);
        auto pos = std::string(filePath).find_last_of(".");
        auto hint = std::string(filePath).substr(pos + 1, std::string(filePath).size());

        scene = (void *)importer.ReadFileFromMemory(path.c_str(), path.size(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes | aiProcess_LimitBoneWeights | aiProcess_RemoveRedundantMaterials, hint.c_str());
        if (!scene)
        {
            Logger::Error("%s", importer.GetErrorString());
        }

        int indices = 0;
        auto *sc = (aiScene *)scene;

        this->node = createNode(sc->mRootNode, sc->mMeshes);

        // ノードを名前から探せるようにする
        createNodeInfo(this->node,
                       this->node_index,
                       this->node_list);
        has_anim = sc->HasAnimations();
        if (has_anim)
        {
            this->node = createNode(sc->mRootNode, sc->mMeshes);
            aiAnimation **anim = sc->mAnimations;
            for (uint32_t i = 0; i < sc->mNumAnimations; ++i)
            {
                animation.push_back(createAnimation(anim[i]));
            }
        }
    }
    // ノードに付随するアニメーション情報を作成
    NodeAnim createNodeAnim(const aiNodeAnim *anim)
    {
        NodeAnim animation;

        animation.node_name = anim->mNodeName.C_Str();

        // 平行移動
        for (uint32_t i = 0; i < anim->mNumPositionKeys; ++i)
        {
            animation.translate.push_back(fromAssimp(anim->mPositionKeys[i]));
        }

        // スケーリング
        for (uint32_t i = 0; i < anim->mNumScalingKeys; ++i)
        {
            animation.scaling.push_back(fromAssimp(anim->mScalingKeys[i]));
        }

        // 回転
        for (uint32_t i = 0; i < anim->mNumRotationKeys; ++i)
        {
            animation.rotation.push_back(fromAssimp(anim->mRotationKeys[i]));
        }

        return animation;
    }
    Animation createAnimation(const aiAnimation *anim)
    {
        Animation animation;

        animation.mDuration = anim->mDuration;

        {
            // 階層アニメーション
            aiNodeAnim **node_anim = anim->mChannels;
            for (uint32_t i = 0; i < anim->mNumChannels; ++i)
            {
                animation.body.push_back(createNodeAnim(node_anim[i]));
            }
        }

        return animation;
    }
    std::shared_ptr<Node> createNode(const aiNode *const n, aiMesh **mesh)
    {
        auto node = std::make_shared<Node>();

        node->name = n->mName.C_Str();

        for (uint32_t i = 0; i < n->mNumMeshes; ++i)
        {
            node->mesh.push_back(createMesh(mesh[n->mMeshes[i]]));
        }
        memcpy(&node->matrix.mat, &n->mTransformation, sizeof(float) * 16);
        // 初期値を保存しておく
        node->matrix_orig = node->matrix;

        for (uint32_t i = 0; i < n->mNumChildren; ++i)
        {
            node->children.push_back(createNode(n->mChildren[i], mesh));
        }

        return node;
    }
    // ボーンの情報を作成
    Bone createBone(const aiBone *b)
    {
        Bone bone;

        bone.name = b->mName.C_Str();
        memcpy(&bone.offset.mat, &b->mOffsetMatrix, sizeof(float) * 16);

        const aiVertexWeight *w = b->mWeights;
        for (uint32_t i = 0; i < b->mNumWeights; ++i)
        {
            Weight weight{w[i].mVertexId, w[i].mWeight};
            bone.weights.push_back(weight);
        }

        return bone;
    }

    Mesh createMesh(const aiMesh *const m)
    {
        Mesh mesh;
        // 頂点データを取り出す
        uint32_t num_vtx = m->mNumVertices;
        const aiVector3D *vtx = m->mVertices;
        std::vector<Vertex> v;
        v.resize(num_vtx);

        if (m->HasPositions())
        {
            for (uint32_t h = 0; h < num_vtx; ++h)
            {
                v[h].position = fromAssimp(vtx[h]);
            }
        }

        // 法線
        if (m->HasNormals())
        {
            const aiVector3D *normal = m->mNormals;
            for (uint32_t h = 0; h < num_vtx; ++h)
            {
                v[h].normal = fromAssimp(normal[h]);
            }
        }

        // テクスチャ座標(マルチテクスチャには非対応)
        if (m->HasTextureCoords(0))
        {
            const aiVector3D *uv = m->mTextureCoords[0];
            for (uint32_t h = 0; h < num_vtx; ++h)
            {
                v[h].uv = Vector2(uv[h].x, uv[h].y);
            }
        }

        {
            VertexArray va;
            va.vertices = v;
            for (uint32_t i = 0; i < m->mNumFaces; i++)
            {
                for (uint32_t j = 0; j < m->mFaces[i].mNumIndices; j++)
                {
                    va.indices.push_back(m->mFaces[i].mIndices[j]);
                }
            }
            va.indexCount = va.indices.size();
            mesh.body = va;
        }
        // 骨情報
        mesh.has_bone = m->HasBones();
        if (mesh.has_bone)
        {
            aiBone **b = m->mBones;
            for (uint32_t i = 0; i < m->mNumBones; ++i)
            {
                mesh.bones.push_back(createBone(b[i]));
            }

            // 骨アニメーションで書き換えるので、元を保存
            mesh.original = mesh.body;
        }
        return mesh;
    }
    // 再帰を使って全ノード情報を生成
    void createNodeInfo(const std::shared_ptr<Node> &node,
                        std::map<std::string, std::shared_ptr<Node>> &node_index,
                        std::vector<std::shared_ptr<Node>> &node_list)
    {

        node_index.insert(std::make_pair(node->name, node));
        node_list.push_back(node);

        for (auto child : node->children)
        {
            createNodeInfo(child, node_index, node_list);
        }
    }

    void Model::UpdateAnimation(const double time, const size_t index)
    {
        if (!this->has_anim)
            return;

        // 最大時間でループさせている
        float current_time = std::fmod(time, this->animation[index].mDuration);

        // アニメーションで全ノードの行列を更新
        UpdateNodeMatrix(current_time, this->animation[index]);

        // ノードの行列を再計算
        UpdateNodeDerivedMatrix(this->node, Matrix4::Identity);

        // メッシュアニメーションを適用
        UpdateMesh();
    }
    template <typename T>
    struct Comp
    {
        bool operator()(const double lhs, const T &rhs) { return lhs < rhs.time; }
    };
    Quaternion getLerpValue(const float time, const std::vector<QuatKey> &values)
    {
        auto result = std::upper_bound(values.begin(), values.end(),
                                       time, Comp<QuatKey>());

        Quaternion value;
        if (result == values.begin())
        {
            // 先頭より小さい時間
            value = result->value;
        }
        else if (result == values.end())
        {
            // 最後尾より大きい時間
            value = (result - 1)->value;
        }
        else
        {
            // 球面補間
            double dt = result->time - (result - 1)->time;
            double t = time - (result - 1)->time;
            value = Quaternion::Slerp((result - 1)->value, result->value, (float)(t / dt));
        }

        return value;
    }
    // キーフレームから直線補間した値を取り出す
    Vector3 getLerpValue(const float time, const std::vector<VectorKey> &values)
    {
        // 適用キー位置を探す
        auto result = std::upper_bound(values.begin(), values.end(),
                                       time, Comp<VectorKey>());

        Vector3 value;
        if (result == values.begin())
        {
            // 先頭より小さい時間
            value = result->value;
        }
        else if (result == values.end())
        {
            // 最後尾より大きい時間
            value = (result - 1)->value;
        }
        else
        {
            // 直線補間
            double dt = result->time - (result - 1)->time;
            double t = time - (result - 1)->time;
            value = Vector3::Lerp((result - 1)->value, result->value, t / dt);
        }

        return value;
    }
    // 階層アニメーション用の行列を計算
    void Model::UpdateNodeMatrix(const float time, const Animation &animation)
    {
        for (const auto &body : animation.body)
        {
            Matrix4 matrix;

            // 階層アニメーションを取り出して行列を生成
            Vector3 scaling = getLerpValue(time, body.scaling);
            Matrix4 s = Matrix4::Identity;
            s.mat[0][0] = scaling.x;
            s.mat[1][1] = scaling.y;
            s.mat[2][2] = scaling.z;

            matrix = s;

            Quaternion rotation = getLerpValue(time, body.rotation);
            matrix *= Matrix4::CreateFromQuaternion(rotation);

            Vector3 translate = getLerpValue(time, body.translate);
            matrix *= Matrix4::CreateTranslation(translate);
            // ノードの行列を書き換える
            auto &node = this->node_index.at(body.node_name)->matrix;
            node = matrix;
        }
    }
    // 全ノードの親行列適用済み行列と、その逆行列を計算
    //   メッシュアニメーションで利用
    void Model::UpdateNodeDerivedMatrix(const std::shared_ptr<Node> &node,
                                        const Matrix4 &parent_matrix)
    {
        node->global_matrix = parent_matrix * node->matrix;
        Matrix4 temp = node->global_matrix;
        temp.Invert();
        node->invert_matrix = temp;

        for (auto child : node->children)
        {
            UpdateNodeDerivedMatrix(child, node->global_matrix);
        }
    }

    void Model::UpdateMesh()
    {
        for (const auto &node : node_list)
        {
            for (auto &mesh : node->mesh)
            {
                if (!mesh.has_bone)
                    continue;

                // 座標変換に必要な行列を用意
                std::vector<Matrix4> bone_matrix;
                for (auto &bone : mesh.bones)
                {
                    auto local_node = node_index.at(bone.name);
                    bone_matrix.push_back(node->invert_matrix * local_node->global_matrix * bone.offset);
                }

                // 変換結果を書き出す頂点配列
                auto &v = mesh.body.vertices;
                for (auto &i : v)
                {
                    i.position = Vector3::Zero;
                    i.normal = Vector3::Zero;
                }

                // オリジナルの頂点データ
                const auto &orig_v = mesh.original.vertices;

                // 全頂点の座標を再計算
                for (uint32_t i = 0; i < mesh.bones.size(); ++i)
                {
                    const auto &bone = mesh.bones[i];
                    const auto &m = bone_matrix[i];

                    for (const auto &weight : bone.weights)
                    {
                        auto t1 = Vector3::Transform(orig_v[weight.vertex_id].position, m);
                        auto t2 = Vector3::Transform(orig_v[weight.vertex_id].normal, m);
                        v[weight.vertex_id].position += weight.value * t1;
                        v[weight.vertex_id]
                            .normal += weight.value * t2;
                    }
                }
            }
        }
    }

}