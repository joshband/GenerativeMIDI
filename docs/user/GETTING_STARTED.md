# Getting Started with Generative MIDI Processor

This guide will help you get up and running with the Generative MIDI Processor quickly.

## Installation

### Option 1: Pre-built Binaries (When Available)
1. Download the appropriate binary for your platform
2. Copy the plugin to your system's plugin folder:
   - **macOS AU**: `~/Library/Audio/Plug-Ins/Components/`
   - **macOS VST3**: `~/Library/Audio/Plug-Ins/VST3/`
   - **Windows VST3**: `C:\Program Files\Common Files\VST3\` (planned)
   - **Linux VST3**: `~/.vst3/` (planned)

### Option 2: Build from Source
See the main [README.md](../../README.md) and [BUILD.md](../developer/BUILD.md). Canonical build is **CMake**; initialize the `art/` submodule with `git submodule update --init --recursive`.

## First Steps

### 1. Load the Plugin
- Open your DAW (Ableton Live, Logic Pro, FL Studio, Reaper, etc.)
- Create a new MIDI track
- Insert "Generative MIDI" as a MIDI effect (before any instrument)
- Route the output to a synthesizer or sampler
- Start host transport (when a playhead is available, generation follows play/stop)

### 2. Choose Your Generator

The editor exposes **9** generators (Polyrhythm is deferred — engine retained for a later branch):

| Generator | Best For | Use Case |
|-----------|----------|----------|
| **Euclidean** | Rhythmic patterns | Drums, percussion, driving basslines |
| **Markov** | Melodic evolution | Melodies that follow learned patterns |
| **L-System** | Fractal patterns | Evolving sequences, generative composition |
| **Cellular** | Emergent patterns | Experimental, evolving textures |
| **Probabilistic** | Random variation | Controlled randomness, improvisation |
| **Brownian** | Smooth random walk | Organic pitch drift |
| **Perlin Noise** | Natural randomness | Soft, continuous variation |
| **Drunk Walk** | Discrete steps | Jagged random melodies |
| **Lorenz** | Deterministic chaos | Unpredictable but structured lines |

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

#### For Algorithmic Generators:
```
Note Density / Probability: 0.5
Pitch Min: 48        (C2)
Pitch Max: 84        (C5)
Velocity Min / Max: as desired
Scale + Root: constrain pitch
```

#### For Stochastic Generators:
```
Step Size, Momentum, Time Scale: start mid-range and audition
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

### Example 3: Chaos Lead

**Goal**: Unpredictable melodic line

1. **Set Generator** to Lorenz (or Brownian)
2. Constrain **Scale** / pitch range
3. Raise **Probability** until density feels right
4. Layer a second instance on another MIDI channel for contrast

## Understanding the Controls

### Tempo and Timing
- **Tempo**: Internal clock speed (20–400 BPM)
- Prefer host transport play/stop when available

### Generator-Specific Parameters

#### Euclidean
- **Steps**: Pattern length
- **Pulses**: Active hits
- **Rotation**: Pattern shift

#### Density and Range
- **Probability / Density**: Chance of generating notes
- **Pitch / Velocity Range**: Output constraints
- **Scale / Root**: Quantization

### Expression / articulation (shipped)
- Gate length, legato, ratcheting
- Swing + timing / velocity humanization
- MIDI channel 1–16

MIDI expression (aftertouch / CC / pitch bend) is in the EXPRESSION section — enable each and set amount/range; emitted on note-on (not MPE). Velocity always works.

## Tips and Tricks

### 1. Layering Generators
- Use multiple instances on separate tracks / MIDI channels
- Route to different sounds

### 2. Constraining to Scales
- Use built-in Scale Root / Type controls
- Tighten Pitch Min / Max to an octave

### 3. Creating Variations
- Automate **Rotation** for Euclidean patterns
- Modulate **Probability** for intensity
- Change generator type between sections

## Troubleshooting

### No MIDI Output
- Check that your DAW recognizes MIDI effects
- Ensure the track is armed/enabled
- Verify host transport is playing (when playhead is present)

### Pattern Not Repeating
- For Euclidean: Check steps and pulses
- For algorithmic / stochastic: evolving output is expected

### Too Many/Few Notes
- Adjust **Probability**
- For Euclidean: change **Pulses**

## Next Steps

1. Experiment with the nine UI generators
2. Read [FEATURES.md](FEATURES.md) for the full reference
3. See the [showcase](https://joshband.github.io/GenerativeMIDI/) for architecture / evidence

---

**Have fun creating generative music!**
