#pragma once

#include "AssetLib/VRMConcepts.hpp"

#include "UnityEngine/Avatar.hpp"
#include "UnityEngine/GameObject.hpp"

#include "AssetLib/structure/node.hpp"

namespace AssetLib::Generators
{
    class AvatarGenerator
    {
        public:
        AvatarGenerator() = default;
        ~AvatarGenerator() = default;

        // Builds a Unity humanoid Avatar from a VRM\'s bone mapping. VRM0 and
        // VRM1 store this differently (VRM0: per-bone struct with a `bone`
        // enum tag; VRM1: named fields per bone) -- hence the two full
        // template specializations rather than one generic implementation.
        template<AssetLib::VRMVersion T>
        UnityEngine::Avatar* Generate(const T& vrm, std::vector<AssetLib::Structure::Node*> bones, UnityEngine::GameObject* root);
    };
};
