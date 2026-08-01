#pragma once

#include "QavatarsRevived/Main.hpp"
#include "custom-types/shared/macros.hpp"
#include "UnityEngine/ScriptableObject.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/GameObject.hpp"

// Reverted to the wiki-documented pattern (bsmg.wiki/modding/quest/custom-types.html):
// "parameters are (namespace, class name, parent class, contents)" -- this IS
// the correct, current custom-types API. The earlier switch to
// DECLARE_CLASS_CODEGEN_INTERFACES was based on a misread of a truncated
// macro definition; that macro's variadic part is for interface types
// (fed into ExtractClasses<...>()), not a field body, which is why it broke.
DECLARE_CLASS_CODEGEN(VRMData, ShaderSO, UnityEngine::ScriptableObject,
    DECLARE_INSTANCE_FIELD(UnityEngine::Shader*, mToonShader);
    DECLARE_INSTANCE_FIELD(UnityEngine::Shader*, mirrorShader);
    DECLARE_INSTANCE_FIELD(UnityEngine::Material*, shadowMaterial);
)
