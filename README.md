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
- [x] **Basic VRM avatar loading (this pass):** node tree, gameobject
      hierarchy, armature detection, real Unity meshes (vertices, normals,
      UVs, bone weights, blendshape frames), and VRM extension JSON parsing
      (VRM0/VRM1 + blendshape master), ported from the original mod\'s
      `AssetLib` pipeline
- [ ] Material/texture generation (meshes currently render with no material)
- [ ] Humanoid Avatar / skeleton (Mecanim) bone mapping
- [ ] VRIK + full-body tracking
- [ ] Springbones (hair/cloth physics)
- [ ] In-game settings/config UI
- [ ] Replay-mode support
- [ ] Testing on-device -- **nothing in this repo has been built or run yet**

## How this is being built

The original author\'s source (`BSQ-VRM/VRM-Qavatars`) was provided as a
reference and is being ported in stages rather than reimplemented from
scratch, since it\'s tested, working code. Each stage is kept close to the
original with minimal changes -- mainly updated include paths and dependency
versions for current beatsaber-hook/bs-cordl/custom-types/scotland2 APIs.

**What\'s ported so far** (`AssetLib/modelImporter.{hpp,cpp}` and its direct
dependencies): the assimp-based scene load, node/armature tree construction,
and Unity mesh generation (`intermediateMeshGenerator` -> `meshGenerator`),
plus the glTF-binary-container parsing that extracts a VRM file\'s VRM0/VRM1
extension JSON and blendshape master data.

**Deliberately cut from this pass** (see the comment block at the end of
`ModelImporter::LoadVRM` in `src/AssetLib/modelImporter.cpp` for exactly
where): material/texture generation, humanoid `Avatar`/skeleton mapping
(`avatarGenerator`), VRIK + `TargetManager` setup, first/third-person mesh
splitting, and springbone generation (`springBoneGenerator`). These are
each their own subsystem in the original and will be ported in follow-up
passes rather than folded in all at once.

**Practical result right now:** calling `QavatarsRevived::LoadAvatar(path)`
will load a `.vrm` file\'s mesh and skeleton structure into Unity GameObjects,
but the result has no material (will render pink/default) and no working
humanoid animation rig yet. This is real progress toward "basic loading,"
not a finished feature.

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
(explicitly 1.40.8-targeted). `assimp`, `sombrero`, `bsml`, `fmt` version
ranges are carried over from the original VRM-Qavatars `qpm.json`
(pre-1.40.8) and have **not** been reverified -- expect to bump these via
`qpm restore`.

## Planned porting order

1. ~~Confirm dependency versions against current QPM registry for 1.40.8~~
2. ~~Avatar file loading (VRM parse -> node/armature/mesh)~~ -- this pass
3. Material/texture generation
4. Humanoid skeleton (Avatar) mapping + VRIK
5. Springbones + blendshape/expression wiring
6. Settings UI (BSML)
7. Replay-mode hook
8. Bug pass / device testing -- first real build+run happens here

## Credits

Ported from [BSQ-VRM/VRM-Qavatars](https://github.com/BSQ-VRM/VRM-Qavatars)
by FutureMapper and contributors (MIT license).
