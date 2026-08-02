#include "MaterialTracker.hpp"

#include "QavatarsRevived/Main.hpp"

// NOTE: the original mod also calls VRMQavatars::MirrorManager::UpdateMirror()
// here if a mirror is active. MirrorManager (in-game mirror rendering) is a
// separate, unrelated feature that hasn't been ported -- it's not needed for
// materials to work, so that call is left out rather than faked.

namespace VRMQavatars
{
    bool MaterialTracker::bloomEnabled = false;
    std::vector<SafePtrUnity<UnityEngine::Material>> MaterialTracker::materials;

    void MaterialTracker::UpdateMaterials()
    {
        VRMLogger.info("updating materials");
        for(auto material : materials)
        {
            if(material)
            {
                VRMLogger.info("valid material {} {}", static_cast<std::string>(material->get_name()).c_str(), static_cast<std::string>(material->shader->get_name()).c_str());
                material->SetInt("_BloomSupport", bloomEnabled ? 1 : 0);
                material->SetInt("_ColorMask", bloomEnabled ? 14 : 15);
            }
        }
    }
}
