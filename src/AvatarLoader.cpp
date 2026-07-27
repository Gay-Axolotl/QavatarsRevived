#include "QavatarsRevived/AvatarLoader.hpp"
#include "QavatarsRevived/Main.hpp"
#include "AssetLib/modelImporter.hpp"

namespace QavatarsRevived {
    void InitAvatarLoader() {
        VRMLogger.info("Avatar loader ready (basic loading only -- see AvatarLoader.hpp)");
    }

    std::future<AssetLib::Structure::VRM::VRMModelContext*> LoadAvatar(const std::string& filePath) {
        VRMLogger.info("Loading avatar: {}", filePath.c_str());
        return AssetLib::ModelImporter::LoadVRM(filePath);
    }
}
