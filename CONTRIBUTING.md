# Contributing

Thanks for helping. This is a spare-time open-source project; small, focused changes with tests are the easiest to review.

## Build and test

Canonical build system is CMake. Full instructions: [docs/developer/BUILD.md](docs/developer/BUILD.md); iOS/iPadOS: [docs/developer/BUILDING-iOS.md](docs/developer/BUILDING-iOS.md).

Prerequisites: clone with submodules (`git submodule update --init --recursive`) and put or symlink a JUCE checkout at `./JUCE` (CI uses JUCE 8.0.15, see [docs/developer/CI.md](docs/developer/CI.md)).

Quick command:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCOPY_PLUGIN_AFTER_BUILD=OFF \
  && cmake --build build -j8 \
  && (cd build && ctest --output-on-failure -C Release)
```

About `COPY_PLUGIN_AFTER_BUILD`: a CMake option that defaults to ON, which makes JUCE copy each built plugin into the user plugin folders (for VST3 on macOS, `~/Library/Audio/Plug-Ins/VST3`, per [BUILD.md](docs/developer/BUILD.md)). The command above passes `OFF`, so a development build leaves your installed plugins alone.

## Where tests go

Two Catch2 executables, both registered with CTest (`CMakeLists.txt:150`, `:180`, `:217-218`):

- `Tests/EngineTests.cpp` (`GenerativeMIDITests`): engine and DSP logic that does not need the plugin. The executable compiles a fixed source list; if your test needs a new `.cpp`, add it to that list in `CMakeLists.txt`.
- `Tests/HostSmokeTests.cpp` (`GenerativeMIDIHostSmokeTests`): processor-level behaviour through `GenerativeMIDIProcessor::processBlock` with the `FakePlayHead`, including cross-thread tests.

Tests added for a change belong in the same PR as the change. See [docs/developer/ARCHITECTURE.md](docs/developer/ARCHITECTURE.md#testing).

## Bug fixes: red then green

A bug-fix test must fail without the fix. Write the test, rebuild, see it fail, apply the fix, rebuild, see it pass. After rebuilding, check the build output (`grep 'error:'`) so a failed compile does not leave you running a stale test binary.

## Catch2 evaluation order

Never put a call that mutates state inside an `==` operand of `REQUIRE`/`CHECK`. Operand evaluation order differs between compilers, so a test can pass on one platform and fail on another. Call the function first, store the result, then compare.

```cpp
const int got = engine.nextStep();   // mutating call outside the assertion
REQUIRE(got == 3);
```

## Pull requests and CI

Policy as implemented in [.github/workflows/ci.yml](.github/workflows/ci.yml) and described in [docs/developer/CI.md](docs/developer/CI.md):

- Open the PR as a **draft** while you iterate. Draft PRs run no CI; marking it ready for review starts the checks.
- A non-draft PR runs the macOS build (VST3 plus tests), the Linux VST3 build, and ASan+UBSan.
- The `full-ci` label additionally runs iOS, Windows, TSan and CodeQL (c-cpp). These also run when the diff touches `CMakeLists.txt`, `cmake/`, `*.cmake`, `.github/` or `.gitmodules`. Add the label before merging if your change could affect them.
- A push to `master` runs the full matrix.
- Docs-only PRs (`docs/**`, `*.md`, `LICENSE`, some `.github` metadata) skip the build jobs.
- Prefer one PR per theme, with several commits, over many one-commit PRs.

Fill in the [pull request template](.github/pull_request_template.md): what and why, how it was checked, and CI cost.

## Code style

Match the surrounding code; there is no formatter config in the repo, and the files are not perfectly uniform. Observed in `Source/`:

- 4-space indentation, no tabs, braces on their own lines for classes and functions.
- Types and classes `PascalCase` (`EventScheduler`, `PolyrhythmEngine`), functions and variables `camelCase`, member state unprefixed, parameter IDs as `PARAM_*` constants with `camelCase` string values (`Source/PluginProcessor.h:137`).
- JUCE idioms: `juce::` always qualified, `jlimit`/`jmax`/`jmin`, `JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR`, `juce::ScopedNoDenormals` in `processBlock`.
- Audio-thread code follows the realtime rules in [ARCHITECTURE.md](docs/developer/ARCHITECTURE.md#realtime-rules): no allocation, locks or string lookups on the audio path.
- File headers are a boxed comment with the file name and a one-line purpose.

## Screenshots

When a PR includes screenshots, capture the **app window only**. Never attach a full-desktop screenshot: it can expose other windows, notifications and file names.

## Security

Do not post vulnerability details in a public issue. Follow [SECURITY.md](SECURITY.md).
