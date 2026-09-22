# GenerativeMIDI QA Findings — v0.8.0

**Date:** 2026-09-22  
**Commit:** `b1e8f0b`  
**Campaign:** Host QA Exercise Plan + REAPER MCP Phase C

## 1. Environment

| Item | Value |
|------|--------|
| OS | macOS (darwin 25.x) |
| Formats tested | AU (`aumi Gmid Osrc`), VST3 Debug artefact, **Debug Standalone (rebuilt)** |
| Hosts present | REAPER.app (launched), Logic Pro.app (not exercised) |
| Automated | `ctest` **25/25** (engine + host smoke + polyrhythm persistence), `auval` PASS, pluginval v1.0.4 strictness 5 SUCCESS |
| Permissions (re-verified) | **Accessibility OK** (System Events); **Screen Recording OK** (`screencapture`) — [`logs/permissions_check.txt`](logs/permissions_check.txt) |
| Reaper / DAW MCP | **TwelveTake** `user-reaper` (`uvx twelvetake-reaper-mcp`) — bridge via `__startup.lua` |

Logs: [`docs/qa/logs/`](logs/).

### Rebuild note (stale Debug binary)

Earlier Phase B used a **stale Debug Standalone** (`mtime` Sep 22 **14:40**) while `CMAKE_BUILD_TYPE` was **Release**, so Release Standalone relinked (18:22) but Debug did not. Force reconfigure `-DCMAKE_BUILD_TYPE=Debug` + `cmake --build build --target GenerativeMIDI_Standalone` produced a fresh binary:

| Artefact | mtime |
|----------|-------|
| `Source/PluginEditor.cpp` | 2026-09-22 18:23:33 |
| `PluginEditor.cpp.o` | 2026-09-22 18:23:37 |
| Debug Standalone `Generative MIDI` | **2026-09-22 18:23:48** |

Rebuild log: [`logs/rebuild_debug_standalone.txt`](logs/rebuild_debug_standalone.txt).

---

## 2. Works

- **Unit / engine suite** — `ctest` **25/25** (engine + headless host playhead smoke + polyrhythm layer persistence; prior campaign log still [`logs/ctest.txt`](logs/ctest.txt) at 20/20).
- **AU validation** — `auval -v aumi Gmid Osrc` → **AU VALIDATION SUCCEEDED**; 10 generator value strings including Polyrhythm ([`logs/auval.txt`](logs/auval.txt)).
- **VST3 pluginval** — strictness 5, `--skip-gui-tests`, in-process → **SUCCESS** ([`logs/pluginval-strict5.txt`](logs/pluginval-strict5.txt)).
- **Permissions** — Accessibility + Screen Recording granted for this agent shell; screenshots and AX probes succeed.
- **Standalone launches** — Debug Standalone after rebuild; UI screenshots captured.
- **Generator dropdown includes Polyrhythm (10 items)** — **PASS after rebuild**. Open-menu screenshot lists: Euclidean, **Polyrhythm**, Markov, L-System, Cellular, Probabilistic, Brownian, Perlin Noise, Drunk Walk, Lorenz ([`logs/standalone_generator_menu.png`](logs/standalone_generator_menu.png), [`logs/generator_menu_items.txt`](logs/generator_menu_items.txt)). Matches `PluginEditor.cpp` `addItemList`.
- **Presets UI** — Preset Manager opens with 11 factories including **Polyrhythm Layers** ([`logs/standalone_presets_menu.png`](logs/standalone_presets_menu.png)).
- **Generator cycling (partial)** — Keyboard/cliclick cycling reached Brownian and other gens ([`logs/standalone_brownian.png`](logs/standalone_brownian.png), [`logs/standalone_ui_exercise_after_rebuild.txt`](logs/standalone_ui_exercise_after_rebuild.txt)). LFO button clickable ([`logs/standalone_lfo_toggled.png`](logs/standalone_lfo_toggled.png)). Density/Probability knobs visible in UI.
- **Transport gate (code)** — `processBlock` advances only when playhead reports playing ([`PluginProcessor.cpp`](../../Source/PluginProcessor.cpp)); Standalone without playhead free-runs.
- **REAPER present** — App launches; AX OK ([`logs/reaper_launched.png`](logs/reaper_launched.png), [`logs/reaper_phase_c.txt`](logs/reaper_phase_c.txt)).
- **REAPER transport gate via MCP (Phase C)** — **PASS**: TwelveTake MCP `create_project` → insert track → `Generative MIDI` + `ReaSynth` → play ~3.5s → stop (`get_play_state` = 0). FX resolved as `VST3: Generative MIDI (OpenSource)` + `VSTi: ReaSynth (Cockos)`. Evidence: [`logs/reaper_transport_gate_mcp.txt`](logs/reaper_transport_gate_mcp.txt), [`logs/reaper_transport_gate_mcp.png`](logs/reaper_transport_gate_mcp.png); setup: [`reaper/MCP_SETUP.md`](reaper/MCP_SETUP.md).

