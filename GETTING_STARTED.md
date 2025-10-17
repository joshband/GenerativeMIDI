# Getting Started with Generative MIDI Processor

This guide will help you get up and running with the Generative MIDI Processor quickly.

## Installation

### Option 1: Pre-built Binaries (When Available)
1. Download the appropriate binary for your platform
2. Copy the plugin to your system's plugin folder:
   - **macOS AU**: `~/Library/Audio/Plug-Ins/Components/`
   - **macOS VST3**: `~/Library/Audio/Plug-Ins/VST3/`
   - **Windows VST3**: `C:\Program Files\Common Files\VST3\`
   - **Linux VST3**: `~/.vst3/`

### Option 2: Build from Source
See the main [README.md](README.md) for detailed build instructions.

## First Steps

### 1. Load the Plugin
- Open your DAW (Ableton Live, Logic Pro, FL Studio, Reaper, etc.)
- Create a new MIDI track
- Insert "Generative MIDI" as a MIDI effect (before any instrument)
- Route the output to a synthesizer or sampler

### 2. Choose Your Generator

The plugin offers 6 generator types:

| Generator | Best For | Use Case |
|-----------|----------|----------|
| **Euclidean** | Rhythmic patterns | Drums, percussion, driving basslines |
| **Polyrhythm** | Complex rhythms | Layered patterns, polymetric music |
| **Markov** | Melodic evolution | Melodies that follow learned patterns |
| **L-System** | Fractal patterns | Evolving sequences, generative composition |
| **Cellular** | Emergent patterns | Experimental, evolving textures |
| **Probabilistic** | Random variation | Controlled randomness, improvisation |

### 3. Basic Configuration

#### For Euclidean Rhythms:
```
Steps: 16       (Length of the pattern)
Pulses: 5       (Number of hits)
Rotation: 0     (Start position)
Tempo: 120 BPM
```

**Try these classic Euclidean patterns:**
- **(16, 5)** - Bossa Nova clave
- **(8, 3)** - Tresillo pattern
- **(12, 5)** - African bell pattern
- **(16, 9)** - Aksak rhythm

#### For Polyrhythmic Patterns:
1. Start with the default layer (4:4)
2. Add a second layer with different division (3:4)
3. Adjust phase offset to create interesting interactions

#### For Algorithmic Generators:
```
Note Density: 0.5    (50% chance of notes)
Pitch Min: 48        (C2)
Pitch Max: 84        (C5)
Velocity Min: 0.5
Velocity Max: 1.0
```

## Quick Examples

### Example 1: Euclidean Drum Pattern

**Goal**: Create a drum pattern generator

1. **Load the plugin** on a MIDI track
2. **Route to drum sampler** (e.g., kick on C1, snare on D1)
3. **Set parameters:**
   - Generator: Euclidean
   - Steps: 16
   - Pulses: 4
   - Pitch Min: 36 (C1 - Kick)
   - Pitch Max: 36
4. **Duplicate track** for snare (D1 = 38)
5. **Adjust pulses** on snare track: 7 pulses

**Result**: Interlocking kick and snare patterns!

### Example 2: Generative Bassline

**Goal**: Create an evolving bassline

1. **Load plugin** before a bass synthesizer
2. **Set parameters:**
   - Generator: Markov or Probabilistic
   - Note Density: 0.6
   - Pitch Min: 28 (E1)
   - Pitch Max: 48 (C3)
   - Tempo: 128 BPM
3. **Let it run** - the pattern will evolve naturally

### Example 3: Polyrhythmic Melody

**Goal**: Create a complex rhythmic melody

1. **Set Generator** to Polyrhythm
2. **Add 3 layers** with different divisions:
   - Layer 1: 4 (quarter notes)
   - Layer 2: 6 (eighth note triplets)
   - Layer 3: 16 (sixteenth notes)
3. **Set different pitches** for each layer
4. **Adjust phase** for each layer to offset them

## Understanding the Controls

### Tempo and Timing
- **Tempo**: Sets the internal clock speed (20-400 BPM)
- **Time Signature**: Controls the bar length and subdivision behavior
- **External Sync**: Enable to sync with your DAW's tempo

### Generator-Specific Parameters

#### Euclidean Parameters
- **Steps**: How many positions in the pattern
- **Pulses**: How many active hits
- **Rotation**: Shifts the pattern left/right

#### Density and Range
- **Note Density**: Probability of generating notes (0.0 = none, 1.0 = always)
- **Pitch Range**: Constrains generated notes to a specific range
- **Velocity Range**: Sets the dynamic range of generated notes

### Expression Controls

#### Velocity Curve
- **Linear**: Even velocity distribution
- **Exponential**: More loud notes
- **Logarithmic**: More quiet notes

#### CC Modulation
Enable automated control change modulation:
- **CC Number**: Which controller to modulate (1 = Mod Wheel, 74 = Filter Cutoff)
- **Rate**: LFO speed in Hz
- **Depth**: Amount of modulation
- **Shape**: Sine, Triangle, Square, Saw, Random

## Tips and Tricks

### 1. Layering Generators
- Use multiple instances on separate tracks
- Route to different sounds
- Create complex polyrhythmic textures

### 2. Constraining to Scales
- Set pitch range to a specific octave
- Use pitch quantizer after the plugin
- Or modify the code to add scale constraints

### 3. Creating Variations
- Automate the **Rotation** parameter for Euclidean patterns
- Modulate **Note Density** for dynamic intensity changes
- Change **Generator Type** between sections

### 4. Humanization
- Adjust **Velocity Range** (e.g., 0.6-0.9 instead of 0.5-1.0)
- Add subtle timing variations with your DAW
- Use velocity curve to shape dynamics

### 5. Syncing Multiple Instances
- All instances share the same tempo
- Use **Phase** parameter in polyrhythm mode
- Create polymetric relationships (3 against 4, etc.)

## Common Workflows

### Workflow 1: Generative Performance
1. Set up multiple generator instances
2. Map parameters to MIDI controllers
3. Perform by changing generator types and parameters in real-time

### Workflow 2: Pattern Capture
1. Let generator run and capture patterns you like
2. Record MIDI output from the plugin
3. Edit and arrange the captured patterns

### Workflow 3: Hybrid Approach
1. Use generator for rhythm only
2. Play melody on top with MIDI keyboard
3. Or vice versa - generated melody, played rhythm

## Troubleshooting

### No MIDI Output
- Check that your DAW recognizes MIDI effects
- Ensure the track is armed/enabled
- Verify tempo is running (not stopped)

### Pattern Not Repeating
- For Euclidean: Check steps and pulses values
- For algorithmic: This is normal - patterns evolve
- Use Euclidean or Polyrhythm for repeating patterns

### Too Many/Few Notes
- Adjust **Note Density** parameter
- For Euclidean: Reduce **Pulses** value
- For Polyrhythm: Adjust pattern per layer

### Notes Outside Expected Range
- Set **Pitch Min** and **Pitch Max** to constrain range
- Check that your synth is set to the correct octave

## Next Steps

1. **Experiment** with different generator combinations
2. **Read the full documentation** for advanced features
3. **Check the examples** folder for preset files
4. **Join the community** to share your creations

## Resources

- [Full README](README.md) - Complete documentation
- [API Reference](API.md) - For developers
- [Examples](examples/) - Preset files and usage examples

---

**Have fun creating generative music!**
