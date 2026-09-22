# GenerativeMIDI QA — minimal REAPER transport project notes

## Goal

Validate host playhead gating: notes generate while playing; **new** note-ons stop when transport stops.

## Setup (operator or agent with UI access)

1. Open REAPER (`/Applications/REAPER.app`).
2. New project → Insert track.
3. FX → **Generative MIDI** (Audio Unit `aumi Gmid Osrc` or VST3 `Generative MIDI`).
4. Add a soft synth after it (e.g. ReaSynth) or use a MIDI logger.
5. Run [`transport_gate_probe.lua`](transport_gate_probe.lua) via Actions → ReaScript, **or** manually Play ~3s then Stop.

## Expected

| State | Expectation |
|-------|-------------|
| Play | Audible / monitored generative MIDI |
| Stop | No new note-ons from Generative MIDI (gate in `processBlock` when `AudioPlayHead` reports not playing) |
| Contrast | Standalone keeps generating (no playhead) |

## Agent run status (2026-09-22)

- REAPER is installed; no Reaper MCP in Cursor.
- Attempted automation limited by Accessibility (same -25211 as Standalone UI).
- Project file: optional — loading AU by GUID in `.rpp` is host-version fragile; prefer the Lua probe + manual FX insert for reproducibility.

## How to record evidence

- Note PASS/FAIL in `docs/qa/QA_FINDINGS_v0.8.0.md` Phase C section.
- Attach MIDI monitor screenshot if Screen Recording is granted later.
