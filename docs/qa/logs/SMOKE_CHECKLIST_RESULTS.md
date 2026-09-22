# Smoke Checklist Results — QA campaign 2026-09-22

**Commit:** `dd809e9`  
**Harness notes:** Standalone process launched successfully. **Accessibility denied** (`osascript` error -25211). **Screen Recording denied** (screenshot helper). Interactive UI clicks and window capture were **not** possible from the agent. Items below use: automated validators + process evidence + code/auval param list; UI feel marked **manual-required**.

| Section | Result | Evidence |
|---------|--------|----------|
| 1. 10 generators listed | **PASS (auval)** | `auval` enumerates Euclidean…Lorenz (10 strings). MIDI emit per generator: **manual-required** in Standalone |
| 1. Polyrhythm experimental | **PASS (param)** | Index 1 string `Polyrhythm` in auval |
| 2. Factory presets | **PASS (ctest)** | 11 factories round-trip in Catch2; UI load **manual-required** |
| 3. Standalone free-run | **PASS (process)** | PID running `Generative MIDI` Standalone binary; always-run clock is code path when no playhead |
| 3. DAW Play/Stop gate | **Deferred to Phase C** | REAPER script/project |
| 4. Expression AT/PB/CC | **PARTIAL** | Params present in AU (auval); note-on emit **manual-required** |
| 5. MIDI channel 1–16 | **PARTIAL** | Param in APVTS/auval; routing **manual-required** |
| 6. LFO Vel / Dens | **PARTIAL** | Params `modLfoEnable`, `modLfoDepth`, `modLfoDensityDepth` in processor; audible check **manual-required** |

## Accessibility / capture

```
AX_ERROR -25211: osascript is not allowed assistive access.
Screen Recording is still not granted.
```

## Sign-off

| Field | Value |
|-------|-------|
| Date | 2026-09-22 |
| Build / commit | Debug Standalone + AU/VST3 @ dd809e9 |
| Tester | Agent (automated + blocked UI) |
| Result | Automated gates PASS; interactive UI **blocked** pending Accessibility/Screen Recording |
