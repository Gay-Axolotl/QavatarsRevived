#include "AssetLib/generators/meshGenerator.hpp"

#include "UnityEngine/SkinnedMeshRenderer.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/MeshFilter.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Matrix4x4.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Color.hpp"

#include "AssetLib/arrayUtils.hpp"

namespace AssetLib::Generators
{
    void MeshGenerator::Generate(const AssetLib::Structure::Node* node, const AssetLib::Structure::ModelContext* context)
    {
        if(node->mesh.has_value())
        {
            auto mesh = node->mesh.value();

            UnityEngine::Mesh* unityMesh = UnityEngine::Mesh::New_ctor();
            unityMesh->set_name(node->name);
            unityMesh->set_indexFormat(mesh.vertices.size() > 65535 ? UnityEngine::Rendering::IndexFormat::UInt32 : UnityEngine::Rendering::IndexFormat::UInt16);

            // NOTE: Sombrero::Fast* types are math-optimized wrappers that behave
            // like their UnityEngine:: counterparts in expressions, but ArrayW<T>
            // has no cross-type conversion even when the element types are field-
            // compatible -- these calls need the explicit converting ToArrayW
            // overload, constructing a real UnityEngine:: value per element.
            unityMesh->set_vertices(ArrayUtils::ToArrayW<UnityEngine::Vector3>(mesh.vertices,
                [](const Sombrero::FastVector3& v) { return UnityEngine::Vector3(v.x, v.y, v.z); }));
            unityMesh->set_normals(ArrayUtils::ToArrayW<UnityEngine::Vector3>(mesh.normals,
                [](const Sombrero::FastVector3& v) { return UnityEngine::Vector3(v.x, v.y, v.z); }));
            unityMesh->set_tangents(ArrayUtils::ToArrayW(mesh.tangents));
            unityMesh->set_uv(ArrayUtils::ToArrayW<UnityEngine::Vector2>(mesh.uv1,
                [](const Sombrero::FastVector2& v) { return UnityEngine::Vector2(v.x, v.y); }));
            unityMesh->set_uv2(ArrayUtils::ToArrayW<UnityEngine::Vector2>(mesh.uv2,
                [](const Sombrero::FastVector2& v) { return UnityEngine::Vector2(v.x, v.y); }));
            unityMesh->set_uv3(ArrayUtils::ToArrayW<UnityEngine::Vector2>(mesh.uv3,
                [](const Sombrero::FastVector2& v) { return UnityEngine::Vector2(v.x, v.y); }));
            unityMesh->set_uv4(ArrayUtils::ToArrayW<UnityEngine::Vector2>(mesh.uv4,
                [](const Sombrero::FastVector2& v) { return UnityEngine::Vector2(v.x, v.y); }));
            unityMesh->set_colors(ArrayUtils::ToArrayW<UnityEngine::Color>(mesh.colors,
                [](const Sombrero::FastColor& c) { return UnityEngine::Color(c.r, c.g, c.b, c.a); }));

            std::vector<UnityEngine::BoneWeight> convertedBW = std::vector<UnityEngine::BoneWeight>(mesh.boneWeights.size());

            for (size_t i = 0; i < mesh.boneWeights.size(); i++)
            {
                convertedBW[i] = mesh.boneWeights[i].convert();
            }

            unityMesh->set_boneWeights(ArrayUtils::ToArrayW(convertedBW));
            unityMesh->set_subMeshCount(mesh.indices.size());
            uint baseVertex = 0;
            for (int i = 0; i < mesh.indices.size(); i++)
            {
                const ArrayW<int> indices = ArrayUtils::ToArrayW(mesh.indices[i]);
                unityMesh->SetIndices(indices, mesh.topology[i], i, false, static_cast<int>(baseVertex));
                baseVertex += mesh.vertexCounts[i];
            }
            unityMesh->RecalculateBounds();

            for (int i = 0; i < mesh.morphTargetNames.size(); ++i)
            {
                auto name = mesh.morphTargetNames[i];
                auto [vertices, normals, tangents] = mesh.morphTargetInfos[i];
                if(unityMesh->GetBlendShapeIndex(name) == -1 && !System::String::IsNullOrWhiteSpace(name))
                {
                    unityMesh->AddBlendShapeFrame(name, 100,
                        ArrayUtils::ToArrayW<UnityEngine::Vector3>(vertices,
                            [](const Sombrero::FastVector3& v) { return UnityEngine::Vector3(v.x, v.y, v.z); }),
                        ArrayUtils::ToArrayW<UnityEngine::Vector3>(normals,
                            [](const Sombrero::FastVector3& v) { return UnityEngine::Vector3(v.x, v.y, v.z); }),
                        nullptr
                    );
                }
            }

            if(context->isSkinned)
            {
                const auto renderer = node->gameObject->AddComponent<UnityEngine::SkinnedMeshRenderer*>();
                renderer->set_updateWhenOffscreen(true);
                renderer->set_allowOcclusionWhenDynamic(false);
                const auto armature = context->armature.value();

                std::vector<UnityEngine::Matrix4x4> bindPoses = std::vector<UnityEngine::Matrix4x4>(context->nodes.size());

                for (size_t i = 0; i < context->nodes.size(); i++)
                {
                    const auto ctxNode = context->nodes[i];
                    if(ctxNode->isBone)
                    {
                        bindPoses[i] = UnityEngine::Matrix4x4::op_Multiply(ctxNode->gameObject->get_transform()->get_worldToLocalMatrix(), renderer->get_transform()->get_localToWorldMatrix());
                    }
                } 

                unityMesh->set_bindposes(ArrayUtils::ToArrayW(bindPoses));

                renderer->set_rootBone(armature.rootBone->gameObject->get_transform());

                auto bones = ArrayUtils::Select<UnityEngine::Transform*>(context->nodes,
                                        [](const AssetLib::Structure::Node* libNode){
                                            return libNode->gameObject->get_transform();
                                        }
                                    );
                renderer->set_bones(ArrayUtils::ToArrayW(bones));
                renderer->set_sharedMesh(unityMesh);
                unityMesh->RecalculateBounds();
            }
            else
            {
                const auto filter = node->gameObject->AddComponent<UnityEngine::MeshFilter*>();
                node->gameObject->AddComponent<UnityEngine::MeshRenderer*>();
                filter->set_sharedMesh(unityMesh);
            }
        }
    }
}
