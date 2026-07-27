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

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "json.hpp"

// NOTE ON THIS FILE: this is a reduced port of VRM-Qavatars\' modelImporter.cpp.
// It keeps the node-tree/armature/mesh-building pipeline close to the original
// (tested, working logic -- minimal changes only), but the tail end of the
// original LoadVRM() -- material generation, humanoid Avatar/skeleton mapping,
// VRIK setup, springbone generation, first/third-person mesh splitting -- is
// cut. Those land in follow-up passes once their own subsystems are ported.
// What DOES work after this file: loading a VRM\'s node tree, armature, real
// Unity meshes (with blendshapes/bone weights), and the parsed VRM0/VRM1
// extension JSON + blendshape master, all attached to VRMModelContext.

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
        VRMLogger.info("{}{}", std::string(depth, \'-\'), node->mName.C_Str());
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

            // --- Reduced port stops here ---
            // The original mod continues on the main thread from this point to:
            //   1. Decode embedded image buffers + generate Unity textures
            //   2. Generate VRM (MToon) materials and assign them to renderers
            //   3. Build a Unity humanoid Avatar from the VRM bone mapping (VRM0/VRM1)
            //   4. Add an Animator, fix crossed-leg rest pose, add VRIK + TargetManager
            //   5. Split first-person/third-person meshes (erase head-descendant bones
            //      for the first-person copy)
            //   6. Generate VRM springbones
            // None of that is wired in yet -- modelContext at this point has a real
            // node/armature/mesh tree and parsed VRM extension data, but no materials,
            // no humanoid Avatar, and no physics/expression systems. See AvatarLoader.

            return modelContext;
        });
    }
}
