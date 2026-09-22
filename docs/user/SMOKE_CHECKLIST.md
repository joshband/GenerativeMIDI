# Standalone / DAW Smoke Checklist

**Product:** GenerativeMIDI **v0.8.0**  
**Kind:** Manual smoke only — not automated UI tests  
**Baseline commit:** ~`f6ef718` (master)

Use this after a local CMake build (or a known-good binary) to confirm the live editor path still behaves. Check boxes as you go; leave notes if something fails.

**Prerequisites**

- Build Standalone (+ AU or VST3 for the DAW section): see [BUILD.md](../developer/BUILD.md)
- Route MIDI out to a soft synth / sampler so notes are audible
- Optional: a MIDI monitor (host MIDI log, MIDI Monitor, etc.) for expression checks

---

## 1. All 9 generators produce MIDI (Standalone)

Open **Generative MIDI.app**. Standalone has no host playhead — generation should run without pressing DAW play.

For each generator in the dropdown, confirm notes emit (audible and/or MIDI monitor) within a few bars:

| # | Generator | Pass? | Notes |
|---|-----------|-------|-------|
| 0 | Euclidean | ☐ | |
| 1 | Markov | ☐ | |
| 2 | L-System | ☐ | |
| 3 | Cellular | ☐ | |
| 4 | Probabilistic | ☐ | |
| 5 | Brownian | ☐ | |
| 6 | Perlin | ☐ | |
| 7 | Drunk Walk | ☐ | |
| 8 | Lorenz | ☐ | |

**Out of scope:** Polyrhythm is deferred from the UI / APVTS choice list (engine retained only).

**Hints if silent:** density / pulses > 0; tempo sane; MIDI channel matches the instrument; velocity range not zeroed.

---

## 2. Ten factory presets load and change behavior

Open **Presets**. Load each factory preset (double-click or prev/next). Confirm the UI updates (generator / key knobs) and audible or visible pattern changes vs the previous preset:

| # | Factory preset | Pass? | Notes |
|---|----------------|-------|-------|
| 0 | Euclidean Basic | ☐ | |
| 1 | Euclidean Complex | ☐ | |
| 2 | Brownian Drift | ☐ | |
| 3 | Markov Melody | ☐ | |
| 4 | L-System Fractal | ☐ | |
| 5 | Cellular Automata | ☐ | |
| 6 | Probabilistic Sparse | ☐ | |
| 7 | Ratchet Groove | ☐ | |
| 8 | Ambient Drift | ☐ | |
| 9 | Percussive Hits | ☐ | |

Factory presets use the live **9-generator** APVTS indices. See [PRESET_GUIDE.md](PRESET_GUIDE.md).

---

## 3. Host transport stop gates notes (DAW); Standalone still free-runs

Clock advance rule: when a playhead exists, generation follows **isPlaying**; when no playhead (Standalone), the clock always advances.

| Context | Expectation | Pass? | Notes |
|---------|-------------|-------|-------|
| **Standalone** | Notes keep generating with no DAW transport | ☐ | |
| **DAW — Play** | Notes generate while host is playing | ☐ | |
| **DAW — Stop** | New notes stop while host is stopped (playhead present) | ☐ | |

Any AU/VST3 host with a real playhead is fine (REAPER, Logic, Ableton, etc.). Hang notes already scheduled may still release; the check is that **new** generative note-ons stop while transport is stopped.

---

## 4. MIDI expression: AT / PB / CC on note-on (static PB — not MPE)

In the **EXPRESSION** section, with MIDI monitoring (or a synth that reacts to AT/CC/PB):

| Toggle | Amount / control | Expectation on each note-on | Pass? | Notes |
|--------|------------------|-----------------------------|-------|-------|
| **AT** enable | Aftertouch amount | Polyphonic aftertouch for that note | ☐ | |
| **CC** enable | CC number + amount | CC message at note-on | ☐ | |
| **PB** enable | PB Semi (1–24) | Static pitch-bend wheel offset (not continuous / not MPE) | ☐ | |
| All expression **off** | — | Note-ons still emit; no AT / CC / PB from those toggles | ☐ | |

**Honest limits (v0.8.0):** per-note emit on note-on only. Continuous CC/PB modulation and MPE are deferred ([STATUS.md](../../STATUS.md)).

---

## Sign-off

| Field | Value |
|-------|-------|
| Build / binary | |
| Tester | |
| Date | |
| Result (pass / fail + blockers) | |

Automated coverage elsewhere: Catch2 / `ctest` engine and preset round-trips — not a substitute for this UI/host smoke.
