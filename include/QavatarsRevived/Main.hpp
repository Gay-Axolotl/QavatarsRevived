#pragma once

// Called once, as early as possible, before other mods load their game data.
// Register hooks / install here, but don't touch anything scene-dependent yet.
extern "C" void setup(void* info);

// Called once Unity/BS game data is loaded and it's safe to touch game state.
extern "C" void late_load();
