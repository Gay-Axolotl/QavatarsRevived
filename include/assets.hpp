#pragma once
#include "kaleb/shared/kaleb.hpp"

// Only the shader bundle is declared here -- the original mod also embeds
// several UI PNGs (avatar picker thumbnails, icons) via this same mechanism,
// but those belong to the settings/avatar-picker UI, which hasn\'t been
// ported yet. Add them here alongside that work instead of speculatively
// now.
DECLARE_FILE(_binary_shaders_sbund, Assets, shaders_sbund);
