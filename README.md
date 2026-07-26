# Qavatars Revived

Scaffold for a from-scratch rebuild of custom VRM avatar support for Beat Saber
Quest, targeting **1.40.8**. This repo currently contains a **build skeleton
only** — no avatar loading, no replay support, no game logic. It builds and
produces an empty `.so` / `.qmod`, and nothing more.

## Status

- [x] QPM project scaffold (`qpm.json`, `mod.template.json`)
- [x] CMake build
- [x] GitHub Actions CI (build on push/PR, attach `.qmod` to tagged releases)
- [ ] VRM avatar parsing/loading
- [ ] Avatar rendering / bone mapping in the player's view
- [ ] In-game settings/config UI
- [ ] Replay-mode support
- [ ] Testing on-device

**Dependency versions:** the core toolchain deps (`beatsaber-hook ^6.4.2`,
`bs-cordl ^4008.*`, `custom-types ^0.18.3`, `scotland2 ^0.1.6`,
`paper2_scotland2 ^4.6.4`) are cross-checked against
[hardcpp/QBeatSaberPlus-GameTweaker](https://github.com/hardcpp/QBeatSaberPlus-GameTweaker),
an actively maintained mod whose v6.4.1 release explicitly targets game
version 1.40.8. These should be correct as of when this was written, but
still run `qpm restore` and expect drift over time.

VRM-specific libraries this mod will eventually need (VRM/glTF parsing,
skeleton math, UI) — likely `assimp`, `bsml`, `fmt`, `sombrero`,
`rapidjson-macros` based on the original VRM-Qavatars mod's dependency
list — are **not yet added**, since the versions seen for that mod predate
1.40.8 and haven't been re-verified. Add these with checked-current versions
once avatar loading is actually being implemented, not before.

## Build

Requires: Android NDK (r27, per the pinned `ndk` field), Ninja, QPM CLI, CMake ≥ 3.21.

```sh
qpm restore
qpm s configure   # or run the cmake command in qpm.json workspace.scripts.configure
qpm s build
qpm s qmod
```

## Planned porting order

1. Confirm dependency versions against current QPM registry for 1.40.8
2. Avatar file loading (VRM parse → internal representation)
3. Skeleton/bone mapping onto the player rig
4. Basic rendering in normal play
5. Settings UI (BSML)
6. Replay-mode hook — render avatar during replay playback
7. Bug pass / device testing

## Why this is a skeleton, not a finished mod

This project started as a request to bring back a previous mod (Qavatars)
with a full rewrite — VRM parsing, bone/finger mapping, and replay-mode
hooks are nontrivial subsystems that need iteration against real device
testing and (ideally) reference to prior working source. Generating that
logic in one shot without either would produce code that compiles but is
wrong in ways that are hard to spot until you're on-headset. Build it up
feature by feature from here.
