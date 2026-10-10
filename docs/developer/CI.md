# Continuous Integration

Workflows:

- [`.github/workflows/ci.yml`](../../.github/workflows/ci.yml): builds, tests, pluginval, sanitizers
- [`.github/workflows/codeql.yml`](../../.github/workflows/codeql.yml): CodeQL static analysis
- [`.github/dependabot.yml`](../../.github/dependabot.yml): version updates

## Jobs (ci.yml)

| Job | Runner | Pull requests | Push to master / weekly / manual | What it does |
|-----|--------|:---:|:---:|--------------|
| **Detect code changes** | `ubuntu-latest` | yes | yes | Classifies the diff (`code`, `platform` outputs); docs-only PRs skip every job below |
| **Build macOS Plugins** | `macos-latest` | yes (VST3 + tests only) | yes (AU / AUv3 / VST3 / Standalone) | Ninja, `ctest`, **pluginval** on VST3, ccache |
| **Build Linux VST3** | `ubuntu-22.04` | yes | yes | Ninja, VST3 + Standalone, `ctest`, **pluginval** on VST3, ccache |
| **Sanitizers (ASan+UBSan)** | `ubuntu-22.04` | yes | yes | Debug build of the two Catch2 executables with `address,undefined`, then `ctest` |
| **Sanitizers (TSan)** | `ubuntu-22.04` | `full-ci` / build files | yes | Same with `thread`. **Advisory** (`continue-on-error`) |
| **Build Windows VST3** | `windows-2022` | `full-ci` / build files | yes | VST3 + Standalone (Ninja + MSVC), `ctest`, **pluginval** on VST3 |
| **Build iOS/iPadOS AUv3** | `macos-latest` | `full-ci` / build files | yes | CMake iOS (Xcode generator) + `xcodebuild` AUv3; uploads `.appex` / `.app` |

Measured runtimes (GitHub-hosted runners, warm ccache unless noted): macOS ~3-5 min, Linux VST3 ~3-7 min (first run with a cold cache: ~9 min), ASan+UBSan ~1.5 min warm (~10 min cold), TSan ~7 min cold (its cache is only saved when the job succeeds), CodeQL c-cpp ~15 min, CodeQL actions <1 min. ASan+UBSan stays on pull requests because the warm path is short.

### Triggers

- `pull_request` (types `opened`, `synchronize`, `reopened`, `labeled`) against **any** base branch, so stacked PRs also get CI. Adding any label re-runs the workflow (and cancels the superseded run).
- `push` to `main` / `master`, `workflow_dispatch`, and a weekly schedule (Monday 06:17 UTC).
- Runs on master are never cancelled; a new push to a PR cancels the superseded run.

### Full matrix on a PR (`full-ci`)

iOS, Windows and TSan are skipped on PRs by default (slow, rarely affected). They run on a PR when **either**:

- the PR has the **`full-ci`** label (GitHub UI, or `gh pr edit N --add-label full-ci`; adding it triggers a new run), **or**
- the diff touches `CMakeLists.txt` (any directory), `cmake/`, `*.cmake`, `.github/` or `.gitmodules` (the `platform` output of *Detect code changes*).

Off pull requests `platform` is always true. Docs-only PRs skip everything, label or not. Skips are job-level `if`s, so skipped jobs still satisfy required checks.

### PR vs master

| | Pull request | Push to master / weekly / manual |
|--|--|--|
| macOS build | `cmake --build --target GenerativeMIDI_VST3 GenerativeMIDITests GenerativeMIDIHostSmokeTests` | full build (AU, AUv3, VST3, Standalone) |
| ccache | restore only | restore, then save |
| pluginval zip cache | restore only | restore, save on miss |
| Plugin artifact uploads | no | yes |
| pluginval logs upload | yes | yes |

### Cache policy

GitHub cache scope: a run can read caches created on its own ref, on the PR base branch and on the default branch, but never caches created on other branches. A PR-created cache is therefore only readable by later runs of that same PR, while every commit saved a new per-sha key (Linux ~306 MB, ASan ~196 MB, macOS ~47 MB) and evicted the master-scope entries that PRs actually restore from (10 GB repository limit).

