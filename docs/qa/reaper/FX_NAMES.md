# Known REAPER FX name strings (GenerativeMIDI QA)

`track_fx_add_by_name` / `TrackFX_AddByName` need a string REAPER accepts.
Verified 2026-09-22 via TwelveTake MCP (`user-reaper`).

| Plugin | Working name for `track_fx_add_by_name` | Resolved REAPER display name |
|--------|------------------------------------------|------------------------------|
| Generative MIDI | `Generative MIDI` | `VST3: Generative MIDI (OpenSource)` |
| ReaSynth | `ReaSynth` | `VSTi: ReaSynth (Cockos)` |

Also try if short names fail: `AU: Generative MIDI`, `VST3: Generative MIDI`.

AU validation identity (not an FX browser string): `aumi Gmid Osrc`.
