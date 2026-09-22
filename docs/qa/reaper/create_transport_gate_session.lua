-- GenerativeMIDI QA: create a minimal transport-gate session
-- Usage: Actions → Load ReaScript → run this file (with Generative MIDI installed).
-- Or invoke via TwelveTake MCP after the bridge is running (agent can mirror these steps).
--
-- Inserts one track with Generative MIDI (MIDI FX) then ReaSynth for audible monitor.
-- Then run transport_gate_probe.lua (or MCP play/stop) to verify host gating.
--
-- Exact FX name strings: see FX_NAMES.md (update after first successful add).

local function log(msg)
  reaper.ShowConsoleMsg(tostring(msg) .. "\n")
end

-- Candidate names tried in order (TrackFX_AddByName returns >= 0 on success).
local GEN_MIDI_NAMES = {
  "Generative MIDI",
  "AU: Generative MIDI",
  "VST3: Generative MIDI",
  "AU: Generative MIDI (OpenSource)",
  "VST3: Generative MIDI (OpenSource)",
}

local SYNTH_NAMES = {
  "ReaSynth",
  "VSTi: ReaSynth (Cockos)",
  "JS: ReaSynth",
}

local function add_fx_first_match(track, names, insert_pos)
  for _, name in ipairs(names) do
    local idx = reaper.TrackFX_AddByName(track, name, false, insert_pos)
    if idx ~= nil and idx >= 0 then
      return idx, name
    end
  end
  return -1, nil
end

reaper.Undo_BeginBlock()

local track_idx = reaper.GetNumTracks()
reaper.InsertTrackAtIndex(track_idx, true)
local track = reaper.GetTrack(0, track_idx)
reaper.GetSetMediaTrackInfo_String(track, "P_NAME", "GenerativeMIDI transport gate", true)

local gen_idx, gen_name = add_fx_first_match(track, GEN_MIDI_NAMES, -1)
if gen_idx < 0 then
  log("FAIL: could not add Generative MIDI. Update FX_NAMES.md with REAPER FX browser string.")
  reaper.Undo_EndBlock("GenerativeMIDI create_transport_gate_session (failed)", -1)
  return
end
log("OK: added Generative MIDI as [" .. gen_name .. "] at FX index " .. tostring(gen_idx))

local syn_idx, syn_name = add_fx_first_match(track, SYNTH_NAMES, -1)
if syn_idx < 0 then
  log("WARN: Generative MIDI added but ReaSynth not found. Add any soft synth after it and continue.")
else
  log("OK: added monitor synth as [" .. syn_name .. "] at FX index " .. tostring(syn_idx))
end

reaper.Undo_EndBlock("GenerativeMIDI create_transport_gate_session", -1)
log("Session ready. Run transport_gate_probe.lua or MCP play(~3s)/stop.")
log("=== create_transport_gate_session finished ===")
