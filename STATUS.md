# GenerativeMIDI - Development Status

**Last Updated**: 2026-10-09  
**Current Version**: v1.0.0 (`CMakeLists.txt` project version and the GitHub release; the feature history below keeps its earlier milestone labels)  
**Repository**: https://github.com/joshband/GenerativeMIDI  
**Build**: [![CI](https://github.com/joshband/GenerativeMIDI/actions/workflows/ci.yml/badge.svg)](https://github.com/joshband/GenerativeMIDI/actions/workflows/ci.yml)

## Honest product snapshot

| Claim | Reality |
|-------|---------|
| Generators in editor UI | **10** (Euclidean + Polyrhythm + 4 algorithmic + 4 stochastic) |
| Polyrhythm | **Experimental shipped** — selectable (APVTS index 1), DSP wired, step-row editor (polymeter lengths) |
| Velocity / density LFO | Modulation bar. Routes 1–2 are LFO → velocity and LFO → density. Routes 3–4 add sample-and-hold to gate, pitch, CC, or bend |
| `stochasticType` APVTS param | **Legacy** — kept for session load; non-automatable; **unused by DSP** |
| Modulation matrix | Fixed four-slot bar is live. Envelope and tempo-sync remain deferred |
| Standalone piano | Piano switch plays generated notes in the app and starts off. AU/VST3 remain MIDI effects with no audio output |
| Voice and parts | Poly or Mono on the melody channel. Parts 1–4 add root, chord, and arp on the next MIDI channels |
| AUv3 / iOS | CMake iOS target (`GenerativeMIDI_AUv3`) + docs; **not App Store–ready**; macOS desktop build has `JucePlugin_Build_AUv3=0` |
| Touch / a11y | iOS larger hit targets + scrollable editor; key controls have AX `setTitle` names; MIDI Log toggle/Clear titled |
| Tests | Catch2 + `ctest` (**78** cases, incl. host playhead smoke + polyrhythm layer persistence + Markov trained lookup + Markov/L-System/Cellular controls + MIDI activity FIFO) in CMake / CI |
| CI matrix | Pull requests run macOS plugins only (docs-only PRs skip builds). iOS AUv3 + Windows/Linux VST3 run on push to master, weekly, and manual dispatch. pluginval (VST3) on macOS, Windows, Linux |
| Canonical build | **CMake** (`GenerativeMIDI.jucer` deprecated) |

Showcase: https://joshband.github.io/GenerativeMIDI/

## Completed features (summary)

### v0.8.0 — Expression, LFO MVP, Polyrhythm experimental
- MIDI channel routing (1–16)
- MIDI expression UI (AT / PB / CC) with note-on emit, plus held-note CC and pitch-bend from the velocity LFO
- Velocity LFO (+ density destination)
- Experimental Polyrhythm generator with per-layer step rows (click to toggle, polymeter lengths)
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

### Look (2026-09-23)
Cavity-first editor: header identity, pattern hero, performance knobs, musical row, collapsed Shape disclosure, MIDI log pinned under the viewport. Spec: [`docs/design/MACHINED_UI.md`](docs/design/MACHINED_UI.md). Proof: [`docs/qa/logs/ui_overhaul_default.png`](docs/qa/logs/ui_overhaul_default.png), [`docs/qa/logs/ui_overhaul_euclidean.png`](docs/qa/logs/ui_overhaul_euclidean.png), [`docs/qa/logs/ui_overhaul_narrow_shape.png`](docs/qa/logs/ui_overhaul_narrow_shape.png).

### UI review (2026-09-23)
Post-overhaul critique and width/pattern/disabled-knob fixes: [`docs/design/UI_UX_REVIEW_2026-09.md`](docs/design/UI_UX_REVIEW_2026-09.md). Desktop minimum editor size is **720×520**; content width tracks the window.

### UI overhaul (2026-09-22)
Deferred polish items shipped on `master`: branded combo popups, full Preset Manager restyle, round-cyan slider thumbs, scrollable cross-format editor sizes, **MIDI activity log pane**. Screenshots: [`docs/qa/logs/ui_overhaul_overview.png`](docs/qa/logs/ui_overhaul_overview.png), [`docs/qa/logs/ui_overhaul_combo_popup.png`](docs/qa/logs/ui_overhaul_combo_popup.png), [`docs/qa/logs/ui_overhaul_presets.png`](docs/qa/logs/ui_overhaul_presets.png), [`docs/qa/logs/ui_overhaul_compact.png`](docs/qa/logs/ui_overhaul_compact.png), MIDI log: [`docs/qa/logs/auv3_sim_midi_log.jpg`](docs/qa/logs/auv3_sim_midi_log.jpg). **Format note:** macOS Debug builds AU + VST3 + Standalone; AUv3 is the **iOS** CMake target (deployment **15.0+**); iOS also builds Standalone for Simulator editor QA.

### AUv3 Simulator QA (2026-09-22)
**Verified on iPad Pro 13" Simulator (iOS 27):** build (AUv3 + Standalone host), install/launch, editor layout, status chip free-run/note, Euclidean pattern, MIDI Log live ON/OFF events. Evidence: [`docs/qa/logs/auv3_sim_qa.txt`](docs/qa/logs/auv3_sim_qa.txt), [`auv3_sim_overview.jpg`](docs/qa/logs/auv3_sim_overview.jpg), [`auv3_sim_running.jpg`](docs/qa/logs/auv3_sim_running.jpg), [`auv3_sim_midi_log.jpg`](docs/qa/logs/auv3_sim_midi_log.jpg). **Not verified:** physical device (Offline); interactive drag/tap gestures (no Simulator.app GUI on this host). **Not App Store–ready.**

## Planned (explicitly deferred)

### MIDI expression depth
MPE (per-note channels and zone config).

### Modulation matrix
Envelope follower and tempo-synced LFO. The fixed-slot router (LFO, sample-and-hold, four routes) is in the editor.

### Remaining UI / shipping
Physical-device AUv3 touch QA; App Store packaging.

## Project metrics

| Metric | Value |
|--------|-------|
| Product version | v1.0.0 |
| UI generators | 10 (Polyrhythm experimental) |
| Parameters | 57 registered (56 automatable; legacy `stochasticType` is non-automatable) |
| Factory presets | 11 (including Polyrhythm Layers) |
| Build | `.github/workflows/ci.yml` (4 jobs) |
| Tests | `ctest` (Catch2; **78** cases — 51 in `EngineTests.cpp`, 27 in `HostSmokeTests.cpp`) |
| Docs | README, FEATURES, GETTING_STARTED, BUILD, Pages, [SMOKE_CHECKLIST](docs/user/SMOKE_CHECKLIST.md) |

## Notes

- Prefer CMake; initialize `art/` with `git submodule update --init --recursive`.
- **Realtime**: Euclidean regen, ratchet, algorithmic single-note, EventScheduler steady-state path allocation-free (queue pre-reserved). Trained Markov lookup reuses a pre-reserved scratch key (no per-note heap); map remains `vector`-keyed (learn off-RT). Polyrhythm uses shared `scheduleNote` (expression intact).
- **Host/UI smoke (2026-09-22):** Phase A green (ctest/auval/pluginval). Phase B: Debug Standalone rebuilt; generator menu shows **10 items including Polyrhythm** (earlier miss = stale Debug binary under `CMAKE_BUILD_TYPE=Release`). Phase C: TwelveTake REAPER MCP — insert Generative MIDI VST3 + ReaSynth, play/stop **PASS**. Optional ear-check re-run: play→stop **PASS with residual human glance** (no MIDI monitor). **Headless host smoke:** `GenerativeMIDIHostSmokeTests` (fake `AudioPlayHead`, note-ons while playing + stop gate). **Polyrhythm layers** persist in session (`get`/`setStateInformation`) and preset XML (`PolyrhythmLayers` ValueTree child, schema **1.2**). **AX titles** on Generator Type / Presets / LFO / Probability / layer editor controls (System Events probe). **Polyrhythm layer UI** confirmed in Standalone screenshots. Details: [`docs/qa/QA_FINDINGS_v0.8.0.md`](docs/qa/QA_FINDINGS_v0.8.0.md); MCP setup: [`docs/qa/reaper/MCP_SETUP.md`](docs/qa/reaper/MCP_SETUP.md).

**License**: MIT  
**Repository**: https://github.com/joshband/GenerativeMIDI
