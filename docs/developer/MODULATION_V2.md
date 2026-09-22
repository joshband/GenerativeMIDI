# Modulation v2

**Status:** MVP on **master** — LFO → velocity shipped; LFO → density depth follows. Full matrix / multi-destination panel deferred.  
**Archive reference:** `archive/modulation_v1/` (do not half-wire those includes into the live tree).

## Why not restore v1?

`archive/modulation_v1/` is a useful design sketch, not a drop-in:

| v1 piece | Issue on current master |
|----------|-------------------------|
| `ModulationMatrix` | Heap `vector` / `unique_ptr` growth; string `parameterID` lookups on the render path |
| `ModulationSource` hierarchy | Virtual calls + mutable RNG in `const` methods; not aligned with recent RT hardening |
| `ModulationPanel` / `ModulationTarget` | Drag-drop UI, SynaptikUIToolkit path hacks, `CustomLookAndFeel` include layout that does not match `Source/UI/` |
| Include graph | Assumes `../Modulation/` next to a different editor layout |

v2 keeps the *ideas* (sources → depth → destinations) and rebuilds a compiling, RT-safe path.

## Architecture (target)

```
┌─────────────┐     depth      ┌──────────────┐     apply      ┌──────────────────┐
│ Mod sources │ ─────────────► │ Router (v2+) │ ─────────────► │ Destinations     │
│ LFO, …      │   bipolar ±1   │ fixed slots  │   clamp/scale  │ velocity, dens…  │
└─────────────┘                └──────────────┘                └──────────────────┘
        ▲                              ▲
        │ APVTS raw loads              │ no String IDs on audio thread
        │ advance(Δt) on audio thread  │ destinations = enum / fixed table
```

### What modulates what (roadmap)

| Destination | MVP | Later |
|-------------|-----|-------|
| Note velocity | **Yes** (LFO bipolar × depth) | Multi-source sum, unipolar option |
| Note density / probability | **Yes** (`modLfoDensityDepth`) | Multi-source sum |
| Gate length | — | After velocity path proves out |
| Pitch / ratchet / expression amounts | — | Explicit allow-list; never free-form APVTS strings |

### Sources (roadmap)

1. **LFO** (MVP): bipolar sine, rate Hz, depth, enable  
2. Random / S&H  
3. Envelope (note-triggered)  
4. Optional host/tempo sync (phase from clock)

## Realtime rules

1. **Audio thread:** only `getRawParameterValue` / atomics; advance sources with `Δt = numSamples / sampleRate`; apply modulation with fixed math; no heap, locks, `String`, or XML.  
2. **Destinations:** enum or fixed index — never resolve `parameterID` strings while generating notes.  
3. **UI thread:** APVTS attachments only; optional atomic read of last LFO value for meters.  
4. **State:** APVTS owns enable/rate/depth; source phase is processor-owned runtime (reset in `prepareToPlay`).  
5. **Ordering (MVP):** base velocity → humanize → LFO depth → clamp `[0,1]` → schedule.

## UI surface

**MVP:** three controls in ADVANCED — Enable, Rate (Hz), Depth — labeled as velocity LFO.  
**Later:** dedicated Modulation panel, source meters, drag-route or matrix rows, per-destination indicators (inspired by archive `ModulationPanel` / `ModulatedSlider`, rebuilt against current LookAndFeel).

## MVP this pass

- `Source/Modulation/ModLfo.h` — RT-safe sine LFO  
- `Source/Modulation/ModulationDestination.h` — destination enum stub  
- APVTS: `modLfoEnable`, `modLfoRate`, `modLfoDepth`  
- Apply LFO → velocity in `onSubdivisionHit` / `scheduleNote`  
- Tiny editor controls + Catch2 smoke tests  
- **Not in MVP:** matrix, multiple sources, density modulation, drag-drop, XML matrix serialization

## Next steps

1. Optional second destination: density (same LFO or dedicated depth).  
2. Fixed-slot `ModulationRouter` (compile-time max connections).  
3. Tempo-sync LFO phases via `ClockManager`.  
4. Rebuild a lean Modulation panel (no archive include graft).  
5. Preset migration notes when adding more mod params.