---

## 3. Broken / blocked / regressions

- **Earlier false negative (resolved)** — Generator menu without Polyrhythm was a **stale Debug binary**, not missing source. Source already had 10 gens; auval already listed Polyrhythm.
- **AX popup menus fragile** — After opening a JUCE combo, `pop up button 1` often returns **-1719 Invalid index**; menu item enumeration fails. Prefer `screencapture` + `cliclick` for menu proof (as used here).
- **Reliable programmatic select of Polyrhythm** — Hit-testing menu rows / keyboard nav was off-by-one; agent did **not** consistently land on Polyrhythm selected state. Menu **presence** is proven; layer-editor screenshot under Polyrhythm selection remains incomplete.
- **No live MIDI note stream via MCP** — Phase C asserts FX chain + transport play/stop; ear/MIDI-monitor confirmation of “no new note-ons while stopped” is still a short human glance (MCP does not tee generative FX MIDI). **Mitigated in CI/local:** headless `GenerativeMIDIHostSmokeTests` assert note-ons while playing and zero new note-ons when stopped.
- **No expression MIDI monitor capture** — AT/PB/CC and per-generator audibility not instrumented this run.

No auval/pluginval/ctest failures observed on this commit.

---

## 4. Needs improvement (prioritized)

### P0
1. **Human confirm Polyrhythm layer editor** — Select Polyrhythm (or load Polyrhythm Layers preset) and verify layer UI + MIDI emit.
2. **Optional ear-check** — With Phase C session still loaded, confirm silence/no new note-ons after Stop (MCP cannot stream generative MIDI).

### P1
3. **Headless MIDI emit harness** — **DONE** — `Tests/HostSmokeTests.cpp` + `GenerativeMIDIHostSmokeTests` (links `GenerativeMIDI` shared code); fake `AudioPlayHead`; playing → note-ons > 0; stopped → no new note-ons; Standalone free-run contrast documented in source comments.
4. **AX names on JUCE controls** — Named accessibility for combo/menu automation (avoid -1719 race).
5. **Polyrhythm layer persistence** — **DONE** — `PolyrhythmLayers` ValueTree child in session XML (`get`/`setStateInformation`) and preset state blob; schema **1.2**; Catch2 coverage in EngineTests + HostSmokeTests.

### P2
6. Document Debug vs Release CMake single-config caveat in BUILD.md (stale Debug artefacts when `CMAKE_BUILD_TYPE=Release`).
7. Markov trained-path RT residual (STATUS honesty item).

---

## 5. Helpful new features (grounded)

- **MIDI activity meter / last-note HUD** in Standalone.
- **Built-in MIDI log pane** for expression/channel debugging.
- **“QA mode”** that cycles generators for N bars and reports note counts.
- **Transport indicator** — Host playing / Standalone free-run / Stopped.
- **IAC / virtual MIDI out** preference in Standalone.

---

## 6. Refactor / tech debt

- ~~Link a **HostSmoke** Catch2 target to `GenerativeMIDIProcessor`.~~ Done (`GenerativeMIDIHostSmokeTests`).
- Split [`PluginEditor.cpp`](../../Source/PluginEditor.cpp) layout vs control wiring.
- ~~Persist Polyrhythm layer state in `getStateInformation` / preset XML.~~ Done (schema 1.2).
- Separate `build-debug/` / `build-release/` dirs to avoid single-config artefact confusion.

---

## 7. Recommended next engineering slice (top 3)

1. **AX names on JUCE controls** — named accessibility for combo/menu automation (P1 item 4).
2. **Optional MCP MIDI-monitor / activity probe** for stop-gate without ear check.
3. **Human confirm Polyrhythm layer editor** — remaining P0 manual check.

---

## Harness scorecard

| Harness | Used? | Effective? |
|---------|-------|------------|
| ctest | Yes | High for DSP/helpers + host playhead gate (22 cases) |
| auval | Yes | High for AU compliance |
| pluginval VST3 | Yes | High for load/crash/state; weak for UI/MIDI feel |
| Standalone UI | Launch + screenshots + partial cliclick | **High for menu presence**; medium for control cycling |
| REAPER | Launch + MCP FX insert + play/stop | **High** for session/transport; medium for MIDI ear-gate |
| Logic | Not exercised | — |
| AUv3 device | Out of scope | — |
| Reaper MCP (TwelveTake) | Yes | **High** for project/track/FX/transport |

---

## Smoke checklist pointer

Filled agent results: [`logs/SMOKE_CHECKLIST_RESULTS.md`](logs/SMOKE_CHECKLIST_RESULTS.md). Master checklist template: [`docs/user/SMOKE_CHECKLIST.md`](../user/SMOKE_CHECKLIST.md).
