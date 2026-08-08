# Qavatars Revived

Custom VRM avatar support for Beat Saber Quest, targeting **1.40.8**.
This is a ported-and-updated rebuild based on the real
[BSQ-VRM/VRM-Qavatars](https://github.com/BSQ-VRM/VRM-Qavatars) source
(MIT licensed), not a from-scratch reimplementation — see "How this is being
built" below.

## Status

- [x] QPM project scaffold (`qpm.json`, `mod.template.json`)
- [x] CMake build, GitHub Actions CI
- [x] Core dependency versions cross-checked for 1.40.8
- [x] **Basic VRM avatar loading:** node tree, gameobject hierarchy,
      armature detection, real Unity meshes (vertices, normals, UVs, bone
      weights, blendshape frames), and VRM extension JSON parsing (VRM0/VRM1
      + blendshape master), ported from the original mod's `AssetLib`
      pipeline
- [x] **Material/texture generation:** VRM0 mToon materials (real, ported),
      texture decoding from embedded glTF images, the actual bundled shader
      AssetBundle wired in via `kaleb`. VRM1 material generation is a stub
      **in the original mod itself** — not something cut by this port.
- [x] **Avatar-picker UI:** scans for `.vrm` files and lets you pick one to
      load. This is a **redesign, not a port** — the original picker (and
      its whole UI layer) is built on `chatplex-sdk-bs`, a separate
      third-party UI framework this project doesn't depend on. Rebuilt in
      plain BSML instead, verified against real Quest-BSML source.
- [x] **Humanoid Avatar / skeleton mapping (this pass):** VRM0/VRM1 bone
      mapping into a Unity Mecanim `HumanDescription`, built via the
      `AvatarBuilder::BuildHumanAvatarInternal_Injected` icall, attached to
      an `Animator`, with the crossed-leg rest-pose fix. Ported verbatim
      from `avatarGenerator.cpp` — dense, tested bone-name-mapping logic
      where inventing a substitute would be actively wrong.
- [ ] VRIK + full-body tracking (the humanoid Avatar/Animator from this pass
      is what VRIK will attach to)
- [ ] Springbones (hair/cloth physics)
- [ ] First/third-person mesh splitting
- [ ] Settings screen (bloom toggle, etc.) — the original's settings UI is
      also built on `chatplex-sdk-bs`; deferred for the same reason as the
      picker, to be redesigned in BSML rather than ported
- [ ] Replay-mode support
- [ ] Testing on-device — **nothing in this repo has been built or run yet**

## Current build status

**The `.so` compiles and links successfully as of this pass.** The multi-session
linker mystery (`undefined hidden symbol` for `Transform::SetParent` etc.) is
resolved -- confirmed by diffing against a real, working, confirmed-1.40.8 mod's
actual `CMakeLists.txt` (`hardcpp/QBeatSaberPlus-GameTweaker`). The fix was:
`project(${COMPILE_ID})` sourced from `qpm_defines.cmake` (included as line 1,
before `project()`), `-fdeclspec -fvisibility=hidden` compile options, and
`UNITY_2021` / `CORDL_RUNTIME_FIELD_NULL_CHECKS` compile definitions -- none of
which were in this project's original CMakeLists.txt. `bs-cordl`'s qpm.json
entry is back to plain `"additionalData": {}`, matching the reference exactly.

**Remaining, non-blocking issue:** `qpm qmod manifest` (the current, non-deprecated
packaging command) fails with `missing field 'version'` when deserializing
`qpm.shared.json` -- happens regardless of `mod.template.json` content. The
older, deprecated `qpm qmod build` also fails, but with a different error:
`Value for key 'author' was requested, but wasn't set!` -- doesn't match any
difference found between this project's `mod.template.json` and the working
reference's. Neither has been root-caused; this only affects producing the
final packaged `.qmod` file, not the compiled mod code itself. Worth a focused
session against qpm CLI's actual source/issue tracker rather than more guessing.

## VRIK / full-body IK -- scope found, not yet started

Turns out to be much bigger than initially assumed: **not** a thin wrapper
around a Unity plugin, but a **full custom reimplementation of RootMotion's
FinalIK VRIK solver** in the original mod -- ~28 files covering spine, arm,
leg, footstep, and locomotion solvers, virtual bone math, quaternion/vector3
utility classes, all under `include(src)/customTypes/FinalIK/`. Built using
`chatplex-sdk-bs`'s IL2CPP inheritance macros (`CP_SDK_IL2CPP_INHERIT` etc.),
which this project deliberately doesn't depend on (see the UI section above).

This is a substantially bigger undertaking than any subsystem ported so far
and needs its own dedicated planning pass: likely either (a) porting the
math-heavy solver files close to verbatim (pure C++, framework-agnostic) while
reworking only the thin `custom-types`-registration shell around `VRIK`/
`TargetManager` themselves, or (b) something scoped smaller first. Not
started -- next session should begin by reading the full solver file set
before deciding an approach, the same way `avatarGenerator.cpp` was handled.

## How this is being built

The original author's source (`BSQ-VRM/VRM-Qavatars`) was provided as a
reference. Most subsystems are ported in stages, kept close to the original
with minimal changes — mainly updated include paths and dependency versions
for current beatsaber-hook/bs-cordl/custom-types/scotland2 APIs.

**The UI layer is an exception.** The entire original UI (avatar picker,
settings screen) is built on `chatplex-sdk-bs`, a separate third-party
framework with its own view-controller base class and widget library
(`CP_SDK::XUI`). Rather than take on learning and porting against an
unfamiliar SDK's full API from partial documentation, the UI is being
**redesigned in plain BSML** — same end-user functionality, different
(verified) implementation. See `src/UI/AvatarPicker.cpp`.

**What's ported so far** (`AssetLib/modelImporter.{hpp,cpp}` and its direct
dependencies): the assimp-based scene load, node/armature tree construction,
Unity mesh generation, VRM extension JSON parsing, VRM0 material/texture
generation (real mToon materials via the bundled shader AssetBundle), and
humanoid Avatar/Animator setup with VRM0/VRM1 bone mapping.

