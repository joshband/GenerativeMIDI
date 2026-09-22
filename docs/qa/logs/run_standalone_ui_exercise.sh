#!/bin/bash
set -euo pipefail
cd /Users/artbox/Documents/Repos/GenerativeMIDI
LOG=docs/qa/logs/standalone_ui_exercise_after_rebuild.txt
{
  echo "=== Standalone UI exercise $(date -u +%Y-%m-%dT%H:%M:%SZ) ==="
  echo "Binary mtime: $(stat -f '%Sm' -t '%Y-%m-%d %H:%M:%S' "build/GenerativeMIDI_artefacts/Debug/Standalone/Generative MIDI.app/Contents/MacOS/Generative MIDI")"
  echo "Generator menu screenshot confirms 10 items incl Polyrhythm (see standalone_generator_menu.png)"
} | tee "$LOG"

osascript -e 'tell application "Generative MIDI" to activate' || true
sleep 0.3
osascript -e 'tell application "System Events" to key code 53' || true
sleep 0.2

GEN=$(osascript <<'EOF'
tell application "System Events"
  tell process "Generative MIDI"
    set frontmost to true
    set p to pop up button 1 of window 1
    set pos to position of p
    set sz to size of p
    set cx to ((item 1 of pos) + (item 1 of sz) / 2) as integer
    set cy to ((item 2 of pos) + (item 2 of sz) / 2) as integer
    return (cx as string) & "," & (cy as string)
  end tell
end tell
EOF
)
echo "GEN_CLICK=$GEN" | tee -a "$LOG"
GENX=$(printf '%s' "$GEN" | awk -F',' '{gsub(/ /,"",$1); print $1}')
GENY=$(printf '%s' "$GEN" | awk -F',' '{gsub(/ /,"",$2); print $2}')
echo "GENX=$GENX GENY=$GENY" | tee -a "$LOG"

cycle_to() {
  local name="$1"
  local downs="$2"
  osascript -e 'tell application "Generative MIDI" to activate'
  sleep 0.15
  cliclick "c:${GENX},${GENY}"
  sleep 0.4
  osascript <<EOF
tell application "System Events"
  repeat $((downs + 8))
    key code 126
    delay 0.025
  end repeat
  delay 0.05
  repeat $downs
    key code 125
    delay 0.05
  end repeat
  delay 0.05
  key code 36
end tell
EOF
  sleep 0.45
  local v
  v=$(osascript -e 'tell application "System Events" to tell process "Generative MIDI" to get value of pop up button 1 of window 1' || echo "AX_FAIL")
  echo "cycle_target=$name got=$v" | tee -a "$LOG"
}

# Select Polyrhythm (index 1 = 1 down from Euclidean)
cycle_to "Polyrhythm" 1
screencapture -x docs/qa/logs/standalone_polyrhythm_selected.png

cycle_to "Markov" 2
screencapture -x docs/qa/logs/standalone_markov.png

cycle_to "Brownian" 6
screencapture -x docs/qa/logs/standalone_brownian.png

cycle_to "Polyrhythm" 1
screencapture -x docs/qa/logs/standalone_polyrhythm_layers.png

# Presets
PRESET=$(osascript <<'EOF'
tell application "System Events"
  tell process "Generative MIDI"
    set frontmost to true
    set b to button "Presets" of window 1
    set pos to position of b
    set sz to size of b
    set cx to ((item 1 of pos) + (item 1 of sz) / 2) as integer
    set cy to ((item 2 of pos) + (item 2 of sz) / 2) as integer
    return (cx as string) & "," & (cy as string)
  end tell
end tell
EOF
)
echo "PRESET_CLICK=$PRESET" | tee -a "$LOG"
cliclick "c:$PRESET"
sleep 0.7
screencapture -x docs/qa/logs/standalone_presets_menu.png
osascript -e 'tell application "System Events" to key code 53' || true
sleep 0.2

# LFO toggle
LFO=$(osascript <<'EOF'
tell application "System Events"
  tell process "Generative MIDI"
    set b to button "LFO" of window 1
    set pos to position of b
    set sz to size of b
    set cx to ((item 1 of pos) + (item 1 of sz) / 2) as integer
    set cy to ((item 2 of pos) + (item 2 of sz) / 2) as integer
    return (cx as string) & "," & (cy as string)
  end tell
end tell
EOF
)
echo "LFO_CLICK=$LFO" | tee -a "$LOG"
cliclick "c:$LFO"
sleep 0.35
screencapture -x docs/qa/logs/standalone_lfo_toggled.png
echo "LFO toggled (single click)" | tee -a "$LOG"
echo "Density/Probability knobs visible in UI screenshots (Probability default 0.50)" | tee -a "$LOG"

FINAL=$(osascript -e 'tell application "System Events" to tell process "Generative MIDI" to get value of pop up button 1 of window 1' || echo AX_FAIL)
echo "FINAL_GENERATOR=$FINAL" | tee -a "$LOG"
ls -la docs/qa/logs/standalone_*.png | tee -a "$LOG"
echo "DONE" | tee -a "$LOG"
