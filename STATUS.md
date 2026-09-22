# GenerativeMIDI - Development Status

**Last Updated**: 2026-09-22  
**Current Version**: v0.8.0  
**Repository**: https://github.com/joshband/GenerativeMIDI  
**Build**: [![CI](https://github.com/joshband/GenerativeMIDI/actions/workflows/ci.yml/badge.svg)](https://github.com/joshband/GenerativeMIDI/actions/workflows/ci.yml)

## Honest product snapshot

| Claim | Reality |
|-------|---------|
| Generators in editor UI | **9** (Euclidean + 4 algorithmic + 4 stochastic) |
| Polyrhythm | Engine sources retained; **deferred** from UI / APVTS choice list |
| Modulation matrix | Archived under `archive/modulation_v1/` — not live |
| AUv3 / iOS | CMake target + docs exist; **not App Store–ready** |
| Touch / a11y | No dedicated touch redesign claimed |
| Tests | Catch2 + `ctest` in CMake / CI |
| Canonical build | **CMake** (`GenerativeMIDI.jucer` deprecated) |

Showcase (architecture + evidence): https://joshband.github.io/GenerativeMIDI/

## Completed features (summary)

### v0.8.0 — MIDI channel & UI polish
- MIDI channel routing (1–16)
- Stochastic UI knobs (step size, momentum, time scale)
- Color-coded generator labels
- Live Euclidean pattern playhead
- Context-aware control enablement
- MIDI expression UI (AT / PB / CC) with per-note emit on note-on

### v0.7.x — Stochastic / chaos + dynamic UI
- Brownian, Perlin, Drunk Walk, Lorenz
- Generator-dependent control enabling / opacity

### v0.6.x — Presets
- XML `.gmpreset` save/load, browser, import/export
- **10 factory presets** live (9-generator APVTS indices; Polyrhythm not included)

### v0.5.x — Ratcheting + layout
- Probability-based retriggers, decay, gate length / legato

### v0.4.0 — Gate length
- Duration as % of step; legato mode

### v0.3.0 — Gilded steampunk UI
- CustomLookAndFeel brass / gold / copper theme

### v0.2.0 — Scale & humanization
- 16 scales, swing templates, timing / velocity humanize

### v0.1.0 — Core engines
- Euclidean + algorithmic + (later) stochastic paths; AU / VST3 / Standalone

## In progress

None for this refactor wave — see CI for live build status.

## Planned (explicitly deferred)

### Polyrhythm layer editor
`PolyrhythmEngine` kept for a future branch; no layer UI in the current editor.

### MIDI expression depth
UI + per-note emit shipped; continuous CC/PB modulation and MPE remain deferred.

## Project metrics

| Metric | Value |
|--------|-------|
| Product version | v0.8.0 |
| UI generators | 9 |
| Polyrhythm in UI | Deferred |
| Build | CI workflow `.github/workflows/ci.yml` |
| Tests | `ctest` (Catch2 engine tests) |
| Docs | README, FEATURES, GETTING_STARTED, BUILD, Pages showcase |

## Notes

- Prefer CMake; initialize `art/` with `git submodule update --init --recursive`.
- Unused `BitmapUIComponent` moved to `archive/`.
- Do not treat session “token budget” notes in historical docs as project status.

**License**: MIT  
**Repository**: https://github.com/joshband/GenerativeMIDI
