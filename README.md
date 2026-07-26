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

**Important:** dependency versions in `qpm.json` (`beatsaber-hook`,
`bs-cordl`, `custom-types`, `paper2_scotland2`, `scotland2`, `song-control`)
are best-guess placeholders and have **not** been verified against the live
QPM registry. Run `qpm restore` and expect to bump versions before this
builds cleanly.

## Build

Requires: Android NDK (r26d used in CI), Ninja, QPM CLI, CMake ≥ 3.21.

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
