#pragma once

#include "QavatarsRevived/Main.hpp"
#include "UnityEngine/ScriptableObject.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Material.hpp"

// NOTE: this is NOT a registered custom-types class. The bundled shader
// AssetBundle's "Assets/shaders.asset" is a real, existing C# ScriptableObject
// type from the original mod, loaded at runtime via LoadAssetAsync -- we
// don't need to (and shouldn't) redeclare/register a new IL2CPP type for it.
// custom-types' DECLARE_CLASS_CODEGEN + DECLARE_INSTANCE_FIELD macro pattern
// (as used in the original mod, and documented for an older custom-types
// version) doesn't produce a working class body in the resolved
// custom-types 0.18.4 here -- rather than fight that further, this reads
// the three fields at runtime via il2cpp_utils::GetFieldValue, which is
// the standard, verified way to read fields off an arbitrary Il2CppObject*
// by name without declaring a matching C++ type.
namespace VRMData {
    namespace ShaderSOFields {
        UnityEngine::Shader* GetMToonShader(UnityEngine::ScriptableObject* instance);
        UnityEngine::Shader* GetMirrorShader(UnityEngine::ScriptableObject* instance);
        UnityEngine::Material* GetShadowMaterial(UnityEngine::ScriptableObject* instance);
    }
}
