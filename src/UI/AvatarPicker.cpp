#include "QavatarsRevived/UI/AvatarPicker.hpp"
#include "QavatarsRevived/Main.hpp"
#include "QavatarsRevived/AvatarLoader.hpp"

#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML/Components/CustomListTableData.hpp"

#include <filesystem>
#include <vector>
#include <string>
#include <fmt/format.h>

namespace QavatarsRevived::UI {

    namespace {
        // Cached between DidActivate calls so re-opening the picker doesn't
        // re-scan disk every time; RefreshList() rebuilds it on demand.
        std::vector<std::string> foundAvatarPaths;
        BSML::CustomListTableData* avatarListData = nullptr;
        HMUI::CurvedTextMeshPro* statusText = nullptr;

        std::vector<std::string> ScanForVrmFiles() {
            std::vector<std::string> found;
            std::error_code ec;

            if (!std::filesystem::exists(vrm_avatars_path, ec) || ec) {
                // Directory doesn't exist yet -- not an error, just means the
                // user hasn't dropped any .vrm files in there.
                return found;
            }

            for (const auto& entry : std::filesystem::directory_iterator(vrm_avatars_path, ec)) {
                if (ec) break;
                if (!entry.is_regular_file()) continue;
                auto ext = entry.path().extension().string();
                // case-insensitive .vrm check
                if (ext.size() == 4 && (ext[1] == 'v' || ext[1] == 'V') && (ext[2] == 'r' || ext[2] == 'R') && (ext[3] == 'm' || ext[3] == 'M') && ext[0] == '.') {
                    found.push_back(entry.path().string());
                }
            }
            return found;
        }

        void RefreshList() {
            foundAvatarPaths = ScanForVrmFiles();

            if (!avatarListData) return;

            // NOTE: passing std::string directly where StringW is expected relies
            // on an implicit conversion that's standard in this ecosystem but
            // wasn't directly confirmed against BSML source (StringW itself is a
            // beatsaber-hook type, not part of the BSML headers read for this
            // pass). If this doesn't compile, wrap with StringW(...) explicitly.
            auto cells = ListW<BSML::CustomCellInfo*>::New();
            cells->EnsureCapacity(foundAvatarPaths.size());
            for (const auto& path : foundAvatarPaths) {
                auto filename = std::filesystem::path(path).filename().string();
                cells->Add(BSML::CustomCellInfo::construct(filename, path));
            }
            avatarListData->data = cells;
            avatarListData->tableView->ReloadData();

            if (statusText) {
                if (foundAvatarPaths.empty()) {
                    statusText->set_text(fmt::format("No .vrm files found in {}", vrm_avatars_path));
                } else {
                    statusText->set_text(fmt::format("{} avatar(s) found", foundAvatarPaths.size()));
                }
            }
        }

        void OnAvatarCellClicked(int idx) {
            if (idx < 0 || static_cast<size_t>(idx) >= foundAvatarPaths.size()) return;
            const auto& path = foundAvatarPaths[idx];

            VRMLogger.info("Avatar picker: loading {}", path.c_str());
            if (statusText) {
                statusText->set_text(fmt::format("Loading {}...", std::filesystem::path(path).filename().string()));
            }

            // LoadAvatar() is async (see AvatarLoader.hpp) -- this just kicks
            // it off. There's currently no UI feedback for completion beyond
            // this "Loading..." label and the VRMLogger output; a proper
            // loading-progress/error callback is follow-up work once
            // avatar rendering itself is further along.
            QavatarsRevived::LoadAvatar(path);
        }
    }

    void AvatarPickerDidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        if (!firstActivation) return;

        UnityEngine::GameObject* container = BSML::Lite::CreateScrollableSettingsContainer(self->get_transform());

        BSML::Lite::CreateText(container->get_transform(), "Qavatars Revived - Select an Avatar");

        statusText = BSML::Lite::CreateText(container->get_transform(), "Scanning...");

        avatarListData = BSML::Lite::CreateScrollableList(
            container->get_transform(),
            UnityEngine::Vector2(60.0f, 50.0f),
            &OnAvatarCellClicked
        );

        BSML::Lite::CreateUIButton(container->get_transform(), "Refresh", &RefreshList);

        RefreshList();
    }

    void RegisterAvatarPicker() {
        BSML::Register::RegisterMainMenuViewControllerMethod(
            "Qavatars Revived",
            "VRM Avatars",
            "Pick a .vrm avatar to load",
            &AvatarPickerDidActivate
        );
        VRMLogger.info("Avatar picker registered");
    }
}
