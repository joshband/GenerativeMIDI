# Continuous Integration

Workflows:

- [`.github/workflows/ci.yml`](../../.github/workflows/ci.yml): builds, tests, pluginval, sanitizers
- [`.github/workflows/codeql.yml`](../../.github/workflows/codeql.yml): CodeQL static analysis
- [`.github/dependabot.yml`](../../.github/dependabot.yml): version updates

## Jobs (ci.yml)

| Job | Runner | Pull requests | Push to master / weekly / manual | What it does |
|-----|--------|:---:|:---:|--------------|
| **Detect code changes** | `ubuntu-latest` | yes | yes | Classifies the diff; docs-only PRs skip every job below |
| **Build macOS Plugins** | `macos-latest` | yes | yes | AU / AUv3 / VST3 / Standalone, `ctest`, **pluginval** on VST3, ccache |
| **Build Linux VST3** | `ubuntu-22.04` | yes | yes | VST3 + Standalone, `ctest`, **pluginval** on VST3, ccache |
| **Sanitizers (ASan+UBSan)** | `ubuntu-22.04` | yes | yes | Debug build of the two Catch2 executables with `address,undefined`, then `ctest` |
| **Sanitizers (TSan)** | `ubuntu-22.04` | no | yes | Same with `thread`. **Advisory** (`continue-on-error`) |
| **Build Windows VST3** | `windows-2022` | no | yes | VST3 + Standalone (VS 2022), `ctest`, **pluginval** on VST3 |
| **Build iOS/iPadOS AUv3** | `macos-latest` | no | yes | CMake iOS + `xcodebuild` AUv3; uploads `.appex` / `.app` |

Measured runtimes (GitHub-hosted runners, warm ccache unless noted): macOS ~3-5 min, Linux VST3 ~3-7 min (first run with a cold cache: ~9 min), ASan+UBSan ~1.5 min warm (~10 min cold), TSan ~7 min cold (its cache is only saved when the job succeeds), CodeQL c-cpp ~15 min, CodeQL actions <1 min. ASan+UBSan stays on pull requests because the warm path is short.

The weekly schedule is Monday 06:17 UTC. Runs on master are never cancelled; a new push to a PR cancels the superseded run.

CodeQL (codeql.yml) runs `CodeQL (c-cpp)` (manual build: `cmake` on Ubuntu, targets `GenerativeMIDI` and `GenerativeMIDITests`, no ccache so extraction sees every compile) and `CodeQL (actions)` (scans the workflow files) on pull requests, pushes to master, weekly (Wednesday 05:41 UTC) and manual dispatch. JUCE, `art` and `build` are excluded via `.github/codeql/codeql-config.yml`. Results appear under Security > Code scanning.

### Docs-only skip

Docs-only PRs (`docs/**`, `*.md`, `LICENSE`) skip the build jobs. The skip is a job-level `if`, so skipped jobs report as skipped and still satisfy required status checks (a `paths-ignore` filter would leave required checks pending forever). Anything under `.github/` counts as code.

### Required checks

The branch ruleset "Protect master" currently requires **Build macOS Plugins**. Recommended additions (the owner approves and edits the ruleset; this repo's workflows do not change it):

- **Build Linux VST3**: fast and stable now that it runs on PRs.
- **Sanitizers (ASan+UBSan)**: passes cleanly today.

Do not require **Sanitizers (TSan)** until the Polyrhythm thread-safety fix (`fix/polyrhythm-thread-safety`) has landed and the job has been green; then delete `continue-on-error: true` from that job. Do not require the CodeQL checks until the first scans have been triaged.

Pinned toolchain:

- **JUCE** `8.0.15` (clone in CI; keep in sync with local/docs)
- **pluginval** `v1.0.4` (release ZIP per OS, each verified against a pinned sha256 in the `PLUGINVAL_SHA256_*` env vars; update them together with `PLUGINVAL_VERSION`)
- GitHub Actions pinned by full commit SHA (with a version comment); Dependabot keeps them current
- ccache on macOS (`~/Library/Caches/ccache`) and Linux (`~/.cache/ccache`) via `actions/cache`; the sanitizer jobs have their own caches (`ccache-linux-asan-`, `ccache-linux-tsan-`)
- Artifacts (plugin builds and pluginval logs) are kept for 14 days (`retention-days: 14`)

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
3. **Windows runner + VS discovery:** Job pins `windows-2022` because `windows-latest` may ship Visual Studio 2026 only (CMake `-G "Visual Studio 17 2022"` then fails). Configure/build use PowerShell — Git Bash often cannot find VS either. JUCE is cloned into `./JUCE` (no symlink).
4. **Windows artifact paths:** Multi-config VS layout under `build/GenerativeMIDI_artefacts/Release/…`.
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
