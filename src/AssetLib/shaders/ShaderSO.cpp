#include "AssetLib/shaders/ShaderSO.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

namespace VRMData::ShaderSOFields {
    UnityEngine::Shader* GetMToonShader(UnityEngine::ScriptableObject* instance) {
        auto result = il2cpp_utils::GetFieldValue<UnityEngine::Shader*>(instance, "mToonShader");
        return result.has_value() ? result.value() : nullptr;
    }

    UnityEngine::Shader* GetMirrorShader(UnityEngine::ScriptableObject* instance) {
        auto result = il2cpp_utils::GetFieldValue<UnityEngine::Shader*>(instance, "mirrorShader");
        return result.has_value() ? result.value() : nullptr;
    }

    UnityEngine::Material* GetShadowMaterial(UnityEngine::ScriptableObject* instance) {
        auto result = il2cpp_utils::GetFieldValue<UnityEngine::Material*>(instance, "shadowMaterial");
        return result.has_value() ? result.value() : nullptr;
    }
}
