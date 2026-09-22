# GenerativeMIDI QA Findings — v0.8.0

**Date:** 2026-09-22  
**Commit:** `dd809e9abe4b055a396156d382cf61a19ac55b4e`  
**Campaign:** [Host QA Exercise Plan](../../.cursor/plans/host_qa_exercise_plan_9a5ca05f.plan.md) (plan file may live under user `.cursor/plans/`)

## 1. Environment

| Item | Value |
|------|--------|
| OS | macOS (darwin 25.x) |
| Formats tested | AU (`aumi Gmid Osrc`), VST3 Debug artefact, Standalone Debug |
| Hosts present | REAPER.app, Logic Pro.app |
| Automated | `ctest` 20/20, `auval` PASS, pluginval v1.0.4 strictness 5 SUCCESS |
| Blocked | Accessibility for `osascript` (−25211); Screen Recording for screenshots |
| Reaper / DAW MCP | **Not available** in Cursor — used install + Lua probe docs only |

Logs: [`docs/qa/logs/`](logs/) (text only in git; download pluginval locally per [`docs/developer/CI.md`](../developer/CI.md)).

---

## 2. Works

- **Unit / engine suite** — `ctest` **20/20** ([`logs/ctest.txt`](logs/ctest.txt)).
- **AU validation** — `auval -v aumi Gmid Osrc` → **AU VALIDATION SUCCEEDED**; 10 generator value strings including Polyrhythm ([`logs/auval.txt`](logs/auval.txt)).
- **VST3 pluginval** — strictness 5, `--skip-gui-tests`, in-process → **SUCCESS** exit 0 ([`logs/pluginval-strict5.txt`](logs/pluginval-strict5.txt)). MIDI-effect buses correctly report 0 audio I/O channels.
- **Standalone launches** — process `Generative MIDI` starts from Debug Standalone `.app`.
- **Factory / migration / LFO math** — covered by Catch2 (11 factories, 9→10 migration helpers, ModLfo).
- **Transport gate (code)** — `processBlock` only advances clock when `AudioPlayHead::getIsPlaying()` is true when a playhead exists ([`PluginProcessor.cpp`](../../Source/PluginProcessor.cpp) ~372–377); Standalone without playhead always advances (documented).

---

## 3. Broken / blocked / regressions

- **Interactive UI automation blocked** — Assistive access denied to `osascript`; cannot script generator combo, presets, or knobs ([`logs/standalone_ax_probe.txt`](logs/standalone_ax_probe.txt)).
- **Screenshot evidence blocked** — Screen Recording not granted ([`logs/screenshot_attempt.txt`](logs/screenshot_attempt.txt)).
- **REAPER interactive transport not agent-verified** — REAPER launches, but AX denied; play/stop MIDI confirmation remains **operator-assisted** via [`docs/qa/reaper/`](reaper/).
- **No MIDI monitor capture this run** — expression AT/PB/CC and per-generator audibility not instrumented; do not mark as PASS for “feel.”

No auval/pluginval/ctest failures observed on this commit.

---

## 4. Needs improvement (prioritized)

### P0
1. **Grant Accessibility + Screen Recording** to the agent/terminal (or run a human pass) to finish [`SMOKE_CHECKLIST.md`](../user/SMOKE_CHECKLIST.md) for generators, LFO, expression, and REAPER stop.
2. **Operator REAPER pass** using [`reaper/transport_gate_probe.lua`](reaper/transport_gate_probe.lua) — confirm stop gates new note-ons.

### P1
3. **Headless MIDI emit harness** — small console/`ctest` target linking processor + fake `AudioPlayHead` to assert note-ons per generator and zero note-ons when stopped (UI-independent). Attempted during this campaign but not wired into CMake (plugin link weight); worth a dedicated slice.
4. **pluginval GUI / higher strictness** — optional local run without `--skip-gui-tests` once Accessibility works; expect MIDI-FX flake — document only.
5. **Polyrhythm layer persistence** — layer patterns/offsets not in APVTS/session XML; reload loses UI edits (product gap vs factory default seed).

### P2
6. Editor monolith / AX names — JUCE controls lack accessible names useful for UI automation.
7. Markov trained-path RT residual (STATUS) — still a honesty item for audio-thread alloc.

---

## 5. Helpful new features (grounded)

- **MIDI activity meter / last-note HUD** in Standalone — proves generation without external synth.
- **Built-in MIDI log pane** (last N events) for expression/channel debugging.
- **“QA mode”** button that cycles generators for N bars and reports note counts (doubles as demo).
- **Transport indicator** — “Host playing / Standalone free-run / Stopped” so DAW gating is visible.
- **IAC / virtual MIDI out** preference in Standalone for monitor-only workflows.

---

## 6. Refactor / tech debt

- Link a **HostSmoke** console or Catch2 target to `GenerativeMIDIProcessor` without duplicating the whole plugin graph (shared static lib).
- Split [`PluginEditor.cpp`](../../Source/PluginEditor.cpp) layout vs control wiring for testability.
- Persist Polyrhythm layer state in `getStateInformation` / preset XML.
- Reaper MCP or official ReaScript CI job if interactive host QA becomes recurrent.

---

## 7. Recommended next engineering slice (top 3)

1. **Human or Accessibility-enabled smoke** — complete Standalone + REAPER transport checklist; attach one screenshot.
2. **Headless processor smoke in `ctest`** — fake playhead, note-on counts per generator + stop gate.
3. **Polyrhythm layer state save/load** — so experimental UI survives session reload.

---

## Harness scorecard

| Harness | Used? | Effective? |
|---------|-------|------------|
| ctest | Yes | High for DSP/helpers |
| auval | Yes | High for AU compliance |
| pluginval VST3 | Yes | High for load/crash/state; weak for UI/MIDI feel |
| Standalone UI | Launch only | Low without Accessibility |
| REAPER | Launch + Lua docs | Medium after operator run |
| Logic | Not exercised | — |
| AUv3 device | Out of scope | — |
| Reaper MCP | N/A | — |

## Smoke checklist pointer

Filled agent results: [`logs/SMOKE_CHECKLIST_RESULTS.md`](logs/SMOKE_CHECKLIST_RESULTS.md). Master checklist template: [`docs/user/SMOKE_CHECKLIST.md`](../user/SMOKE_CHECKLIST.md).
