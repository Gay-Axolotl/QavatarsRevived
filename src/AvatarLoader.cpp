#include "QavatarsRevived/AvatarLoader.hpp"
#include "QavatarsRevived/Main.hpp"
#include "AssetLib/modelImporter.hpp"
#include "AssetLib/shaders/shaderLoader.hpp"

#include "bsml/shared/BSML/SharedCoroutineStarter.hpp"

namespace QavatarsRevived {
    void InitAvatarLoader() {
        VRMLogger.info("Avatar loader ready (basic loading only -- see AvatarLoader.hpp)");

        // Kick off loading the embedded mToon shader bundle now, so it\'s
        // ready by the time anyone calls LoadAvatar(). Materials generated
        // before this finishes would silently get a null shader.
        BSML::SharedCoroutineStarter::get_instance()->StartCoroutine(
            custom_types::Helpers::CoroutineHelper::New(AssetLib::ShaderLoader::LoadBund())
        );
    }

    std::future<AssetLib::Structure::VRM::VRMModelContext*> LoadAvatar(const std::string& filePath) {
        VRMLogger.info("Loading avatar: {}", filePath.c_str());
        return AssetLib::ModelImporter::LoadVRM(filePath);
    }
}