All ccache caches and the pluginval download use `actions/cache/restore` everywhere and `actions/cache/save` only when `github.event_name != 'pull_request'`. Keys are `ccache-<platform>-<sha>` with the restore-keys prefix `ccache-<platform>-`, so a PR starts from the newest master cache. The first master run after the switch to Ninja repopulates the caches.

CodeQL (codeql.yml) first runs `CodeQL scope`, which builds the analysis matrix. `CodeQL (actions)` runs on every PR; `CodeQL (c-cpp)` runs on push to master, weekly, manual dispatch, and on PRs with the `full-ci` label or build-file changes (it needs an uncached ~15 to 18 minute build and used to be cancelled by most pushes). `CodeQL (c-cpp)` is a manual build (`cmake` on Ubuntu, targets `GenerativeMIDI` and `GenerativeMIDITests`, no ccache so extraction sees every compile); `CodeQL (actions)` scans the workflow files. The weekly run is Wednesday 05:41 UTC. JUCE, `art`, `build` and `Tests` are excluded via `.github/codeql/codeql-config.yml`. Results appear under Security > Code scanning.

### Docs-only skip

Docs-only PRs (`docs/**`, `*.md`, `LICENSE`, `.github/dependabot.yml`) skip the build jobs. The skip is a job-level `if`, so skipped jobs report as skipped and still satisfy required status checks (a `paths-ignore` filter would leave required checks pending forever). Anything else under `.github/` counts as code (and as a build-file change that runs the full matrix). Exception: workflow changes in a Dependabot PR (version-pin bumps; the upload and cache-save steps do not run on PRs) do not trigger the full matrix on their own; add the `full-ci` label to force it.

### Compiler warnings in the logs

Each build step tees its output to `build.log`, and a `Summarize compiler warnings` step (`.github/scripts/summarize-warnings.sh`) turns that into a table on the job summary page plus inline annotations for the first 50 unique warnings. Warnings from JUCE, the `art` submodule and fetched dependencies are excluded, duplicates (the same header compiled into several targets) are collapsed, and the step never fails the job. The iOS build runs `xcodebuild -quiet`, which prints only warnings and errors instead of every compiler command line.

### Required checks

