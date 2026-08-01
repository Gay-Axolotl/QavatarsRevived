#pragma once

#include "QavatarsRevived/Main.hpp"
#include "custom-types/shared/macros.hpp"
#include "UnityEngine/ScriptableObject.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/GameObject.hpp"

// Confirmed via direct inspection of the resolved custom-types 0.18.4
// macros.hpp: DECLARE_CLASS_CODEGEN(namespace, name, baseT) here takes
// exactly 3 fixed args -- no body/variadic parameter. (The bsmg.wiki docs
// describe a different/newer custom-types API where the macro does accept
// inline body content; that doesn't match what's actually resolved here.)
// DECLARE_INSTANCE_FIELD itself expands to ordinary statements (a "public:"
// label plus registrator/accessor definitions), so it's meant to be used
// directly inside a normal, separately-opened C++ class body -- not passed
// as a macro argument.
DECLARE_CLASS_CODEGEN(VRMData, ShaderSO, UnityEngine::ScriptableObject);

namespace VRMData {
    class ShaderSO : public UnityEngine::ScriptableObject {
        public:
        DECLARE_INSTANCE_FIELD(UnityEngine::Shader*, mToonShader);
        DECLARE_INSTANCE_FIELD(UnityEngine::Shader*, mirrorShader);
        DECLARE_INSTANCE_FIELD(UnityEngine::Material*, shadowMaterial);
    };
}
