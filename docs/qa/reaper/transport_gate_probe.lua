-- GenerativeMIDI QA: transport gate probe (ReaScript Lua)
-- Usage (REAPER): Actions → Show action list → Load ReaScript → run this file
-- Or: reaper -nonewinst docs/qa/reaper/transport_gate_probe.lua  (if CLI available)
--
-- Expectation (documented in QA findings):
--   With Generative MIDI as MIDI FX on a track:
--   - Play: generative note-ons appear downstream
--   - Stop: no NEW note-ons (hanging notes may release)
-- Standalone (no playhead) always advances — contrast that behavior.

local function log(msg)
  reaper.ShowConsoleMsg(tostring(msg) .. "\n")
end

log("=== GenerativeMIDI transport gate probe ===")
log("1) Insert Generative MIDI (AU or VST3) as MIDI FX before a synth on a MIDI track.")
log("2) Arm/record-arm if needed; open MIDI monitor or use a loud synth.")
log("3) This script will Play for ~3s then Stop.")
log("")

reaper.CSurf_OnPlay()
local play_start = reaper.time_precise()
local function after_play()
  if reaper.time_precise() - play_start < 3.0 then
    reaper.defer(after_play)
    return
  end
  reaper.CSurf_OnStop()
  log("Stopped transport. Confirm: no new generative note-ons while stopped.")
  log("If notes continue while stopped with AU/VST3 loaded, mark FAIL in QA findings.")
  log("=== probe finished ===")
end
reaper.defer(after_play)
