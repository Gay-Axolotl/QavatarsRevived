#pragma once

#include "custom-types/shared/coroutine.hpp"
#include "UnityEngine/AssetBundle.hpp"
#include "UnityEngine/ScriptableObject.hpp"

namespace AssetLib
{
    class ShaderLoader
    {
    public:
        // Not a registered custom-type -- the loaded asset is read via
        // runtime field lookup (see ShaderSOFields), so this only needs to
        // be a plain ScriptableObject reference.
        static SafePtrUnity<UnityEngine::ScriptableObject> shaders;

        static custom_types::Helpers::Coroutine LoadBund();

        static custom_types::Helpers::Coroutine LoadBundleFromMemoryAsync(ArrayW<uint8_t> bytes, UnityEngine::AssetBundle*& out);

        static custom_types::Helpers::Coroutine LoadAssetFromBundleAsync(UnityEngine::AssetBundle* bundle, std::string_view name, System::Type* type, UnityEngine::Object*& out);
    };
};
