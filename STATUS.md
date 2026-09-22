# GenerativeMIDI - Development Status

**Last Updated**: 2026-09-22  
**Current Version**: v0.8.0  
**Repository**: https://github.com/joshband/GenerativeMIDI  
**Build**: [![CI](https://github.com/joshband/GenerativeMIDI/actions/workflows/ci.yml/badge.svg)](https://github.com/joshband/GenerativeMIDI/actions/workflows/ci.yml)

## Honest product snapshot

| Claim | Reality |
|-------|---------|
| Generators in editor UI | **10** on `feature/polyrhythm-ui` (Euclidean + Polyrhythm + 4 algorithmic + 4 stochastic). **Master remains 9** until this branch merges. |
| `stochasticType` APVTS param | **Legacy** — kept for session load; non-automatable; **unused by DSP** (`generatorType` is source of truth) |
| Polyrhythm | **Experimental restore** on `feature/polyrhythm-ui`: selectable in APVTS/combo, DSP wired, minimal layer editor. Not on master yet. |
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
- **10 factory presets** live (indices remapped for 10-generator APVTS on this branch)

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

### Polyrhythm restore (`feature/polyrhythm-ui`)
- Re-added to APVTS choice list + editor combo (index 1; algorithmic/stochastic shifted +1)
- `polyrhythmEngine` wired into `onSubdivisionHit` / generation path
- Minimal `PolyrhythmLayerEditor` resurrected (enable / division / length / add-remove layers)
- Factory presets + Catch2 mapping tests updated for new indices

## Planned (explicitly deferred)

### Polyrhythm polish (post-MVP)
- Per-layer pitch/velocity slider wiring beyond logging
- Use layer `division` for true rate scaling (currently all layers advance 1:1 per clock subdivision)
- Pattern step editing / polymeter visualization polish
- Session migration for older 9-index user presets that land on shifted algorithmic/stochastic slots
- Dedicated factory preset for Polyrhythm (APVTS index 1)

### MIDI expression depth
UI + per-note emit shipped; continuous CC/PB modulation and MPE remain deferred.

### Modulation matrix
Still archived — not part of this epic.

## Project metrics

| Metric | Value |
|--------|-------|
| Product version | v0.8.0 |
| UI generators (this branch) | 10 |
| Polyrhythm in UI | Experimental (feature branch) |
| Build | CI workflow `.github/workflows/ci.yml` |
| Tests | `ctest` (Catch2 engine tests) |
| Docs | README, FEATURES, GETTING_STARTED, BUILD, Pages showcase, [SMOKE_CHECKLIST](docs/user/SMOKE_CHECKLIST.md) |

## Notes

- Prefer CMake; initialize `art/` with `git submodule update --init --recursive`.
- Unused `BitmapUIComponent` moved to `archive/`.
- Do not treat session “token budget” notes in historical docs as project status.
- **Realtime**: Euclidean regen, ratchet offsets, algorithmic single-note path, and EventScheduler are allocation-free on the steady-state audio path (queue pre-reserved in `prepareToPlay`; over-capacity schedules are dropped). Remaining risk: trained Markov still builds a map key vector on lookup (default untrained path is fine). Polyrhythm path uses the same `scheduleNote` helper (expression MIDI intact).

**License**: MIT  
**Repository**: https://github.com/joshband/GenerativeMIDI
