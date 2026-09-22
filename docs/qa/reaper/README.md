# GenerativeMIDI QA — REAPER transport gate

## Goal

Validate host playhead gating: notes generate while playing; **new** note-ons stop when transport stops.

## Primary path: TwelveTake MCP (agent)

See [`MCP_SETUP.md`](MCP_SETUP.md). With REAPER open and the Lua bridge running:

1. `create_project` (avoid leftover projects).
2. `insert_track` with a clear name (e.g. `GenerativeMIDI transport gate`).
3. `track_fx_add_by_name` with names from [`FX_NAMES.md`](FX_NAMES.md) — `Generative MIDI` then `ReaSynth`.
4. `play` → wait ~3s → `stop` → `get_play_state` (expect stopped).
5. Optionally run [`transport_gate_probe.lua`](transport_gate_probe.lua) for the same play/stop sequence from inside REAPER.

Or load [`create_transport_gate_session.lua`](create_transport_gate_session.lua) once to build the track/FX chain without MCP.

## Fallback: operator GUI

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

- **TwelveTake MCP installed** — `uvx twelvetake-reaper-mcp`; Cursor `~/.cursor/mcp.json` server `reaper` → namespace `user-reaper`.
- Bridge auto-started via `Scripts/__startup.lua` → `reaper_mcp_bridge.lua`.
- Phase C MCP session **PASS**: new project, insert Generative MIDI VST3 + ReaSynth, play ~3.5s, stop (`get_play_state` = 0). Evidence: [`../logs/reaper_transport_gate_mcp.txt`](../logs/reaper_transport_gate_mcp.txt), [`../logs/reaper_transport_gate_mcp.png`](../logs/reaper_transport_gate_mcp.png).
- Working FX names recorded in [`FX_NAMES.md`](FX_NAMES.md).

## How to record evidence

- Note PASS/FAIL in `docs/qa/QA_FINDINGS_v0.8.0.md` Phase C section.
- Attach MCP log + screenshot under `docs/qa/logs/`.

## Future wrappers (not shipped)

Higher-level agent tools such as `run_parameter_sweep` / `compare_plugin_builds` stay out of scope; only transport-gate session + probe helpers exist here.
