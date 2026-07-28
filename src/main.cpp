#include "QavatarsRevived/Main.hpp"
#include "QavatarsRevived/AvatarLoader.hpp"
#include "QavatarsRevived/UI/AvatarPicker.hpp"

#include "bsml/shared/BSML.hpp"

static ModInfo modInfo{MOD_ID, VERSION, 0};

extern "C" void setup(CModInfo* info) {
    *info = modInfo.to_c();
    VRMLogger.info("QavatarsRevived setup()");

    // TODO: install function hooks here once ported.
    // Nothing scene/game-state dependent belongs in setup() -- that\'s late_load()\'s job.
}

extern "C" void late_load() {
    VRMLogger.info("QavatarsRevived late_load()");

    // BSML::Init() must run before any BSML::Register::* call.
    BSML::Init();

    QavatarsRevived::InitAvatarLoader();
    QavatarsRevived::UI::RegisterAvatarPicker();
}
