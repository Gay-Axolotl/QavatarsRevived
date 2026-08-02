#pragma once
#include <string>
#include <fstream>
#include <future>

#include "structure/modelContext.hpp"
#include "structure/VRM/VRMmodelContext.hpp"

#include "UnityEngine/Shader.hpp"

namespace AssetLib
{
    class ModelImporter
    {
        public:
        // Loads any assimp-supported model file and builds: node tree, gameobject
        // hierarchy, armature, and Unity meshes. Works on .vrm files too, but does
        // not parse VRM-specific data (materials/blendshapes/springbones/humanoid
        // mapping) -- use LoadVRM for that.
        static std::future<Structure::ModelContext*> Load(const std::string& filename, bool loadMaterials);

        // .vrm files only. Calls Load() for the base mesh/armature pipeline, then
        // separately parses the glTF binary's JSON chunk to pull out the VRM
        // extension block (VRM 0.0 or 1.0) and blendshape master data.
        //
        // NOTE: this is a reduced port. Humanoid avatar/skeleton mapping, VRIK
        // setup, and springbone generation are not yet wired in here -- see
        // AvatarLoader follow-up work. What this DOES produce: a populated
        // node/armature tree with real Unity meshes and parsed VRM extension
        // data (vrm0/vrm1/blendShapeMaster) attached. Materials are generated
        // separately via Generators::MaterialGenerator once ShaderLoader has
        // finished loading the mToon shader bundle.
        static std::future<Structure::VRM::VRMModelContext*> LoadVRM(const std::string& filename);

        // Set by AssetLib::ShaderLoader once the bundled shader AssetBundle has
        // finished loading (async, at mod startup). Null until then --
        // MaterialGenerator must not run before this is populated.
        static SafePtrUnity<UnityEngine::Shader> mtoon;
    };
};
