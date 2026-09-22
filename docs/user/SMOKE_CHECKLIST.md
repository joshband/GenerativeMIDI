# Standalone / DAW Smoke Checklist

**Product:** GenerativeMIDI **v0.8.0**  
**Kind:** Manual smoke only — not automated UI tests  
**Baseline:** current `master` (10 generators + LFO MVP)

Use this after a local CMake build (or a known-good binary). Check boxes as you go; leave notes if something fails.

**Prerequisites**

- Build Standalone (+ AU or VST3 for the DAW section): see [BUILD.md](../developer/BUILD.md)
- Route MIDI out to a soft synth / sampler so notes are audible
- Optional: a MIDI monitor for expression / channel checks

---

## 1. All 10 generators produce MIDI (Standalone)

Open **Generative MIDI.app**. Standalone has no host playhead — generation should run without pressing DAW play.

| # | Generator | Pass? | Notes |
|---|-----------|-------|-------|
| 0 | Euclidean | ☐ | |
| 1 | Polyrhythm | ☐ | Experimental — default layers should emit; try pitch/vel transforms + division |
| 2 | Markov | ☐ | |
| 3 | L-System | ☐ | |
| 4 | Cellular | ☐ | |
| 5 | Probabilistic | ☐ | |
| 6 | Brownian | ☐ | |
| 7 | Perlin | ☐ | |
| 8 | Drunk Walk | ☐ | |
| 9 | Lorenz | ☐ | |

**Hints if silent:** density / pulses > 0; tempo sane; MIDI channel matches the instrument; velocity range not zeroed.

---

## 2. Factory presets load and change behavior

Open **Presets**. Load each factory. Confirm UI updates and audible/visible change:

| Check | Pass? | Notes |
|-------|-------|-------|
| Each listed factory loads without crash | ☐ | |
| Generator / knobs update | ☐ | |
| Polyrhythm factory (if present) selects index 1 | ☐ | |

See [PRESET_GUIDE.md](PRESET_GUIDE.md). Indices match the **10-generator** APVTS layout.

---

## 3. Host transport stop gates notes (DAW); Standalone free-runs

| Context | Expectation | Pass? | Notes |
|---------|-------------|-------|-------|
| **Standalone** | Notes keep generating with no DAW transport | ☐ | |
| **DAW — Play** | Notes generate while host is playing | ☐ | |
| **DAW — Stop** | New notes stop while host is stopped | ☐ | |

**Agent path (REAPER):** With TwelveTake MCP configured ([MCP_SETUP.md](../qa/reaper/MCP_SETUP.md)), create a clean project, insert Generative MIDI + ReaSynth ([FX_NAMES.md](../qa/reaper/FX_NAMES.md)), play ~3s, stop. Or run [`create_transport_gate_session.lua`](../qa/reaper/create_transport_gate_session.lua) then [`transport_gate_probe.lua`](../qa/reaper/transport_gate_probe.lua). Latest campaign results: [SMOKE_CHECKLIST_RESULTS.md](../qa/logs/SMOKE_CHECKLIST_RESULTS.md).

---

## 4. MIDI expression: AT / PB / CC on note-on (static PB — not MPE)

In **EXPRESSION**, with MIDI monitoring:

| Control | Expectation | Pass? | Notes |
|---------|-------------|-------|-------|
| AT enable + amount | Poly aftertouch on note-on | ☐ | |
| PB enable + semis | Pitch bend on note-on (static offset) | ☐ | |
| CC enable + # + amount | CC on note-on | ☐ | |

---

## 5. MIDI channel routing

| Check | Pass? | Notes |
|-------|-------|-------|
| Channel combo 1–16 changes output channel for generated notes | ☐ | |

---

## 6. ADVANCED LFO (velocity + density)

| Check | Pass? | Notes |
|-------|-------|-------|
| **LFO Vel** on + Depth > 0 → velocity breathes over time | ☐ | |
| **LFO Dens** depth > 0 → note density / miss rate wobbles | ☐ | |
| LFO off → steady density/velocity behavior | ☐ | |

---

## Sign-off

| Field | Value |
|-------|-------|
| Date | |
| Build / commit | |
| Tester | |
| Result | ☐ Pass · ☐ Fail (attach notes) |
