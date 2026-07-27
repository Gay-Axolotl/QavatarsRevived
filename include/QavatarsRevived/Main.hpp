#pragma once

#include "scotland2/shared/modloader.h"
#include "beatsaber-hook/shared/utils/logging.hpp"

// Matches the logger pattern used by the original VRM-Qavatars mod
// (a plain constexpr ConstLoggerContext, not a class/method).
constexpr auto VRMLogger = Paper::ConstLoggerContext("QavatarsRevived");

// Called once, as early as possible, before other mods load their game data.
// Register hooks / install here, but don't touch anything scene-dependent yet.
extern "C" void setup(CModInfo* info);

// Called once Unity/BS game data is loaded and it's safe to touch game state.
extern "C" void late_load();
