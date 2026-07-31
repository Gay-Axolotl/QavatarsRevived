#pragma once

#include "QavatarsRevived/Main.hpp"
#include "custom-types/shared/macros.hpp"
#include "UnityEngine/ScriptableObject.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/GameObject.hpp"

// NOTE: DECLARE_CLASS_CODEGEN in this custom-types version (0.18.3) only
// takes 3 fixed args (namespace, name, baseT) -- no inline body/variadic
// field-declaration block, unlike the pattern shown in the BSMG wiki and
// porting guide (which describe an older custom-types API). Using
// DECLARE_CLASS_CODEGEN_INTERFACES instead, which does support a trailing
// variadic block, passing zero interfaces.
DECLARE_CLASS_CODEGEN_INTERFACES(VRMData, ShaderSO, UnityEngine::ScriptableObject,
    DECLARE_INSTANCE_FIELD(UnityEngine::Shader*, mToonShader);
    DECLARE_INSTANCE_FIELD(UnityEngine::Shader*, mirrorShader);
    DECLARE_INSTANCE_FIELD(UnityEngine::Material*, shadowMaterial);
)
