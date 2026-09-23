# GenerativeMIDI QA Findings — v0.8.0

**Date:** 2026-09-22  
**Commit:** `7b038d3` (AX titles + Polyrhythm UI confirm; builds on `b146357`)  
**Campaign:** Host QA Exercise Plan + REAPER MCP Phase C + AX titles

## 1. Environment

| Item | Value |
|------|--------|
| OS | macOS (darwin 25.x) |
| Formats tested | AU (`aumi Gmid Osrc`), VST3 Debug artefact, **Debug Standalone (rebuilt)** |
| Hosts present | REAPER.app (launched), Logic Pro.app (not exercised) |
| Automated | `ctest` **26/26** (engine + host smoke + polyrhythm persistence + Markov trained lookup), `auval` PASS, pluginval v1.0.4 strictness 5 SUCCESS |
| Permissions (re-verified) | **Accessibility OK** (System Events); **Screen Recording OK** (`screencapture`) — [`logs/permissions_check.txt`](logs/permissions_check.txt) |
| Reaper / DAW MCP | **TwelveTake** `user-reaper` (`uvx twelvetake-reaper-mcp`) — bridge via `__startup.lua` |

Logs: [`docs/qa/logs/`](logs/).

### Rebuild note (stale Debug binary)

Earlier Phase B used a **stale Debug Standalone** (`mtime` Sep 22 **14:40**) while `CMAKE_BUILD_TYPE` was **Release**, so Release Standalone relinked (18:22) but Debug did not. Force reconfigure `-DCMAKE_BUILD_TYPE=Debug` + `cmake --build build --target GenerativeMIDI_Standalone` produced a fresh binary.

AX-title rebuild (same day): Debug Standalone relinked after `PluginEditor` / `PolyrhythmLayerEditor` / `PresetBrowser` `setTitle` changes; verify with `CMAKE_BUILD_TYPE=Debug` before UI probes.

Rebuild log (earlier): [`logs/rebuild_debug_standalone.txt`](logs/rebuild_debug_standalone.txt).

---

## 2. Works

- **Unit / engine suite** — `ctest` **26/26** (engine + headless host playhead smoke + polyrhythm layer persistence + Markov trained lookup); AX pass log [`logs/ctest_ax_names.txt`](logs/ctest_ax_names.txt).
- **AU validation** — `auval -v aumi Gmid Osrc` → **AU VALIDATION SUCCEEDED**; 10 generator value strings including Polyrhythm ([`logs/auval.txt`](logs/auval.txt)).
- **VST3 pluginval** — strictness 5, `--skip-gui-tests`, in-process → **SUCCESS** ([`logs/pluginval-strict5.txt`](logs/pluginval-strict5.txt)).
- **Permissions** — Accessibility + Screen Recording granted for this agent shell; screenshots and AX probes succeed.
- **Standalone launches** — Debug Standalone after rebuild; UI screenshots captured.
- **Generator dropdown includes Polyrhythm (10 items)** — **PASS after rebuild**. Open-menu screenshot lists: Euclidean, **Polyrhythm**, Markov, L-System, Cellular, Probabilistic, Brownian, Perlin Noise, Drunk Walk, Lorenz ([`logs/standalone_generator_menu.png`](logs/standalone_generator_menu.png), [`logs/generator_menu_items.txt`](logs/generator_menu_items.txt)). Matches `PluginEditor.cpp` `addItemList`.
- **Presets UI** — Preset Manager opens with 11 factories including **Polyrhythm Layers** ([`logs/standalone_presets_menu.png`](logs/standalone_presets_menu.png), [`logs/standalone_preset_manager_open.png`](logs/standalone_preset_manager_open.png)).
- **Generator cycling (partial)** — Keyboard/cliclick cycling reached Brownian and other gens ([`logs/standalone_brownian.png`](logs/standalone_brownian.png), [`logs/standalone_ui_exercise_after_rebuild.txt`](logs/standalone_ui_exercise_after_rebuild.txt)). LFO button clickable ([`logs/standalone_lfo_toggled.png`](logs/standalone_lfo_toggled.png)). Density/Probability knobs visible in UI.
- **Transport gate (code)** — `processBlock` advances only when playhead reports playing ([`PluginProcessor.cpp`](../../Source/PluginProcessor.cpp)); Standalone without playhead free-runs.
- **REAPER present** — App launches; AX OK ([`logs/reaper_launched.png`](logs/reaper_launched.png), [`logs/reaper_phase_c.txt`](logs/reaper_phase_c.txt)).
- **REAPER transport gate via MCP (Phase C)** — **PASS**: TwelveTake MCP `create_project` → insert track → `Generative MIDI` + `ReaSynth` → play ~3.5s → stop (`get_play_state` = 0). FX resolved as `VST3: Generative MIDI (OpenSource)` + `VSTi: ReaSynth (Cockos)`. Evidence: [`logs/reaper_transport_gate_mcp.txt`](logs/reaper_transport_gate_mcp.txt), [`logs/reaper_transport_gate_mcp.png`](logs/reaper_transport_gate_mcp.png); setup: [`reaper/MCP_SETUP.md`](reaper/MCP_SETUP.md).
- **AX titles on key controls** — **PASS** — `Component::setTitle` on Generator Type, Presets, Probability, LFO Enable/Rate/Velocity/Density, Advanced ratchet/stochastic knobs, expression toggles, Polyrhythm layer editor (+ Add/Remove Layer, per-layer Division/Length/Pitch/Velocity/Enable), Preset Browser actions. System Events probe: [`logs/ax_names_probe.txt`](logs/ax_names_probe.txt) (all key names **FOUND** with `gen=Polyrhythm`).
- **Polyrhythm layer editor (Standalone)** — **PASS** — Selected Polyrhythm via named Generator Type + OCR/cliclick on menu row; UI shows **POLYRHYTHM LAYERS** with `+ Add Layer` / `- Remove Layer` and Layer 1 row (pattern + Div/Len/Pitch/Vel). Evidence: [`logs/standalone_polyrhythm_layers.png`](logs/standalone_polyrhythm_layers.png), [`logs/standalone_polyrhythm_layers_confirm.png`](logs/standalone_polyrhythm_layers_confirm.png), [`logs/polyrhythm_layer_ui_ax.txt`](logs/polyrhythm_layer_ui_ax.txt), [`logs/polyrhythm_ui_confirm_meta.txt`](logs/polyrhythm_ui_confirm_meta.txt).

