# AGENTS.md

Canonical instructions for coding agents working in this repository. Humans: see [CONTRIBUTING.md](CONTRIBUTING.md).

## Project

GenerativeMIDI is a JUCE/C++ generative MIDI plugin (AU, VST3, Standalone on macOS; VST3 + Standalone on Windows/Linux; AUv3 for iOS builds). Layout: `Source/PluginProcessor.*` (parameters, `processBlock`), `Source/Core/` (engines), `Source/DSP/` (clock, event scheduler), `Source/Modulation/`, `Source/UI/`, `Source/PluginEditor.*`, `Tests/`. Details and threading rules: [docs/developer/ARCHITECTURE.md](docs/developer/ARCHITECTURE.md).

## Build and test

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCOPY_PLUGIN_AFTER_BUILD=OFF \
  && cmake --build build -j8 \
  && (cd build && ctest --output-on-failure -C Release)
```

- JUCE must be at `./JUCE`. In a git worktree, symlink your JUCE checkout to `./JUCE` first (a worktree does not contain it).
- `CMakeLists.txt` hard-codes `COPY_PLUGIN_AFTER_BUILD TRUE` (`CMakeLists.txt:57`); a build may copy plugins into the user's plugin folders. See [CONTRIBUTING.md](CONTRIBUTING.md).
- Two test executables: `GenerativeMIDITests` (engines, fixed source list) and `GenerativeMIDIHostSmokeTests` (full processor with a fake playhead).
- More: [docs/developer/BUILD.md](docs/developer/BUILD.md), [docs/developer/BUILDING-iOS.md](docs/developer/BUILDING-iOS.md).

## Rules

- **Honesty.** Never invent or inflate tests, counts, features or host-compatibility claims. Recount numbers from the code before writing them in docs. Cite `path:line` for non-obvious claims. If you could not verify something, say so.
- **Never push to `master`.** Work on a branch and open a PR.
- **One PR per theme**, with several commits if needed.
- **Draft PRs run no CI.** Iterate as a draft; mark ready for review to start checks. Add the `full-ci` label before merge when iOS, Windows, TSan or CodeQL should run (see [docs/developer/CI.md](docs/developer/CI.md)).
- **Red/green for bug fixes.** The new test must fail without the fix. Rebuild after every change and `grep 'error:'` the build output so a failed compile cannot leave a stale test binary that passes or fails for the wrong reason.
- **REQUIRE evaluation order.** Never put a mutating call inside an `==` operand of `REQUIRE`/`CHECK`; evaluation order differs between compilers. Call first, store, then compare.
- **Verify pushes** with `git ls-remote --heads origin <branch>`; do not assume a push succeeded.
- **Conflicts:** resolve with `git merge origin/master`, then check that no conflict markers (`<<<<<<<`, `=======`, `>>>>>>>`) remain before committing.
- **Realtime safety.** Nothing reachable from `processBlock` may allocate, lock, block, do I/O, or build strings. Cross-thread state uses atomics, the `MidiActivityLog` FIFO, or the `PolyrhythmEngine` snapshot scheme. Known debt: per-block string parameter lookups. See [ARCHITECTURE.md](docs/developer/ARCHITECTURE.md#realtime-rules).
- **Screenshot privacy.** Capture the app window only, never the full desktop.
- **Confirm with the owner** before destructive or remote actions: force-push, deleting branches or releases, changing repo settings or rulesets, publishing, merging.

## Serial files

Only one agent may edit each of these at a time. If another agent or PR is touching one, coordinate through the owner instead of editing in parallel:

- `Source/PluginProcessor.cpp`, `Source/PluginProcessor.h`
- `Source/PluginEditor.cpp`
- `Source/DSP/ClockManager.*`
- `Tests/HostSmokeTests.cpp`
- `CMakeLists.txt`
- `.github/workflows/ci.yml`

## Pointers

- Architecture, threading and transport: [docs/developer/ARCHITECTURE.md](docs/developer/ARCHITECTURE.md)
- Contributing, style, PR policy: [CONTRIBUTING.md](CONTRIBUTING.md)
- CI jobs, labels, sanitizers: [docs/developer/CI.md](docs/developer/CI.md)
- Security reports: [SECURITY.md](SECURITY.md)
