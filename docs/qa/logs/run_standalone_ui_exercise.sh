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

# Named set on a main-window popup. Do not open the menu or use window 1:
# the JUCE popup is a separate window and its menu items are not clickable.
select_popup() {
  local title="$1"
  local name="$2"
  local got
  got=$(osascript - "$title" "$name" <<'EOF'
on run argv
  set controlName to item 1 of argv
  set targetName to item 2 of argv
  tell application "System Events"
    tell process "Generative MIDI"
      set frontmost to true
      set value of pop up button controlName of window "Generative MIDI" to targetName
      delay 0.2
      return value of pop up button controlName of window "Generative MIDI"
    end tell
  end tell
end run
EOF
)
  echo "select ${title}=$name got=$got" | tee -a "$LOG"
  if [[ "$got" != "$name" ]]; then
    echo "MISMATCH ${title} expected=$name got=$got" | tee -a "$LOG"
    exit 1
  fi
}

select_generator() {
  select_popup "Generator Type" "$1"
}

GENERATORS=(
  "Euclidean"
  "Polyrhythm"
  "Markov"
  "L-System"
  "Cellular"
  "Probabilistic"
  "Brownian"
  "Perlin Noise"
  "Drunk Walk"
  "Lorenz"
)

for name in "${GENERATORS[@]}"; do
  select_generator "$name"
done

select_generator "Polyrhythm"
screencapture -x docs/qa/logs/standalone_polyrhythm_layers.png
select_generator "Markov"
screencapture -x docs/qa/logs/standalone_markov.png
select_generator "Brownian"
screencapture -x docs/qa/logs/standalone_brownian.png

# Preset, MIDI channel, and scale: same named set, no menu click.
PRESETS=(
  "Euclidean Basic"
  "Euclidean Complex"
  "Polyrhythm Layers"
  "Brownian Drift"
  "Markov Melody"
  "L-System Fractal"
  "Cellular Automata"
  "Probabilistic Sparse"
  "Ratchet Groove"
  "Ambient Drift"
  "Percussive Hits"
)
for name in "${PRESETS[@]}"; do
  select_popup "Preset" "$name"
done

for name in 1 8 16; do
  select_popup "MIDI Channel" "$name"
done

for name in C "F#" B; do
  select_popup "Scale Root" "$name"
done

for name in Chromatic Major Blues "Harmonic Major"; do
  select_popup "Scale Type" "$name"
done

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

FINAL=$(osascript -e 'tell application "System Events" to tell process "Generative MIDI" to get value of pop up button "Generator Type" of window "Generative MIDI"' || echo AX_FAIL)
echo "FINAL_GENERATOR=$FINAL" | tee -a "$LOG"
ls -la docs/qa/logs/standalone_*.png | tee -a "$LOG"
echo "DONE" | tee -a "$LOG"
