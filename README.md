# Qavatars Revived

Custom VRM avatar support for Beat Saber Quest, targeting **1.40.8**.
This is a ported-and-updated rebuild based on the real
[BSQ-VRM/VRM-Qavatars](https://github.com/BSQ-VRM/VRM-Qavatars) source
(MIT licensed), not a from-scratch reimplementation -- see "How this is being
built" below.

## Status

- [x] QPM project scaffold (`qpm.json`, `mod.template.json`)
- [x] CMake build, GitHub Actions CI
- [x] Core dependency versions cross-checked for 1.40.8
- [x] **Basic VRM avatar loading:** node tree, gameobject hierarchy,
      armature detection, real Unity meshes (vertices, normals, UVs, bone
      weights, blendshape frames), and VRM extension JSON parsing (VRM0/VRM1
      + blendshape master), ported from the original mod\'s `AssetLib`
      pipeline
- [x] **Material/texture generation:** VRM0 mToon materials (real, ported),
      texture decoding from embedded glTF images, the actual bundled shader
      AssetBundle wired in via `kaleb`. VRM1 material generation is a stub
      **in the original mod itself** -- not something cut by this port.
- [x] **Avatar-picker UI (this pass):** scans for `.vrm` files and lets you
      pick one to load. This is a **redesign, not a port** -- the original
      picker (and its whole UI layer) is built on `chatplex-sdk-bs`, a
      separate third-party UI framework this project doesn\'t depend on.
      Rebuilt in plain BSML instead, verified against real Quest-BSML
      source. Two small implementation details (an implicit
      `std::string`->`StringW` conversion, and
      `HMUI::CurvedTextMeshPro::set_text`) are flagged in-code as
      standard-but-not-directly-confirmed -- check those first if the UI
      doesn\'t compile.
- [ ] Settings screen (bloom toggle, etc.) -- the original\'s settings UI is
      also built on `chatplex-sdk-bs`; deferred for the same reason as above,
      to be redesigned in BSML rather than ported
- [ ] Humanoid Avatar / skeleton (Mecanim) bone mapping
- [ ] VRIK + full-body tracking
- [ ] Springbones (hair/cloth physics)
- [ ] Replay-mode support
- [ ] Testing on-device -- **nothing in this repo has been built or run yet**

## How this is being built

The original author\'s source (`BSQ-VRM/VRM-Qavatars`) was provided as a
reference. Most subsystems are ported in stages, kept close to the original
with minimal changes -- mainly updated include paths and dependency versions
for current beatsaber-hook/bs-cordl/custom-types/scotland2 APIs.

**The UI layer is an exception.** The entire original UI (avatar picker,
settings screen) is built on `chatplex-sdk-bs`, a separate third-party
framework with its own view-controller base class and widget library
(`CP_SDK::XUI`). Rather than take on learning and porting against an
unfamiliar SDK\'s full API from partial documentation, the UI is being
**redesigned in plain BSML** -- same end-user functionality, different
(verified) implementation. See `src/UI/AvatarPicker.cpp`.

**What\'s ported so far** (`AssetLib/modelImporter.{hpp,cpp}` and its direct
dependencies): the assimp-based scene load, node/armature tree construction,
Unity mesh generation, VRM extension JSON parsing, and VRM0 material/texture
generation (real mToon materials via the bundled shader AssetBundle).

**Deliberately cut / not yet ported** (see the comment block at the end of
`ModelImporter::LoadVRM` in `src/AssetLib/modelImporter.cpp`): humanoid
`Avatar`/skeleton mapping (`avatarGenerator`), VRIK + `TargetManager` setup,
first/third-person mesh splitting, and springbone generation
(`springBoneGenerator`). Each is its own subsystem and will be ported in
follow-up passes.

**Practical result right now:** the avatar picker will find `.vrm` files
under `sdcard/ModData/com.beatgames.beatsaber/Mods/QavatarsRevived/Avatars`
and load the selected one -- VRM0 avatars will get real mesh + real
materials; VRM1 avatars will load with no material (upstream stub). No
avatar has a working humanoid animation rig yet.

## Build

Requires: Android NDK (r27, per the pinned `ndk` field), Ninja, QPM CLI, CMake >= 3.21.

```sh
qpm restore
qpm s configure
qpm s build
qpm s qmod
```

**Dependency versions:** `beatsaber-hook`, `bs-cordl`, `custom-types`,
`scotland2`, `paper2_scotland2` are cross-checked against
[hardcpp/QBeatSaberPlus-GameTweaker](https://github.com/hardcpp/QBeatSaberPlus-GameTweaker)
(explicitly 1.40.8-targeted). `bsml` (`0.4.55`) is cross-checked against the
live [mods.bsquest.xyz](https://mods.bsquest.xyz/1.40.8_7379/) registry for
1.40.8 (note: some newer mods pin `bsml 0.5.1` -- worth rechecking before a
real build). `assimp`, `sombrero`, `fmt`, `kaleb` version ranges are carried
over from the original VRM-Qavatars `qpm.json` (pre-1.40.8) and have **not**
been reverified -- expect to bump these via `qpm restore`.

## Planned porting order

1. ~~Confirm dependency versions against current QPM registry for 1.40.8~~
2. ~~Avatar file loading (VRM parse -> node/armature/mesh)~~
3. ~~Material/texture generation~~
4. ~~Avatar-picker UI~~ -- this pass (redesigned in BSML, see above)
5. Humanoid skeleton (Avatar) mapping + VRIK
6. Springbones + blendshape/expression wiring
7. Settings UI (BSML redesign, same reasoning as the picker)
8. Replay-mode hook
9. Bug pass / device testing -- first real build+run happens here

## Credits

Ported from [BSQ-VRM/VRM-Qavatars](https://github.com/BSQ-VRM/VRM-Qavatars)
by FutureMapper and contributors (MIT license). UI layer redesigned against
[bsq-ports/Quest-BSML](https://github.com/bsq-ports/Quest-BSML) (MIT
license, RedBrumbler and contributors).
