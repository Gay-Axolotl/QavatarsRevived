#pragma once

#include "QavatarsRevived/Main.hpp"
#include "custom-types/shared/macros.hpp"
#include "UnityEngine/ScriptableObject.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/GameObject.hpp"

// NOTE: This is a temporary minimal version. DECLARE_CLASS_CODEGEN in the
// resolved custom-types 0.18.4 both forward-declares AND fully defines the
// class from its 3 fixed args (namespace, name, baseT) -- passing a body
// (inline or via reopening the class afterward) both failed to compile.
// The exact mechanism this version uses to add DECLARE_INSTANCE_FIELD-style
// fields to a codegen class is not yet confirmed. Since mToonShader/
// mirrorShader/shadowMaterial aren't read anywhere in this project's C++
// code yet (ShaderLoader only needs the ShaderSO type itself to exist for
// LoadAssetFromBundleAsync's type parameter), shipping this without the
// fields unblocks the rest of the build. Revisit when actually wiring up
// mirror/shadow material support.
DECLARE_CLASS_CODEGEN(VRMData, ShaderSO, UnityEngine::ScriptableObject);
