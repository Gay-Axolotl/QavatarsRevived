#pragma once

#include "HMUI/ViewController.hpp"

namespace QavatarsRevived::UI {
    // Registers the avatar-picker as a main-menu button + view controller.
    // Call once from late_load().
    //
    // This is a redesign, not a port: the original VRM-Qavatars mod's avatar
    // picker (and its whole UI layer) is built on chatplex-sdk-bs, a separate
    // third-party UI framework this project doesn't depend on. This version
    // does the same job -- list .vrm files, let the user pick one, load it --
    // using plain BSML instead, verified against the real Quest-BSML source
    // (BSML-Lite/Creation/{Lists,Buttons,Layout,Text}.hpp).
    void RegisterAvatarPicker();

    // The registered callback BSML calls when the picker's view controller
    // is shown. Builds the UI on first activation; matches the signature
    // BSML::Register::RegisterMainMenuViewControllerMethod expects.
    void AvatarPickerDidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
}
