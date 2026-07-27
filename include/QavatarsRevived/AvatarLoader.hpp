#pragma once

#include <string>
#include <future>

#include "AssetLib/structure/VRM/VRMmodelContext.hpp"

namespace QavatarsRevived {
    // Called from late_load(); currently just logs readiness. No file-loading
    // is triggered automatically yet -- that will come with the UI/avatar
    // selection pass.
    void InitAvatarLoader();

    // Loads a .vrm file's node tree, armature, and real Unity meshes, plus
    // parsed VRM extension data (vrm0/vrm1/blendShapeMaster).
    //
    // NOT YET DONE by this call: no materials are generated (meshes will
    // render with Unity's default/missing material), no humanoid Avatar or
    // skeleton mapping is built (so animation/IK won't work), and no
    // springbones/VRIK/expression systems are set up. This is the "basic
    // loading" milestone -- see AssetLib/modelImporter.cpp for exactly what's
    // deferred and why.
    std::future<AssetLib::Structure::VRM::VRMModelContext*> LoadAvatar(const std::string& filePath);
}
