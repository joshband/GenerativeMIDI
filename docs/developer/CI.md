# Continuous Integration

Workflow: [`.github/workflows/ci.yml`](../../.github/workflows/ci.yml)

## Jobs

| Job | Runner | What it does |
|-----|--------|--------------|
| **Build iOS/iPadOS AUv3** | `macos-latest` | CMake iOS + `xcodebuild` AUv3; uploads `.app` |
| **Build macOS Plugins** | `macos-latest` | AU / AUv3 / VST3 / Standalone, `ctest`, **pluginval** on VST3 |
| **Build Windows VST3** | `windows-2022` | VST3 + Standalone (VS 2022), `ctest`, **pluginval** on VST3 |
| **Build Linux VST3** | `ubuntu-22.04` | VST3 + Standalone, `ctest`, **pluginval** on VST3 |

Pinned toolchain:

- **JUCE** `8.0.15` (clone in CI; keep in sync with local/docs)
- **pluginval** `v1.0.4` (release ZIP per OS)
- GitHub Actions pinned by SHA: `actions/checkout@v7.0.1`, `actions/upload-artifact@v7.0.1`

## pluginval scope

- **Validated:** built **VST3** (`Generative MIDI.vst3`) at strictness **5**, `--validate-in-process`, `--skip-gui-tests` (headless runners).
- **Not validated in CI (AU):**
  - AU needs host registration / `auval` and is Apple-only.
  - pluginval’s AU path is useful locally after install (`auval -v aumi Osrc Gmid`); CI sticks to VST3 so Linux/Windows stay comparable.
  - AUv3 (iOS) is build-only here — not store-ready and not pluginval-covered.

Logs upload as `pluginval-logs-{macOS,Windows,Linux}` (warn if empty so a failed download still surfaces the primary pluginval step).

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
