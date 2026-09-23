# GenerativeMIDI - Development Status

**Last Updated**: 2026-09-22  
**Current Version**: v0.8.0  
**Repository**: https://github.com/joshband/GenerativeMIDI  
**Build**: [![CI](https://github.com/joshband/GenerativeMIDI/actions/workflows/ci.yml/badge.svg)](https://github.com/joshband/GenerativeMIDI/actions/workflows/ci.yml)

## Honest product snapshot

| Claim | Reality |
|-------|---------|
| Generators in editor UI | **10** (Euclidean + Polyrhythm + 4 algorithmic + 4 stochastic) |
| Polyrhythm | **Experimental shipped** — selectable (APVTS index 1), DSP wired, minimal layer editor |
| Velocity / density LFO | ADVANCED controls (`modLfoEnable` / rate / depth / density depth); matrix UI still archived |
| `stochasticType` APVTS param | **Legacy** — kept for session load; non-automatable; **unused by DSP** |
| Modulation matrix | Archived under `archive/modulation_v1/` — not live |
| AUv3 / iOS | CMake iOS target (`GenerativeMIDI_AUv3`) + docs; **not App Store–ready**; macOS desktop build has `JucePlugin_Build_AUv3=0` |
| Touch / a11y | iOS larger hit targets + scrollable editor; key controls have AX `setTitle` names |
| Tests | Catch2 + `ctest` (**26**, incl. host playhead smoke + polyrhythm layer persistence + Markov trained lookup) in CMake / CI |
| CI matrix | macOS plugins + iOS AUv3 + Windows/Linux VST3 + pluginval (VST3) |
| Canonical build | **CMake** (`GenerativeMIDI.jucer` deprecated) |

Showcase: https://joshband.github.io/GenerativeMIDI/

## Completed features (summary)

### v0.8.0 — Expression, LFO MVP, Polyrhythm experimental
- MIDI channel routing (1–16)
- MIDI expression UI (AT / PB / CC) with note-on emit
- Velocity LFO (+ density destination)
- Experimental Polyrhythm generator + layer editor
- Stochastic UI knobs; Euclidean playhead; factory presets

### v0.7.x — Stochastic / chaos + dynamic UI
### v0.6.x — Presets (XML `.gmpreset`)
### v0.5.x — Ratcheting + layout
### v0.4.0 — Gate length / legato
### v0.3.0 — Gilded steampunk UI
### v0.2.0 — Scale & humanization
### v0.1.0 — Core engines; AU / VST3 / Standalone

## In progress

None for the post-merge polish wave — see Planned for remaining MVP gaps.

### UI overhaul (2026-09-22)
Deferred polish items shipped on `master`: branded combo popups, full Preset Manager restyle, round-cyan slider thumbs, scrollable cross-format editor sizes. Screenshots: [`docs/qa/logs/ui_overhaul_overview.png`](docs/qa/logs/ui_overhaul_overview.png), [`docs/qa/logs/ui_overhaul_combo_popup.png`](docs/qa/logs/ui_overhaul_combo_popup.png), [`docs/qa/logs/ui_overhaul_presets.png`](docs/qa/logs/ui_overhaul_presets.png), [`docs/qa/logs/ui_overhaul_compact.png`](docs/qa/logs/ui_overhaul_compact.png). **Format note:** macOS Debug builds AU + VST3 + Standalone; AUv3 is the **iOS** CMake target (deployment **15.0+**). Device AUv3 QA still deferred.

## Planned (explicitly deferred)

### Polyrhythm polish
- Pattern step editing / polymeter visualization
- Continuous polish beyond pitch/vel transforms, division rate scaling, and layer XML persistence

### MIDI expression depth
Continuous CC/PB modulation and MPE (note-on emit shipped).

### Modulation matrix
Full router / multi-source panel — not part of current LFO MVP.

### Remaining UI
MIDI log pane; AUv3 on-device touch QA; App Store packaging.

## Project metrics

| Metric | Value |
|--------|-------|
| Product version | v0.8.0 |
| UI generators | 10 (Polyrhythm experimental) |
| Build | `.github/workflows/ci.yml` (4 jobs) |
| Tests | `ctest` (Catch2; host playhead smoke + polyrhythm layer persistence) |
| Docs | README, FEATURES, GETTING_STARTED, BUILD, Pages, [SMOKE_CHECKLIST](docs/user/SMOKE_CHECKLIST.md) |

## Notes

- Prefer CMake; initialize `art/` with `git submodule update --init --recursive`.
- **Realtime**: Euclidean regen, ratchet, algorithmic single-note, EventScheduler steady-state path allocation-free (queue pre-reserved). Trained Markov lookup reuses a pre-reserved scratch key (no per-note heap); map remains `vector`-keyed (learn off-RT). Polyrhythm uses shared `scheduleNote` (expression intact).
- **Host/UI smoke (2026-09-22):** Phase A green (ctest/auval/pluginval). Phase B: Debug Standalone rebuilt; generator menu shows **10 items including Polyrhythm** (earlier miss = stale Debug binary under `CMAKE_BUILD_TYPE=Release`). Phase C: TwelveTake REAPER MCP — insert Generative MIDI VST3 + ReaSynth, play/stop **PASS**. Optional ear-check re-run: play→stop **PASS with residual human glance** (no MIDI monitor). **Headless host smoke:** `GenerativeMIDIHostSmokeTests` (fake `AudioPlayHead`, note-ons while playing + stop gate). **Polyrhythm layers** persist in session (`get`/`setStateInformation`) and preset XML (`PolyrhythmLayers` ValueTree child, schema **1.2**). **AX titles** on Generator Type / Presets / LFO / Probability / layer editor controls (System Events probe). **Polyrhythm layer UI** confirmed in Standalone screenshots. Details: [`docs/qa/QA_FINDINGS_v0.8.0.md`](docs/qa/QA_FINDINGS_v0.8.0.md); MCP setup: [`docs/qa/reaper/MCP_SETUP.md`](docs/qa/reaper/MCP_SETUP.md).

**License**: MIT  
**Repository**: https://github.com/joshband/GenerativeMIDI
