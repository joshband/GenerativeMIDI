# Modulation v2

**Status:** Fixed-slot router is live. LFO → velocity and LFO → density stay on the original parameters. Two extra slots route the LFO or a sample-and-hold to gate, pitch, CC, or pitch bend. Envelope and tempo-sync are still deferred.  
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

The editor has a full-width modulation bar under the faceplate: LFO enable, LFO rate, sample-and-hold rate, and four route cells. Routes 1 and 2 are the original `modLfoDepth` and `modLfoDensityDepth` parameters. Routes 3 and 4 are `modRoute3*` and `modRoute4*` (source, destination, amount), defaulting to Off and amount 0 so older sessions stay put. Schema stays 1.2.

## Next steps

1. Tempo-sync LFO and sample-and-hold from `ClockManager`.
2. Note-triggered envelope source.
3. Source meters on the modulation bar.
