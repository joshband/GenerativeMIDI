# TwelveTake REAPER MCP setup (GenerativeMIDI QA)

Agent-driven Host QA uses [TwelveTake REAPER MCP](https://github.com/TwelveTake-Studios/reaper-mcp) (`twelvetake-reaper-mcp`) over a Lua file-IPC bridge. Prefer this path over GUI automation.

## One-time install

```bash
uvx twelvetake-reaper-mcp --install-bridge
```

This copies `reaper_mcp_bridge.lua` to:

`~/Library/Application Support/REAPER/Scripts/reaper_mcp_bridge.lua`

Bridge data dir (created when the script runs):

`~/Library/Application Support/REAPER/Scripts/mcp_bridge_data/`

## Cursor MCP config (user-local, not in-repo)

`~/.cursor/mcp.json`:

```json
{
  "mcpServers": {
    "reaper": {
      "command": "uvx",
      "args": ["twelvetake-reaper-mcp"]
    }
  }
}
```

Reload MCP / restart Cursor after editing. The namespace appears as `user-reaper` (or `reaper`).

## Every REAPER session

1. Open REAPER (or restart it).
2. Prefer **auto-start**: this machine uses `~/Library/Application Support/REAPER/Scripts/__startup.lua` to run `reaper_mcp_bridge.lua` on launch (GenerativeMIDI QA).
3. Manual fallback: **Actions → Show action list → Load ReaScript** → select `reaper_mcp_bridge.lua` → **Run**.
4. Leave the script running (defer loop).
5. Confirm MCP tools respond (e.g. `get_track_count`).

The MCP server cannot start the Lua bridge by itself if `__startup.lua` is missing. If tools time out, re-run the bridge script.

## GenerativeMIDI QA names

| Role | Typical REAPER FX name to try |
|------|-------------------------------|
| MIDI FX | `Generative MIDI`, `AU: Generative MIDI`, `VST3: Generative MIDI` |
| Monitor synth | `ReaSynth` |
| AU identity (auval) | `aumi Gmid Osrc` |

Store the exact string that `track_fx_add_by_name` accepts in [`FX_NAMES.md`](FX_NAMES.md) after the first successful insert.

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| MCP timeout / no response | Bridge not running — Load ReaScript and Run |
| FX add returns -1 / not found | Wrong name — open FX browser, copy exact string into `FX_NAMES.md` |
| Wrong project open | File → New Project (or MCP project tools) before QA |
| After package upgrade | Re-run `uvx twelvetake-reaper-mcp --install-bridge` and re-run the Lua script |

## Related

- [`README.md`](README.md) — transport-gate procedure
- [`create_transport_gate_session.lua`](create_transport_gate_session.lua)
- [`transport_gate_probe.lua`](transport_gate_probe.lua)