---

## 3. Broken / blocked / regressions

- **Earlier false negative (resolved)** — Generator menu without Polyrhythm was a **stale Debug binary**, not missing source. Source already had 10 gens; auval already listed Polyrhythm.
- **AX popup menus still fragile for `menu item` clicks** — Opening a JUCE combo by **named** `AXPopUpButton` works; enumerating/`click menu item "Polyrhythm"` often returns **-1719 Invalid index**. Prefer: click named combo → OCR/screencapture row → `cliclick`, or load factory preset. Named titles remove the “unnamed popup” discovery problem; they do not fully fix JUCE menu AX trees.
- **No live MIDI note stream via MCP** — Phase C / ear-check assert FX chain + transport play/stop; ear/MIDI-monitor confirmation of “no new note-ons while stopped” remains a short human glance (**PASS with residual human glance**, [`logs/reaper_earcheck_mcp.txt`](logs/reaper_earcheck_mcp.txt)). **Mitigated in CI/local:** headless `GenerativeMIDIHostSmokeTests` assert note-ons while playing and zero new note-ons when stopped. Polyrhythm UI confirm did **not** attach a MIDI monitor (Standalone free-runs; DSP already covered by host smoke).
- **No expression MIDI monitor capture** — AT/PB/CC and per-generator audibility not instrumented this run.
- **Preset Manager dialog AX shallow** — Native `DialogWindow` tree often omits nested control titles under System Events (`entire contents` ~window chrome only); main editor AX titles are reliable.

No auval/pluginval/ctest failures observed on this campaign.

---

## 4. Needs improvement (prioritized)

### P0
1. **Human confirm Polyrhythm layer editor** — **DONE (agent)** — Standalone Polyrhythm selection + layer UI screenshot + AX layer titles. Optional: human ear-check that notes emit in Standalone/REAPER.
2. **Optional ear-check** — **PASS with residual human glance** — Re-ran Generative MIDI + ReaSynth via TwelveTake MCP: play ~3.5s → stop (`get_play_state` 1→0). Evidence: [`logs/reaper_earcheck_mcp.txt`](logs/reaper_earcheck_mcp.txt), [`logs/reaper_earcheck_playing.png`](logs/reaper_earcheck_playing.png), [`logs/reaper_earcheck_stopped.png`](logs/reaper_earcheck_stopped.png). No MIDI-monitor/mic proof of silence-on-stop; CI HostSmokeTests remain the objective gate.