**Deliberately cut / not yet ported** (see the comment block at the end of
`ModelImporter::LoadVRM` in `src/AssetLib/modelImporter.cpp`): VRIK +
`TargetManager` setup (full-body IK), first/third-person mesh splitting, and
springbone generation (`springBoneGenerator`). Each is its own subsystem and
will be ported in follow-up passes.

**Practical result right now:** the avatar picker will find `.vrm` files
under `sdcard/ModData/com.beatgames.beatsaber/Mods/QavatarsRevived/Avatars`
and load the selected one. VRM0 avatars get real mesh, real materials, and a
working humanoid `Animator` — a plain Animator Controller could already
drive this avatar's arms/legs/head. VRM1 avatars load with no material
(upstream stub) but do get the humanoid mapping. No avatar has full-body IK
(VRIK) yet, so head/hand tracking won't move the body correctly until that
lands.

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
1.40.8 (note: some newer mods pin `bsml 0.5.1` — worth rechecking before a
real build). `assimp`, `sombrero`, `fmt`, `kaleb` version ranges are carried
over from the original VRM-Qavatars `qpm.json` (pre-1.40.8) and have **not**
been reverified — expect to bump these via `qpm restore`.

## Planned porting order

1. ~~Confirm dependency versions against current QPM registry for 1.40.8~~
2. ~~Avatar file loading (VRM parse → node/armature/mesh)~~
3. ~~Material/texture generation~~
4. ~~Avatar-picker UI~~ (redesigned in BSML, see above)
5. ~~Humanoid skeleton (Avatar) mapping + Animator~~ — this pass
6. VRIK + full-body tracking
7. Springbones + first/third-person mesh splitting
8. Settings UI (BSML redesign, same reasoning as the picker)
9. Replay-mode hook
10. Bug pass / device testing — first real build+run happens here

## Credits

Ported from [BSQ-VRM/VRM-Qavatars](https://github.com/BSQ-VRM/VRM-Qavatars)
by FutureMapper and contributors (MIT license). UI layer redesigned against
[bsq-ports/Quest-BSML](https://github.com/bsq-ports/Quest-BSML) (MIT
license, RedBrumbler and contributors).
