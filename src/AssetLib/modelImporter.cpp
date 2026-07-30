#include "AssetLib/modelImporter.hpp"

#include "QavatarsRevived/Main.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include <assimp/scene.h>
#include <functional>
#include <future>
#include <thread>

#include "AssetLib/generators/meshGenerator.hpp"
#include "AssetLib/generators/intermediateMeshGenerator.hpp"
#include "AssetLib/generators/textureGenerator.hpp"
#include "AssetLib/generators/materialGenerator.hpp"
#include "AssetLib/generators/avatarGenerator.hpp"

#include "MaterialTracker.hpp"

#include "UnityEngine/Animator.hpp"
#include "UnityEngine/AnimatorCullingMode.hpp"
#include "UnityEngine/HumanBodyBones.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "json.hpp"

namespace AssetLib
{
    SafePtrUnity<UnityEngine::Shader> ModelImporter::mtoon;
}

// NOTE ON THIS FILE: this is a reduced port of VRM-Qavatars' modelImporter.cpp.
// It keeps the node-tree/armature/mesh-building pipeline close to the original
// (tested, working logic -- minimal changes only). Material/texture generation
// is now wired in too (see the end of LoadVRM below), but humanoid Avatar/
// skeleton mapping, VRIK setup, springbone generation, and first/third-person
// mesh splitting are still cut -- those land in follow-up passes once their
// own subsystems are ported. What DOES work after this file: loading a VRM's
// node tree, armature, real Unity meshes (with blendshapes/bone weights),
// parsed VRM0/VRM1 extension JSON + blendshape master, and (once
// ShaderLoader has finished loading the mToon shader bundle) real VRM0
// materials assigned to renderers -- VRM1 material generation is a stub in
// the original mod itself, not something cut by this port; see
// MaterialGenerator::Generate<VRMC_VRM_1_0::Vrm>.