### P1
3. **Headless MIDI emit harness** — **DONE** — `Tests/HostSmokeTests.cpp` + `GenerativeMIDIHostSmokeTests` (links `GenerativeMIDI` shared code); fake `AudioPlayHead`; playing → note-ons > 0; stopped → no new note-ons; Standalone free-run contrast documented in source comments.
4. **AX names on JUCE controls** — **DONE** — `setTitle` on key `PluginEditor` / `PolyrhythmLayerEditor` / `PresetBrowser` controls. Verify: rebuild Debug Standalone → System Events `name` of UI elements (see [`logs/ax_names_probe.txt`](logs/ax_names_probe.txt)). Catch2 for AX impractical; probe steps above.
5. **Polyrhythm layer persistence** — **DONE** — `PolyrhythmLayers` ValueTree child in session XML (`get`/`setStateInformation`) and preset state blob; schema **1.2**; Catch2 coverage in EngineTests + HostSmokeTests.

### P2
6. Document Debug vs Release CMake single-config caveat in BUILD.md — **DONE** (see [`docs/developer/BUILD.md`](../developer/BUILD.md) “Single-config caveat”).
7. Markov trained-path RT residual — **DONE (small harden)** — `generateOrDefault` reuses a pre-reserved `lookupScratch` vector key (no per-note heap after warmup). Catch2: trained lookup case in EngineTests. Map still keyed by `std::vector<int>` (learn/rebuild remains off-RT).

---

## 5. Helpful new features (grounded)

- **MIDI activity meter / last-note HUD** in Standalone.
- **Built-in MIDI log pane** for expression/channel debugging — **shipped** (collapsible bottom strip; see `auv3_sim_midi_log.jpg`).
- **“QA mode”** that cycles generators for N bars and reports note counts.
- **Transport indicator** — Host playing / Standalone free-run / Stopped.
- **IAC / virtual MIDI out** preference in Standalone.

---

## 6. Refactor / tech debt

- ~~Link a **HostSmoke** Catch2 target to `GenerativeMIDIProcessor`.~~ Done (`GenerativeMIDIHostSmokeTests`).
- Split [`PluginEditor.cpp`](../../Source/PluginEditor.cpp) layout vs control wiring.
- ~~Persist Polyrhythm layer state in `getStateInformation` / preset XML.~~ Done (schema 1.2).
- Separate `build-debug/` / `build-release/` dirs to avoid single-config artefact confusion.
- ~~AX titles on primary editor controls.~~ Done (`setTitle`).

---

## 7. Recommended next engineering slice

**Closable Host QA / P2 housekeeping for this campaign is done.** Remaining items below are deferred product work (out of scope for this pass):

1. Full mod matrix / deeper expression MIDI monitoring in-host.
2. MPE, Polyrhythm step-grid polish, AUv3 App Store packaging.
3. Optional in-DAW MIDI activity probe if agent ear-checks without a human glance become a hard requirement.

---

## Harness scorecard

| Harness | Used? | Effective? |
|---------|-------|------------|
| ctest | Yes | High for DSP/helpers + host playhead gate (26 cases) |
| auval | Yes | High for AU compliance |
| pluginval VST3 | Yes | High for load/crash/state; weak for UI/MIDI feel |
| Standalone UI | Launch + screenshots + AX-named cliclick/OCR | **High** for menu presence + Polyrhythm layer UI |
| REAPER | Launch + MCP FX insert + play/stop | **High** for session/transport; medium for MIDI ear-gate |
| Logic | Not exercised | — |
| AUv3 device | Out of scope | — |
| Reaper MCP (TwelveTake) | Yes | **High** for project/track/FX/transport |
| System Events AX | Yes | **High** for named controls; weak for JUCE popup menus |

---

## Smoke checklist pointer

Filled agent results: [`logs/SMOKE_CHECKLIST_RESULTS.md`](logs/SMOKE_CHECKLIST_RESULTS.md). Master checklist template: [`docs/user/SMOKE_CHECKLIST.md`](../user/SMOKE_CHECKLIST.md).

### AX verification steps (no Catch2)

1. `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build --target GenerativeMIDI_Standalone`
2. Launch Debug Standalone; grant Accessibility if needed.
3. System Events: confirm UI elements named `Generator Type`, `Presets`, `Probability`, `LFO Enable`, `LFO Velocity`, and (after selecting Polyrhythm) `Add Layer` / `Layer 1 Division`.
4. Expect: named `AXPopUpButton`/`AXButton` discovery works; `menu item` clicks may still -1719 — use screencapture/OCR/`cliclick` for menu rows.
