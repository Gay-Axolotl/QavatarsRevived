#pragma once

#include "scotland2/shared/modloader.h"
#include "beatsaber-hook/shared/utils/logging.hpp"

// Matches the logger pattern used by the original VRM-Qavatars mod
// (a plain constexpr ConstLoggerContext, not a class/method).
constexpr auto VRMLogger = Paper::ConstLoggerContext("QavatarsRevived");

// Mirrors the original VRM-Qavatars mod's directory convention, under this
// mod's own folder name so it doesn't collide with a real VRM-Qavatars install
// on the same device.
constexpr const char* vrm_avatars_path = "sdcard/ModData/com.beatgames.beatsaber/Mods/QavatarsRevived/Avatars";

// Called once, as early as possible, before other mods load their game data.
// Register hooks / install here, but don't touch anything scene-dependent yet.
extern "C" void setup(CModInfo* info);

// Called once Unity/BS game data is loaded and it's safe to touch game state.
extern "C" void late_load();