namespace AssetLib
{
    std::future<aiScene const*> LoadScene(std::string const& fileName) {
        return std::async(std::launch::async, [fileName](){
            static auto importer = Assimp::Importer();
            auto scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_PopulateArmatureData | aiProcess_MakeLeftHanded);
            VRMLogger.info("Loaded scene: {}", scene != nullptr ? "true" : "false");
            return scene;
        });
    }

    bool anyChildMeshesAssimp(aiNode* node)
    {
        bool anymesh = false;
        for (size_t i = 0; i < node->mNumChildren; i++)
        {
            anymesh |= node->mChildren[i]->mNumMeshes > 0;
        }
        return anymesh;
    }

    void IterateNode(aiNode* assnode, AssetLib::Structure::Node* parentNode, AssetLib::Structure::ModelContext* context)
    {
        const auto node = new AssetLib::Structure::Node(std::string(assnode->mName.C_Str()), {}, parentNode, false, false, Sombrero::FastVector3(assnode->mTransformation.a4, assnode->mTransformation.b4, assnode->mTransformation.c4), std::nullopt, assnode);

        if(assnode->mNumMeshes < 1 && !anyChildMeshesAssimp(assnode))
        {
            node->isBone = true;
            context->armature.value().bones.push_back(node);
        }

        if(parentNode == nullptr)
        {
            context->rootNode = node;
        }
        else
        {
            parentNode->children.push_back(node);
        }

        context->nodes.push_back(node);
        for (size_t i = 0; i < assnode->mNumChildren; i++)
        {
            IterateNode(assnode->mChildren[i], node, context);
        }
    }

    void CreateNodeTreeObject(AssetLib::Structure::Node* node)
    {
        if(node->gameObject == nullptr)
        {
            node->gameObject = UnityEngine::GameObject::New_ctor(node->name);
            if(node->parent != nullptr && node->parent->gameObject != nullptr)
            {
                auto trans = node->gameObject->get_transform();
                trans->SetParent(node->parent->gameObject->get_transform());
            }
            node->gameObject->get_transform()->set_localPosition(node->position);
            node->processed = true;
        }
        for (size_t i = 0; i < node->children.size(); i++)
        {
            CreateNodeTreeObject(node->children[i]);
        }
    }

    bool anyChildMeshes(AssetLib::Structure::Node* node)
    {
        bool anymesh = false;
        for (size_t i = 0; i < node->children.size(); i++)
        {
            anymesh |= !node->isBone;
        }
        return anymesh;
    }

    void logAssimpNode(aiNode* node, int depth)
    {
        VRMLogger.info("{}{}", std::string(depth, '-'), node->mName.C_Str());
        for (size_t i = 0; i < node->mNumChildren; i++)
        {
            logAssimpNode(node->mChildren[i], depth + 1);
        }
    }

    std::future<Structure::ModelContext*> ModelImporter::Load(const std::string& filename, bool loadMaterials)
    {
        return std::async(std::launch::async, [filename](){
            VRMLogger.info("Loading model: {}", filename.c_str());
            auto load = LoadScene(filename);
            load.wait();
            auto scene = load.get();
            VRMLogger.info("Loaded scene {}", scene != nullptr ? "true" : "false");

            auto modelContext = new Structure::ModelContext();
            modelContext->fileName = filename;
            modelContext->originalScene = scene;

            if (!scene)
            {
                modelContext = nullptr;
                return modelContext;
            }

            bool finishedMeshes = false;
            BSML::MainThreadScheduler::Schedule([modelContext, &finishedMeshes, scene, filename](){
                VRMLogger.info("moving to mainthread...");
                const auto Root = UnityEngine::GameObject::New_ctor("ROOT");
                UnityEngine::GameObject::DontDestroyOnLoad(Root);
                Root->get_transform()->set_position(Sombrero::FastVector3(0.0f, 0.0f, 0.0f));
                Root->get_transform()->set_rotation(UnityEngine::Quaternion::Euler(0.0f, 0.0f, 0.0f));
                Root->get_transform()->set_localScale(Sombrero::FastVector3(1.0f, 1.0f, 1.0f));
                modelContext->rootGameObject = Root;

                modelContext->armature = Structure::Armature();

                // STEP ONE: Create initial node tree
                IterateNode(scene->mRootNode, nullptr, modelContext);
                modelContext->rootNode->gameObject = Root;
                modelContext->rootNode->processed = true;

                // STEP TWO: Create gameobjects for each node
                CreateNodeTreeObject(modelContext->rootNode);

                // STEP THREE: Load in armature
                Structure::Node* armatureNode = nullptr;

                // TODO (from original): perform this how the assimp docs say to
                for (size_t i = 1; i < modelContext->nodes.size(); i++)
                {
                    const auto node = modelContext->nodes[i];
                    if(node->children.size() > 0 && !anyChildMeshes(node))
                    {
                        modelContext->isSkinned = true;
                        armatureNode = node;
                        break;
                    }
                }
                modelContext->armature.value().rootBone = armatureNode;

                // STEP FOUR: Load in meshes
                auto intermediateMeshGenerator = Generators::IntermediateMeshGenerator();
                auto meshGenerator = Generators::MeshGenerator();

                for (size_t i = 0; i < modelContext->nodes.size(); i++)
                {
                    const auto node = modelContext->nodes[i];
                    if(node->originalNode->mNumMeshes > 0)
                    {
                        node->isBone = false;
                        for (size_t x = 0; x < node->originalNode->mNumMeshes; x++)
                        {
                            if(x == 0)
                            {
                                node->mesh = intermediateMeshGenerator.Generate(scene->mMeshes[node->originalNode->mMeshes[x]], modelContext);
                            }
                            else
                            {
                                node->mesh = intermediateMeshGenerator.Generate(scene->mMeshes[node->originalNode->mMeshes[x]], modelContext, node->mesh);
                            }
                        }
                    }
                }

                // STEP FIVE: Generate Unity meshes
                for (size_t i = 0; i < modelContext->nodes.size(); i++)
                {
                    const auto node = modelContext->nodes[i];
                    if(node->originalNode->mNumMeshes > 0)
                    {
                        meshGenerator.Generate(node, modelContext);
                    }
                }

                finishedMeshes = true;
                VRMLogger.info("Finished loading model: {}", filename.c_str());
            });

            while(!finishedMeshes)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            return modelContext;
        });
    }

    std::vector<UnityEngine::Transform*> Ancestors(UnityEngine::Transform* root)
    {
        std::vector<UnityEngine::Transform*> ancestors;
        UnityEngine::Transform* currentTrans = root;
        while(currentTrans != nullptr)
        {
            ancestors.push_back(currentTrans);
            currentTrans = currentTrans->get_parent();
        }
        return ancestors;
    }

    void SetXLocalRot(UnityEngine::Transform* trans, const float x)
    {
        const auto rot = trans->get_localRotation().get_eulerAngles();
        trans->set_localRotation(UnityEngine::Quaternion::Euler(x, rot.y, rot.z));
    }

    std::future<Structure::VRM::VRMModelContext*> ModelImporter::LoadVRM(const std::string& filename)
    {
        return il2cpp_utils::il2cpp_async(std::launch::async, [filename](){
            Structure::VRM::VRMModelContext* modelContext = nullptr;

            auto load = Load(filename, false);
            load.wait();
            auto originalContext = load.get();
            if (!originalContext)
            {
                return modelContext;
            }
            VRMLogger.info("Loaded model");

            modelContext = new Structure::VRM::VRMModelContext(std::move(*originalContext));

            // Load in binary to parse out VRM extension data (glTF binary
            // container: 12-byte header, then a JSON chunk header + JSON chunk).
            std::ifstream binFile(filename, std::ios::binary);

            binFile.seekg(12); // skip past the 12 byte glb header, to the JSON chunk header
            uint32_t jsonLength;
            binFile.read(reinterpret_cast<char*>(&jsonLength), sizeof(uint32_t));

            std::string jsonStr;
            jsonStr.resize(jsonLength);
            binFile.seekg(20); // skip the rest of the chunk header to the start of the JSON
            binFile.read(jsonStr.data(), jsonLength);

            auto doc = nlohmann::json::parse(jsonStr);
            std::optional<VRMC_VRM_0_0::Vrm> vrm;
            std::optional<VRMC_VRM_1_0::Vrm> vrm1;

            auto exts = doc["extensions"];
            if(exts.contains("VRM"))
            {
                VRMC_VRM_0_0::Vrm vrm0;
                from_json(exts["VRM"], vrm0);
                vrm = vrm0;
                modelContext->vrm0 = vrm;
            }

            if(exts.contains("VRMC_vrm"))
            {
                VRMC_VRM_1_0::Vrm vrm10;
                from_json(exts["VRMC_vrm"], vrm10);
                vrm1 = vrm10;
                modelContext->vrm1 = vrm1;
            }

            VRMLogger.info("Parsed VRM data");

            modelContext->blendShapeMaster = vrm.has_value()
                ? Structure::VRM::VRMBlendShapeMaster::LoadFromVRM0(vrm.value())
                : (vrm1.has_value() ? Structure::VRM::VRMBlendShapeMaster::LoadFromVRM1(vrm1.value()) : nullptr);

            // Decode embedded image buffers (glTF images/bufferViews reference
            // byte ranges inside the same binary chunk the JSON came from).
            auto images = doc["images"];
            auto bufferViews = doc["bufferViews"];

            auto* textures = new ArrayW<uint8_t>[images.size()];
            for (int i = 0; i < images.size(); i++)
            {
                auto img = images[i];
                auto bufferView = bufferViews[img["bufferView"].get<uint32_t>()];

                const uint32_t size = bufferView["byteLength"].get<uint32_t>();
                const uint32_t start = bufferView["byteOffset"].get<uint32_t>();
                std::string thing;
                thing.resize(size);
                binFile.seekg(28 + jsonLength + start);
                binFile.read(thing.data(), size);

                const auto data = thing.data();
                auto ret = ArrayW<uint8_t>(thing.size());
                for (size_t x = 0; x < thing.size(); x++)
                {
                    ret[x] = data[x];
                }

                textures[i] = ret;
            }

            bool finishedMaterials = false;
            BSML::MainThreadScheduler::Schedule([modelContext, images, vrm, textures, &finishedMaterials](){
                VRMLogger.info("Generating textures/materials on mainthread...");

                // ShaderLoader::LoadBund() must have finished before this runs,
                // or ModelImporter::mtoon.ptr() will still be null and every
                // generated Material will silently use a null shader. Start
                // that coroutine once, early, at mod init (see AvatarLoader).
                auto textureGenerator = Generators::TextureGenerator();
                UnityEngine::Texture2D** unityTextures = textureGenerator.Generate(textures, images.size());

                delete[] textures;

                std::vector<UnityEngine::Material*> materials;
                VRMQavatars::MaterialTracker::materials.clear();

                auto materialGenerator = Generators::MaterialGenerator();
                if (vrm.has_value())
                {
                    for (size_t i = 0; i < vrm.value().materialProperties.size(); i++)
                    {
                        auto mat = materialGenerator.Generate(vrm.value(), i, unityTextures);
                        materials.push_back(mat);
                        VRMQavatars::MaterialTracker::materials.push_back(mat);
                    }
                }
                // NOTE: VRM1-only files (vrm.has_value() == false) get an empty
                // materials list here -- MaterialGenerator's VRM1 overload is a
                // stub upstream (see materialGenerator.cpp), so there is nothing
                // real to generate yet. Meshes will render with Unity's default
                // material until that's implemented.
                VRMQavatars::MaterialTracker::UpdateMaterials();

                for (size_t i = 0; i < modelContext->nodes.size(); i++)
                {
                    if(auto node = modelContext->nodes[i]; node->mesh.has_value() && node->processed && !materials.empty())
                    {
                        auto mesh = node->mesh.value();
                        auto matArray = ArrayW<UnityEngine::Material*>(mesh.materialIdxs.size());
                        for (size_t k = 0; k < mesh.materialIdxs.size(); k++)
                        {
                            matArray[k] = materials[mesh.materialIdxs[k]];
                        }

                        if (auto renderer = node->gameObject->GetComponent<UnityEngine::SkinnedMeshRenderer*>())
                        {
                            renderer->set_sharedMaterials(matArray);
                        }
                        // Non-skinned meshes (MeshRenderer, not SkinnedMeshRenderer)
                        // aren't material-assigned here yet -- the original mod
                        // doesn't handle that case either at this point in the
                        // pipeline (it's covered later by the first/third-person
                        // mesh-splitting step, which isn't ported).
                    }
                }

                // Build a Unity humanoid Avatar from the VRM's bone mapping and
                // attach an Animator using it. This is what makes the mesh a
                // "humanoid" Unity can animate/IK against -- without it, VRIK,
                // (next pass) has nothing to attach to.
                auto avatarGenerator = Generators::AvatarGenerator();
                auto avatar = vrm.has_value()
                    ? avatarGenerator.Generate(vrm.value(), modelContext->nodes, modelContext->armature.value().rootBone->gameObject)
                    : avatarGenerator.Generate(vrm1.value(), modelContext->nodes, modelContext->armature.value().rootBone->gameObject);

                auto anim = modelContext->rootGameObject->AddComponent<UnityEngine::Animator*>();
                anim->set_cullingMode(UnityEngine::AnimatorCullingMode::AlwaysAnimate);
                anim->set_avatar(avatar);

                // Fix crossed legs -- VRM's rest pose and Unity's Mecanim rest
                // pose disagree slightly on leg rotation, which without this
                // correction makes the legs render crossed/twisted.
                auto LUleg = anim->GetBoneTransform(UnityEngine::HumanBodyBones::LeftUpperLeg);
                auto RUleg = anim->GetBoneTransform(UnityEngine::HumanBodyBones::RightUpperLeg);
                auto LLleg = anim->GetBoneTransform(UnityEngine::HumanBodyBones::LeftLowerLeg);
                auto RLleg = anim->GetBoneTransform(UnityEngine::HumanBodyBones::RightLowerLeg);

                SetXLocalRot(LUleg, -4.0f);
                SetXLocalRot(RUleg, -4.0f);
                SetXLocalRot(LLleg, 4.0f);
                SetXLocalRot(RLleg, 4.0f);

                finishedMaterials = true;
                VRMLogger.info("Finished generating textures/materials and humanoid Avatar");
            });

            while(!finishedMaterials)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            // --- Reduced port stops here ---
            // The original mod continues on the main thread from this point to:
            //   1. Add VRIK + TargetManager (full-body IK)
            //   2. Split first-person/third-person meshes (erase head-descendant bones
            //      for the first-person copy)
            //   3. Generate VRM springbones
            // None of that is wired in yet -- modelContext at this point has a real
            // node/armature/mesh tree, parsed VRM extension data, (for VRM0 files,
            // once ShaderLoader has finished) real materials on skinned renderers,
            // AND a working humanoid Avatar + Animator -- so a plain Animator
            // Controller could already drive this avatar's arms/legs/head. What's
            // still missing is full-body IK tracking (VRIK) and physics/expression
            // systems. See AvatarLoader.

            return modelContext;
        });
    }
}
