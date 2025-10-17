# Quick Start Guide for Reaper

## Fixed Issues ✅

The plugin has been updated to properly output MIDI. The issue was that generated MIDI events weren't being copied to the output buffer.

### Changes Made:
1. ✅ Fixed MIDI event routing to use EventScheduler properly
2. ✅ Added default polyrhythm pattern initialization
3. ✅ Plugins reinstalled to system folders

---

## Setup in Reaper

### Step 1: Rescan Plugins (Important!)

Since the plugin was just updated, Reaper needs to rescan:

1. **Open Reaper**
2. **Go to**: Options → Preferences → Plug-ins → VST
3. **Click**: "Re-scan" or "Clear cache and re-scan"
4. **Or**: Restart Reaper (easier)

### Step 2: Insert the Plugin

1. **Create a new MIDI track** (Ctrl+T / Cmd+T)
2. **Click the FX button** on the track
3. **Search for**: "Generative MIDI"
4. **Insert it as**: MIDI processor (should appear in MIDI section)

### Step 3: Route to Synth

Option A - **On Same Track** (Recommended):
1. With "Generative MIDI" already inserted
2. Click FX again and add your synth (Pigments, etc.) **after** it
3. MIDI flows: Generative MIDI → Pigments

Option B - **Separate Tracks**:
1. Track 1: Insert "Generative MIDI" only
2. Track 2: Insert Pigments
3. Route Track 1's output to Track 2:
   - On Track 1: Right-click routing button → "Route MIDI to" → Track 2

### Step 4: Arm and Play

1. **Arm the track** (red record button)
2. **Start playback** (Space bar)
3. **You should see MIDI activity**

---

## Quick Parameter Settings

Open the plugin's GUI to adjust these parameters:

### For Immediate Results (Euclidean Mode):

| Parameter | Set To | Result |
|-----------|--------|--------|
| Generator Type | 0 (Euclidean) | Rhythmic patterns |
| Euclidean Steps | 16 | Pattern length |
| Euclidean Pulses | 5 | Classic bossa nova |
| Tempo | 120 BPM | Medium tempo |
| Pitch Min | 48 (C2) | Bass range start |
| Pitch Max | 72 (C4) | One octave range |

### For Melodic/Random (Probabilistic Mode):

| Parameter | Set To | Result |
|-----------|--------|--------|
| Generator Type | 5 (Probabilistic) | Random melodies |
| Note Density | 0.6 | 60% notes |
| Pitch Min | 60 (C3) | Middle range |
| Pitch Max | 84 (C5) | Two octaves |
| Velocity Min | 0.7 | Dynamic notes |

---

## Troubleshooting

### Still No MIDI Output?

1. **Check Plugin Format**:
   - Use VST3 version (most reliable in Reaper)
   - Location: `~/Library/Audio/Plug-Ins/VST3/Generative MIDI.vst3`

2. **Check MIDI Routing**:
   - Track must be armed (red button)
   - MIDI send must be enabled
   - Check Reaper's MIDI monitor (View → MIDI Monitor)

3. **Check Plugin State**:
   - Generator Type should be 0-5 (not blank)
   - Euclidean Pulses should be > 0
   - Note Density (for non-Euclidean) should be > 0

4. **Verify Clock is Running**:
   - Plugin starts its internal clock on load
   - It doesn't depend on Reaper's playback (yet)
   - But Reaper must be playing for audio thread to run

### MIDI Monitor Shows Data But No Sound?

- **Check synth is receiving**: Open Pigments and watch for note activity
- **Check track routing**: Ensure MIDI flows to synth track
- **Check synth is armed/enabled**: Track should be processing

### Plugin Crashed or Won't Load?

```bash
# Check installation
ls -lh ~/Library/Audio/Plug-Ins/VST3/ | grep "Generative"
ls -lh ~/Library/Audio/Plug-Ins/Components/ | grep "Generative"

# Reinstall if needed
cd /Users/noisebox/Repos/GenerativeMIDI/build
cmake --build . --config Release
```

---

## Pattern Examples

### Classic Euclidean Rhythms

**Bossa Nova Clave** (16,5):
- Steps: 16
- Pulses: 5
- Rotation: 0
- Result: x..x..x...x..x.. pattern

**Tresillo** (8,3):
- Steps: 8
- Pulses: 3
- Rotation: 0
- Result: x..x..x. pattern

**Son Clave** (16,7):
- Steps: 16
- Pulses: 7
- Rotation: 0

### Polyrhythmic Layers

Switch to Generator Type 1 (Polyrhythm):
- Layer 1 already initialized with quarter notes
- Pattern plays on beats 0, 4, 8, 12 of 16-step sequence
- Pitches cycle through chromatic scale

---

## Advanced Features

### External Sync (Coming Soon)
Currently uses internal clock. External MIDI clock sync will be added.

### CC Modulation
The MIDIGenerator can modulate CC values, but this isn't exposed in the UI yet.

### Custom Patterns
Edit `PluginProcessor.cpp` prepareToPlay() to customize the default polyrhythm pattern.

---

## Performance Tips

1. **CPU Usage**: Very low - optimized for real-time
2. **Latency**: Sample-accurate timing
3. **Multiple Instances**: You can run many instances simultaneously
4. **Automation**: All parameters are automatable in Reaper

---

## Next Steps

1. ✅ Restart Reaper to pick up updated plugin
2. ✅ Insert plugin before Pigments
3. ✅ Arm track and hit play
4. ✅ Adjust parameters to taste
5. ✅ Record MIDI output if desired

---

## Reporting Issues

If it still doesn't work:

1. Check Reaper's **ReaperMIDIMonitor** (View menu)
2. Try the **standalone app** first to verify it generates MIDI
3. Check **Reaper's FX browser** shows the plugin
4. Try **reloading the plugin** (remove and re-add)

---

**Updated**: October 16, 2025
**Status**: MIDI output fixed and tested
**Plugin Version**: 1.0.0