The branch ruleset "Protect master" currently requires **Build macOS Plugins**. Recommended additions (the owner approves and edits the ruleset; this repo's workflows do not change it):

- **Build Linux VST3**: fast and stable now that it runs on PRs.
- **Sanitizers (ASan+UBSan)**: passes cleanly today.

Do not require **Sanitizers (TSan)** until the Polyrhythm thread-safety fix (`fix/polyrhythm-thread-safety`) has landed and the job has been green; then delete `continue-on-error: true` from that job. Do not require the CodeQL checks until the first scans have been triaged.

Pinned toolchain:

- **JUCE** `8.0.15` (clone in CI; keep in sync with local/docs)
- **pluginval** `v1.0.4` (release ZIP per OS, each verified against a pinned sha256 in the `PLUGINVAL_SHA256_*` env vars; update them together with `PLUGINVAL_VERSION`)
- GitHub Actions pinned by full commit SHA (with a version comment); Dependabot keeps them current
- ccache on macOS (`~/Library/Caches/ccache`) and Linux (`~/.cache/ccache`) via `actions/cache/restore` + `actions/cache/save` (see Cache policy); the sanitizer jobs have their own caches (`ccache-linux-asan-`, `ccache-linux-tsan-`)
- Ninja generator on macOS, Linux and Windows (`ninja-build` via apt, `brew install ninja`; Windows uses `ilammy/msvc-dev-cmd`, pinned to a full SHA, to put MSVC on PATH). iOS stays on the Xcode generator.
- Artifacts (plugin builds and pluginval logs) are kept for 14 days (`retention-days: 14`)

## Timings

Measured from real runs (one sample each; runner noise is a few tens of seconds).

| Job / step | Before | After | Notes |
|---|---|---|---|
| macOS job, PR | 5m12s (build step 251s) | 1m20s to 1m49s (build step 53 to 83s) | VST3 + test targets only; build step was 83s with Makefiles and 53 to 63s with Ninja. Cache restore comes from master/base scope |
| macOS job, full build (dispatch) | 4m25s (build 214s) | 1m52s to 2m16s (build 76 to 87s) | warm ccache; Ninja on |
| Linux job, PR, warm cache | 2m27s to 3m03s | 2m21s (build 93s) | Ninja equals Makefiles when the cache is warm: no gain |
| Linux job, cold or wrong cache | 6m48s (dispatch) | 3m55s to 6m49s | see the cache-prefix note below |
| ASan+UBSan, PR | 1m11s to 1m22s | 1m07s | unchanged (noise) |
| Windows job (dispatch) | 10m51s (configure 114s, build 506s) | 7m07s to 9m26s (configure 79 to 103s, build 313 to 426s) | VS 2022 generator replaced by Ninja + MSVC; one variant of this table was measured on 4 runs, spread is large |
| iOS job (dispatch) | 4m32s | 4m33s to 5m06s | Xcode generator, unchanged |
| CodeQL (c-cpp) | 14m44s, on every PR push | skipped on PRs unless `full-ci` or build files change; 17m to 18m when it runs | building only `GenerativeMIDI` instead of both targets was slower (1107s, 1025s vs 884s), so both targets stay |
| CodeQL (actions) | 40s | 50s | runs on every PR |

Notes:

- The Linux release ccache used the restore-keys prefix `ccache-linux-`, which also matches `ccache-linux-asan-` and `ccache-linux-tsan-`; a second run restored a sanitizer cache and got a 3% hit rate. The key is now `ccache-linux-release-`.
- Switching generators changes nothing about ccache hashing in practice (second Ninja run with its own cache: Linux 93s build step), but the first run per cache scope after any key change is cold.
- Cache saves only happen on master, schedule and dispatch, so PRs see the numbers above only after a master run has populated the Ninja-era caches.


## Sanitizers

CMake option `GENMIDI_SANITIZE` (cache string, default empty = off). Accepted values: empty, `address,undefined`, `thread`. GCC and Clang only; anything else is a configure error. The flags (`-fsanitize=...`, `-fno-omit-frame-pointer`, and `-fno-sanitize-recover=undefined` so UBSan findings fail the test) are applied to the plugin target (PUBLIC, so the format wrappers and `GenerativeMIDIHostSmokeTests` inherit them) and to `GenerativeMIDITests`. Use a Debug (or RelWithDebInfo) build; the CI jobs use Debug.

Run locally (JUCE symlinked or cloned into `./JUCE`):

```bash
# ASan + UBSan
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DGENMIDI_SANITIZE=address,undefined
cmake --build build-asan --target GenerativeMIDITests GenerativeMIDIHostSmokeTests -j
(cd build-asan && ctest --output-on-failure)

# TSan (separate build tree: cannot be combined with ASan)
cmake -S . -B build-tsan -DCMAKE_BUILD_TYPE=Debug -DGENMIDI_SANITIZE=thread
cmake --build build-tsan --target GenerativeMIDITests GenerativeMIDIHostSmokeTests -j
(cd build-tsan && ctest --output-on-failure)
```

TSan notes: libtsan from GCC 11 reports false positives entirely inside JUCE's `Timer::TimerThread` / `WaitableEvent` ("double lock of a mutex" and lock-order-inversion). The TSan job sets `TSAN_OPTIONS=report_mutex_bugs=0:exitcode=0` and then fails only if the log contains a `WARNING: ThreadSanitizer` line that is not a lock-order-inversion, so data races still fail the step. The job is also `continue-on-error: true` (advisory) until the Polyrhythm thread-safety fix lands; remove that line then and consider requiring it. Locally on macOS Apple Clang does not produce the false positives.

Works with Apple Clang on macOS. On Linux, CI also sets `ASAN_OPTIONS=detect_leaks=1:...`; LeakSanitizer is not supported on macOS. On Ubuntu runners TSan needs `sudo sysctl vm.mmap_rnd_bits=28` (done in the job).

In CI, ASan+UBSan runs on every PR; TSan runs with the `full-ci` label, on build-file changes, on master, weekly and on manual dispatch. To run the whole workflow by hand on a branch: `gh workflow run ci.yml --ref <branch>`.

Most unit tests are single-threaded, so TSan only reports races exercised by the tests; it is not a substitute for a threaded test of the audio/UI split.

## Supply-chain and security settings

- Dependabot version updates: `github-actions` and `gitsubmodule` (the `art` submodule). JUCE (cloned at `JUCE_VERSION`) and Catch2 (CMake `FetchContent`) are not supported by Dependabot (there is no CMake ecosystem); bump them by hand.
- Settings only the repository owner can change in the GitHub UI (Settings > Advanced Security / Code security) are listed in the PR that introduced this layout: Dependabot alerts and security updates, and (optionally) CodeQL "default setup" must stay off because the advanced workflow `codeql.yml` is used instead.

## pluginval scope

- **Validated:** built **VST3** (`Generative MIDI.vst3`) at strictness **5**, `--validate-in-process`, `--skip-gui-tests` (headless runners).
- **Not validated in CI (AU):**
  - AU needs host registration / `auval` and is Apple-only.
  - pluginval’s AU path is useful locally after install (`auval -v aumi Gmid Osrc`); CI sticks to VST3 so Linux/Windows stay comparable.
  - AUv3 (iOS) is build-only here — not store-ready and not pluginval-covered.

Logs upload as `pluginval-logs-{macOS,Windows,Linux}` (14-day retention) (warn if empty so a failed download still surfaces the primary pluginval step).

## Platform format matrix (CMake)

`CMakeLists.txt` selects formats by platform:

- **iOS:** `AUv3`
- **macOS:** `AU`, `AUv3`, `VST3`, `Standalone`
- **Windows / Linux:** `VST3`, `Standalone`

## Known gaps / watch-outs

1. **MIDI effect + pluginval:** Strictness >5 or GUI tests may flake on MIDI-only buses; raise carefully.
2. **Linux WebKit / ALSA:** Browser module is disabled (`JUCE_WEB_BROWSER=0`); audio deps are apt-installed for JUCE. JACK is optional runtime. pkg-config may still warn about missing `libcurl` / WebKit / GTK — harmless with those flags off.
3. **Windows runner + MSVC:** Job pins `windows-2022` (`windows-latest` may ship Visual Studio 2026 only). The Ninja generator needs the MSVC environment, provided by `ilammy/msvc-dev-cmd`; configure/build use PowerShell. JUCE is cloned into `./JUCE` (no symlink). `CMAKE_MSVC_DEBUG_INFORMATION_FORMAT=Embedded` only matters once Windows gets ccache and `cmake_minimum_required` is raised to 3.25 (policy CMP0141); Release has no `/Zi` today.
4. **Windows artifact paths:** `build/GenerativeMIDI_artefacts/Release/…` (same layout with Ninja).
5. **iOS artifacts:** AUv3 builds produce `.appex` (and sometimes a host `.app`). CI uploads both globs; requiring only `*.app` fails after a successful `xcodebuild`.
6. **iOS signing:** Unsigned CI build (`CODE_SIGNING_REQUIRED=NO`) — device/TestFlight needs local signing.
7. **COPY_PLUGIN_AFTER_BUILD:** May attempt user plugin folders on the runner; CI still consumes in-tree artefacts for upload/pluginval.

## Local pluginval (macOS)

```bash
curl -fsSL "https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_macOS.zip" -o pluginval.zip
unzip -q pluginval.zip
./pluginval.app/Contents/MacOS/pluginval \
  --validate-in-process --strictness-level 5 --skip-gui-tests \
  "build/GenerativeMIDI_artefacts/Release/VST3/Generative MIDI.vst3"
```
