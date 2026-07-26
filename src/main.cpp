#include "QavatarsRevived/Main.hpp"

#include "beatsaber-hook/shared/utils/logging.hpp"
#include "scotland2/shared/modloader.h"

static ModInfo modInfo{MOD_ID, VERSION, 0};

Logger& getLogger() {
    static auto* logger = new Paper::ConstLoggerContext<sizeof("QavatarsRevived") + 20>(MOD_ID);
    return *reinterpret_cast<Logger*>(logger); // placeholder wiring — swap for real Paper2_Scotland2 logger use
}

extern "C" void setup(CModInfo* info) {
    *info = modInfo.to_c();

    // TODO: install function hooks here once ported.
    // Nothing scene/game-state dependent belongs in setup() — that's late_load()'s job.
}

extern "C" void late_load() {
    // TODO: entry point for avatar-loading, UI, and replay-support wiring
    // once those subsystems are ported from a reference implementation.
}
