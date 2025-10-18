# GenerativeMIDI Features

Complete feature documentation for the GenerativeMIDI plugin - a cross-platform generative MIDI processor with advanced algorithmic composition capabilities.

---

## Table of Contents

- [Overview](#overview)
- [Generative Engines](#generative-engines)
- [Musical Expression](#musical-expression)
- [MIDI Capabilities](#midi-capabilities)
- [User Interface](#user-interface)
- [Platform Support](#platform-support)
- [Performance](#performance)
- [Use Cases](#use-cases)

---

## Overview

GenerativeMIDI transforms simple parameters into complex, evolving MIDI patterns using mathematical algorithms and music theory. Perfect for:
- Live performance improvisation
- Studio composition and inspiration
- Generative music installations
- Rhythmic and melodic experimentation
- Educational exploration of algorithmic composition

---

## Generative Engines

### 1. Euclidean Rhythm Generator

**Based on:** Björklund's algorithm (2003) - originally used for neutron timing at particle accelerators

**What it does:** Distributes rhythmic pulses as evenly as possible across a number of steps, creating patterns found in traditional music worldwide.

**Parameters:**
- **Steps** (1-64): Total length of the pattern
- **Pulses** (0-64): Number of active beats to distribute
- **Rotation** (0-64): Rotate pattern to change downbeat

**Example Patterns:**
```
Steps: 16, Pulses: 4  → [X···X···X···X···]  (4/4 kick drum)
Steps: 8,  Pulses: 5  → [X·X·X·X·X··]       (Cuban tresillo)
Steps: 12, Pulses: 5  → [X··X·X··X·X·]      (West African bell)
Steps: 16, Pulses: 9  → [X·X·X·X·X·X·X·X]   (Complex polyrhythm)
```

**Musical Applications:**
- Drum patterns (kick, snare, hi-hat)
- Rhythmic bass lines
- Polyrhythmic textures
- Cross-cultural rhythm exploration

---

### 2. Polyrhythm Engine

**What it does:** Layers multiple independent rhythmic patterns with different lengths, creating complex polyrhythmic textures.

**Features:**
- **Multiple Layers**: Stack independent patterns
- **Per-Layer Settings**: Individual length, pitch, velocity per step
- **Phase Relationships**: Patterns cycle at different rates
- **Enable/Disable**: Turn layers on/off independently

**Example:**
```
Layer 1: [X·X·X·X·] (8 steps)  ← Playing notes at C4
Layer 2: [X··X··X··] (9 steps) ← Playing notes at G4
Layer 3: [X···X···] (8 steps)  ← Playing notes at C5

Result: Complex evolving pattern that repeats every 72 steps
```

**Musical Applications:**
- African drumming patterns
- Minimalist composition (Steve Reich style)
- Modular techno sequences
- Polymetric composition

---

### 3. Markov Chain Generator

**What it does:** Uses probability tables to generate melodies where each note influences the probability of the next note.

**How it works:**
- Builds a transition matrix from musical patterns
- Each note has weighted probabilities for what comes next
- Creates coherent but unpredictable melodies

**Characteristics:**
- Medium-term musical memory
- Statistical coherence
- Controllable randomness
- Can learn from existing music

**Musical Applications:**
- Jazz improvisation
- Melodic variation
- Theme and development
- Stochastic composition

---

### 4. L-System Generator

**What it does:** Uses fractal grammar rules (Lindenmayer Systems) to create self-similar, evolving patterns.

**How it works:**
- Starts with a seed pattern (axiom)
- Applies rewrite rules iteratively
- Generates recursive, fractal-like structures

**Example:**
```
Axiom: A
Rule: A → AB, B → A

Generation 0: A
Generation 1: AB
Generation 2: ABA
Generation 3: ABAAB
Generation 4: ABAABABA
```

**Characteristics:**
- Self-similar patterns at different scales
- Organic growth and evolution
- Fibonacci-like sequences
- Fractal structures

**Musical Applications:**
- Evolving melodic lines
- Recursive rhythmic structures
- Algorithmic composition
- Generative ambient music

---

### 5. Cellular Automaton Generator

**What it does:** Uses Wolfram's cellular automata rules to generate patterns from simple initial conditions.

**How it works:**
- Each cell has a state (on/off)
- State evolves based on neighbors using simple rules
- Creates complex patterns from simple rules

**Available Rules:**
- Rule 30: Chaotic behavior
- Rule 90: Fractal triangles (Sierpinski)
- Rule 110: Complex, Turing-complete
- Rule 184: Traffic flow model

**Characteristics:**
- Emergent complexity
- Deterministic but unpredictable
- Pattern evolution over time
- Mathematical beauty

**Musical Applications:**
- Generative rhythms
- Evolving textures
- Algorithmic composition
- Scientific sonification

---

### 6. Probabilistic Generator

**What it does:** Pure random note generation with weighted probabilities and density control.

**Features:**
- Weighted random note selection
- Density control (trigger probability)
- Range constraints (pitch/velocity)
- Statistical distribution

**Characteristics:**
- True randomness
- Controllable chaos
- No pattern memory
- Free improvisation

**Musical Applications:**
- Free jazz
- Ambient textures
- Controlled noise
- Stochastic music

---

### 7. Brownian Motion Generator ✨ NEW (v0.7.0)

**What it does:** Simulates random walk with momentum, creating smooth, wandering melodic contours.

**Algorithm:** Random acceleration with momentum damping and boundary bouncing

**Parameters:**
- **Step Size** (0.01-1.0): Controls randomness magnitude
- **Momentum** (0.0-1.0): Controls inertia/friction in motion
- **Time Scale** (0.01-10.0): Controls evolution speed
- **Density** (0.0-1.0): Note trigger probability

**Characteristics:**
- Smooth, continuous motion
- Natural drift and wander
- Boundary respecting
- Organic feel

**Musical Applications:**
- Ambient melodies
- Smooth bass lines
- Evolving pad sequences
- Natural-feeling arpeggios

---

### 8. Perlin Noise Generator ✨ NEW (v0.7.0)

**What it does:** Uses multi-octave Perlin noise for smooth, terrain-like pitch contours.

**Algorithm:** Classic Perlin noise with fade curves, gradient interpolation, 4 octaves

**Parameters:**
- **Step Size**: Controls terrain smoothness
- **Momentum**: Octave blending
- **Time Scale**: Evolution rate through noise field
- **Density**: Note trigger probability

**Characteristics:**
- Very smooth transitions
- Organic, natural randomness
- No sudden jumps
- Terrain-like contours

**Musical Applications:**
- Evolving textures
- Organic melodies
- Landscape sonification
- Generative ambient

---

### 9. Drunk Walk Generator ✨ NEW (v0.7.0)

**What it does:** Discrete random walk with time-stepped movement and smooth interpolation.

**Algorithm:** Random direction changes with discrete steps

**Parameters:**
- **Step Size**: Size of each discrete step
- **Momentum**: Step timing smoothness
- **Time Scale**: How often direction changes
- **Density**: Note trigger probability

**Characteristics:**
- Stepwise melodic movement
- Random direction changes
- Controllable step size
- Discrete but smooth

**Musical Applications:**
- Random melodic walks
- Jazz-like improvisation
- Stepwise bass lines
- Controlled wandering

---

### 10. Lorenz Attractor Generator ✨ NEW (v0.7.0)

**What it does:** Deterministic chaos using the famous Lorenz attractor (butterfly effect).

**Algorithm:** Numerical integration of Lorenz equations (σ=10, ρ=28, β=8/3)

**Parameters:**
- **Step Size**: Integration step size
- **Momentum**: Damping factor
- **Time Scale**: Evolution speed through attractor
- **Density**: Note trigger probability

**Characteristics:**
- Never repeating patterns
- Deterministic chaos
- Strange attractor behavior
- Butterfly effect sensitivity

**Musical Applications:**
- Chaotic sequences
- Complex polyrhythms
- Never-repeating melodies
- Mathematical music
- Experimental composition

---

## Musical Expression

### Scale Quantization ✨ NEW

**What it does:** Ensures all generated notes conform to a chosen musical scale, eliminating "wrong notes."

**16 Available Scales:**

| Scale Type | Intervals | Musical Character |
|------------|-----------|-------------------|
| **Chromatic** | All 12 notes | No quantization, free chromaticism |
| **Major** | W-W-H-W-W-W-H | Happy, bright, resolved |
| **Minor (Natural)** | W-H-W-W-H-W-W | Sad, dark, melancholic |
| **Harmonic Minor** | W-H-W-W-H-W½-H | Exotic, classical, dramatic |
| **Melodic Minor** | W-H-W-W-W-W-H | Jazz, sophisticated |
| **Dorian** | W-H-W-W-W-H-W | Modal jazz, funk, rock |
| **Phrygian** | H-W-W-W-H-W-W | Spanish, metal, exotic |
| **Lydian** | W-W-W-H-W-W-H | Dreamy, ethereal, film scores |
| **Mixolydian** | W-W-H-W-W-H-W | Blues, rock, folk |
| **Locrian** | H-W-W-H-W-W-W | Diminished, unstable, experimental |
| **Major Pentatonic** | W-W-W½-W-W½ | Pop, rock, country, uplifting |
| **Minor Pentatonic** | W½-W-W-W½-W | Blues, rock, always sounds good |
| **Blues** | W½-W-H-H-W½-W | Blues, rock, gritty |
| **Whole Tone** | W-W-W-W-W-W | Impressionist, dreamlike, Debussy |
| **Diminished** | H-W-H-W-H-W-H-W | Jazz, tension, symmetrical |
| **Harmonic Major** | W-W-H-W-H-W½-H | Exotic, Middle Eastern |

**Parameters:**
- **Scale Root** (C-B): Choose the key center
- **Scale Type**: Select from 16 scales

**Use Cases:**
```
Rock/Blues Jam:
  Root: E, Scale: Minor Pentatonic
  → Always sounds bluesy and cool

Jazz Ballad:
  Root: F, Scale: Melodic Minor
  → Sophisticated, jazzy lines

Ambient Soundscape:
  Root: C, Scale: Whole Tone
  → Ethereal, floating, dreamlike

Heavy Metal Riff:
  Root: D, Scale: Phrygian
  → Dark, aggressive, exotic
```

---

### Swing & Humanization ✨ NEW

**What it does:** Adds groove, timing variations, and human feel to generated patterns.

#### Swing Amount (0-100%)

**Controls:** Timing offset of off-beat notes

**Values:**
- **0%**: Straight timing (quantized)
- **33%**: Light swing (subtle groove)
- **50%**: Triplet feel (classic shuffle)
- **66%**: Hard swing (jazz, blues)
- **75%+**: Extreme shuffle (drunk feel)

**How it works:**
```
Straight (0%):     [X·X·X·X·X·X·X·X·]
Light Swing (33%): [X··X··X··X··X··]
Triplet (50%):     [X···X···X···X···]
Hard Swing (66%):  [X····X····X····X]
```

**Musical Applications:**
- Jazz swing feel
- Hip-hop shuffle
- Drum and bass groove
- House music swing

---

#### Timing Humanization (0-50ms)

**Controls:** Random micro-timing variations per note

**Values:**
- **0ms**: Perfect quantization (robotic)
- **1-3ms**: Tight (professional musician)
- **5-10ms**: Natural (human feel)
- **15-25ms**: Loose (drunk, sloppy)
- **30-50ms**: Very loose (experimental)

**Effect:**
- Prevents mechanical/robotic feel
- Adds organic groove
- Simulates human timing imperfections
- Each note gets random offset (+/-)

**Use Cases:**
```
Electronic Music:  0-2ms   (tight, quantized)
Live Drums:        5-10ms  (natural feel)
Jazz Brush:        10-20ms (loose, swinging)
Drunk Piano:       20-50ms (very loose)
```

---

#### Velocity Humanization (0-100%)

**Controls:** Random velocity variations per note

**Values:**
- **0%**: Fixed velocity (all notes same volume)
- **10-20%**: Subtle variation (controlled dynamics)
- **30-50%**: Natural variation (human playing)
- **60-100%**: Extreme variation (wild dynamics)

**How it works:**
- Adds random variation to each note's velocity
- Stays within your min/max velocity range
- Creates dynamic expression automatically

**Musical Applications:**
- Acoustic instrument realism
- Dynamic expression
- Ghost notes and accents
- Live performance feel

---

### Combined Expression Examples

**Funky Bass Line:**
```
Generator: Euclidean (Steps: 16, Pulses: 7)
Scale: E Minor Pentatonic
Swing: 40%
Timing: 8ms
Velocity Var: 25%
Result: Groovy, funky, human-feeling bass
```

**Jazz Piano Comping:**
```
Generator: Markov Chain
Scale: Bb Dorian
Swing: 55%
Timing: 12ms
Velocity Var: 35%
Result: Swinging jazz chords with human feel
```

**Electronic Techno:**
```
Generator: Euclidean (Steps: 16, Pulses: 4)
Scale: Chromatic
Swing: 0%
Timing: 0ms
Velocity Var: 5%
Result: Tight, robotic, machine-perfect
```

**Drunk Piano:**
```
Generator: Probabilistic
Scale: C Major
Swing: 20%
Timing: 30ms
Velocity Var: 60%
Result: Loose, sloppy, experimental jazz
```

---

## MIDI Capabilities

### Note Generation

- **Polyphonic output**: Multiple simultaneous notes
- **Pitch range**: Full MIDI range (0-127)
- **Velocity control**: Min/Max range with humanization
- **Note duration**: Controllable gate length (1-200%)
- **Legato mode**: Overlapping notes with no gaps
- **Ratcheting**: Probability-based note retriggering (1-16 repeats)
- **MIDI channels** ✨ NEW (v0.8.0): Route to any MIDI channel (1-16)

### Expression & Control

- **Velocity**: Dynamic range control with humanization
- **Gate Length**: 1-200% of step duration
- **Ratchet Decay**: Exponential velocity falloff for repeats
- **Polyphonic Aftertouch**: Per-note pressure (available via MIDIGenerator)
- **Control Change (CC)**: Parameter automation (available via MIDIGenerator)
- **Pitch Bend**: Micro-tonal control (available via MIDIGenerator)

### Timing

- **Sample-accurate**: Sub-millisecond precision
- **Internal clock**: 20-400 BPM
- **External sync**: MIDI clock input
- **Time signatures**: 1/1 to 16/16
- **Swing**: Adjustable groove timing

---

## User Interface

### Design Philosophy

**Gilded Steampunk Victorian Aesthetic:**
- Brass, gold, and copper color scheme
- Obsidian glass displays with aether crystal indicators
- Polished brass bezels and filigree
- Victorian instrument panel design
- Gothic industrial aesthetic

### Layout Sections

#### 1. PATTERN DISPLAY ✨ ENHANCED (v0.8.0)
- Real-time visualization of active pattern
- **Live playback position** with amber crystal effect
- Cyan aether crystal highlights for active steps
- Bronze "dormant" indicators for probability misses
- Step numbers on 4-beat intervals
- Brass chamber outlines

#### 2. GENERATOR ✨ ENHANCED (v0.8.0)
- **Generator Type**: Dropdown selector (10 types)
- **Color-coded label**: Changes color by engine type
  - Gold: Euclidean
  - Copper: Polyrhythm
  - Green Verdigris: Algorithmic
  - Violet: Stochastic/Chaos
- **MIDI Channel**: Channel selector (1-16)
- **Tempo**: Master BPM control (20-400)

#### 3. EUCLIDEAN CONTROLS (context-aware)
- **Steps**: Pattern length (1-64)
- **Pulses**: Active beats (0-64)
- **Rotation**: Pattern offset (0-64)
- Auto-disables when not using Euclidean generator

#### 4. STOCHASTIC CONTROLS ✨ NEW (v0.8.0)
- **Step Size**: Randomness magnitude (0.01-1.0)
- **Momentum**: Inertia/friction (0.0-1.0)
- **Time Scale**: Evolution speed (0.01-10.0)
- Auto-enables only for stochastic generators

#### 5. EXPRESSION
- **Velocity Range**: Min/Max sliders
- **Pitch Range**: Min/Max sliders (MIDI 0-127)
- **Scale**: Root and Type dropdowns (16 scales)
- **Swing**: Groove timing knob (6 templates)
- **Timing**: Humanization knob (0-50ms)
- **Vel Var**: Velocity randomness knob (0-100%)

#### 6. ARTICULATION
- **Gate Length**: Note duration (1-200%)
- **Legato**: Overlapping notes toggle
- **Ratchet**: Number of repeats (1-16)
- **R Prob**: Ratchet probability (0-100%)
- **R Decay**: Velocity decay (0-100%)

### Control Types

- **Brass Rotary Knobs**: Gilded brass with aether crystal centers
- **Vertical Sliders**: Art Deco brass rails
- **Dropdown Menus**: Brass Victorian selectors
- **Text Displays**: Engraved brass numerals

### Dynamic UI ✨ NEW (v0.7.1 & v0.8.0)

- **Context-Aware Controls**: Auto-enable/disable based on generator type
- **Visual Feedback**: Disabled controls grey to 30% opacity
- **Color Coding**: Generator label matches engine category
- **Real-time Updates**: UI responds instantly to parameter changes

### Responsiveness

- **Resizable**: 1200x500 to 2000x1000 pixels
- **Touch-optimized**: Works great on iPad/touchscreen
- **30Hz refresh**: Smooth visual updates and playback visualization
- **Real-time feedback**: Immediate parameter response
- **31 parameters**: Full automation support

---

## Platform Support

### Desktop (macOS)

✅ **Audio Unit (AU)**
- Native macOS format
- Tested with Logic Pro, GarageBand, Ableton Live
- Full parameter automation
- State saving/recall

✅ **VST3**
- Cross-DAW compatibility
- Tested with Ableton Live, Bitwig, Reaper
- Full automation and preset support

✅ **Standalone Application**
- Use without a DAW
- MIDI output to external synths
- Perfect for live performance

### Mobile (iOS/iPadOS)

✅ **AUv3 (Audio Unit v3)**
- Native iOS/iPadOS format
- Works in GarageBand, AUM, Cubasis, Beatmaker 3
- Full touch support
- Background audio support
- Inter-app audio compatibility

**System Requirements:**
- iOS 13.0 or later
- iPadOS 13.0 or later
- iPhone 8 or newer recommended
- iPad Pro for best experience

### Future Platforms

📋 **Windows**
- VST3 support planned
- Standalone application

📋 **Linux**
- VST3 and standalone builds
- Community-requested feature

---

## Performance

### CPU Usage

- **Typical**: < 5% on modern hardware
- **Audio thread**: Minimal processing
- **UI thread**: 30Hz refresh (negligible)
- **Background**: Near-zero when idle

### Memory

- **RAM**: ~10MB
- **Disk**: < 20MB installed
- **No streaming**: All processing in memory
- **No allocations**: During playback (real-time safe)

### Latency

- **Processing**: < 1ms
- **Sample-accurate**: Sub-millisecond timing precision
- **Buffer-agnostic**: Works with 32-2048 sample buffers
- **Zero-latency**: MIDI processing

### Sample Rate Support

- ✅ 44.1 kHz
- ✅ 48 kHz
- ✅ 88.2 kHz
- ✅ 96 kHz
- ✅ Higher rates supported

---

## Use Cases

### Live Performance

**Electronic Music:**
```
- Create evolving drum patterns
- Generate bass lines in real-time
- Trigger chord progressions
- Add melodic variations on the fly
```

**Band/Ensemble:**
```
- Send patterns to external synths
- Control hardware drum machines
- Generate background textures
- Provide rhythmic click tracks
```

### Studio Production

**Composition:**
```
- Generate initial ideas and sketches
- Create variation on themes
- Explore harmonic possibilities
- Build rhythmic foundations
```

**Arrangement:**
```
- Layer multiple generators for texture
- Create evolving backgrounds
- Add generative percussion
- Build transitional sections
```

### Education

**Learning:**
```
- Explore music theory (scales, modes)
- Understand polyrhythms
- Study algorithmic composition
- Experiment with fractals in music
```

**Teaching:**
```
- Demonstrate Euclidean rhythms
- Show scale construction
- Explain Markov processes
- Visualize cellular automata
```

### Installations

**Generative Art:**
```
- Create never-repeating music
- Sonify data and algorithms
- Build interactive exhibits
- Generate ambient soundscapes
```

**Background Music:**
```
- Evolving lobby/waiting room music
- Gallery installations
- Retail environments
- Meditation/wellness centers
```

---

## Workflow Examples

### Example 1: Minimal Techno Beat

```
1. Set Generator: Euclidean
2. Steps: 16, Pulses: 4 (kick pattern)
3. Pitch: Set to single note (C1)
4. Scale: Chromatic (no quantization needed)
5. Swing: 0% (straight techno)
6. Timing: 0ms (machine perfect)
7. Result: Tight, repetitive kick pattern

Add Layer:
8. Open second instance
9. Steps: 16, Pulses: 6 (hi-hat)
10. Pitch: High note (G4)
11. Result: Polyrhythmic techno groove
```

### Example 2: Jazz Improvisation

```
1. Set Generator: Markov Chain
2. Scale: Bb Dorian (jazz mode)
3. Pitch Range: Bb3-Bb5 (2 octaves)
4. Swing: 55% (jazz swing)
5. Timing: 10ms (human feel)
6. Velocity Var: 30% (dynamic)
7. Probability: 70% (some space)
8. Result: Swinging jazz solo over changes
```

### Example 3: Ambient Soundscape

```
1. Set Generator: L-System
2. Scale: C Whole Tone (ethereal)
3. Pitch Range: Full range
4. Swing: 0%
5. Timing: 20ms (floating)
6. Velocity Var: 40% (organic)
7. Probability: 40% (sparse)
8. Tempo: 60 BPM (slow)
9. Result: Evolving ambient textures
```

### Example 4: African Drumming

```
1. Set Generator: Euclidean
2. Steps: 12, Pulses: 5 (bell pattern)
3. Scale: Chromatic
4. Pitch: Single note (percussion)
5. Swing: 0% (straight)
6. Timing: 5ms (slight human feel)
7. Velocity Var: 15% (natural accents)

Layer additional Euclidean patterns:
- 16 steps, 7 pulses (low drum)
- 8 steps, 3 pulses (high drum)

Result: Complex West African polyrhythm
```

---

## Tips & Tricks

### Getting Started

1. **Start Simple**: Use Euclidean generator first
2. **One Parameter**: Change one thing at a time
3. **Listen First**: Before adding effects/processing
4. **Save Presets**: When you find something good

### Creative Techniques

**Morphing Patterns:**
- Slowly change Steps parameter while playing
- Transition between scales during performance
- Automate Swing amount for evolving groove

**Layering:**
- Run multiple instances in parallel
- Different generators on different MIDI channels
- Combine rhythmic and melodic generators

**Processing:**
- Send to arpeggiator for doubling
- Add MIDI delay for echoes
- Use MIDI chord plugin for harmony

**Experimentation:**
- Use L-System with Whole Tone scale
- Cellular Automaton with Blues scale
- Markov Chain with high Timing humanization

### Troubleshooting

**Too Chaotic:**
- Reduce Probability/Density
- Use simpler scales (Pentatonic)
- Lower Timing/Velocity humanization

**Too Boring:**
- Increase pattern complexity (more Pulses)
- Add Swing and humanization
- Use more exotic scales (Phrygian, Locrian)
- Switch to algorithmic generator

**Wrong Notes:**
- Check Scale selection
- Verify Pitch Range is within scale
- Try simpler scale (Major/Minor Pentatonic)

---

## Technical Specifications

### Audio
- **Format**: MIDI only (no audio processing)
- **Channels**: 16 MIDI channels
- **Polyphony**: Unlimited (MIDI messages)
- **Latency**: < 1ms processing time

### Processing
- **Thread-safe**: Real-time audio thread safe
- **Lock-free**: No mutex in audio callback
- **Sample-accurate**: Event scheduling to sample
- **Deterministic**: Reproducible patterns

### State Management
- **Parameters**: Full automation support
- **Presets**: Save/load via DAW
- **Session**: Restores exact state
- **Undo/Redo**: Via DAW automation

---

## Credits & References

### Algorithms
- **Euclidean Rhythms**: Godfried Toussaint (2005)
- **Björklund's Algorithm**: E. Björklund (2003)
- **L-Systems**: Aristid Lindenmayer (1968)
- **Cellular Automata**: Stephen Wolfram (1983)
- **Markov Chains**: Andrey Markov (1906)

### Framework
- **JUCE**: Cross-platform audio framework
- **CMake**: Build system
- **C++17**: Modern C++ implementation

### Inspiration
- **Brian Eno**: Generative music pioneer
- **Steve Reich**: Minimalist phase music
- **Aphex Twin**: Algorithmic composition
- **Autechre**: Generative systems

---

## Version History

### v1.1.0 (Current)
- ✅ Scale Quantization (16 scales)
- ✅ Swing & Humanization
- ✅ Updated UI with new controls
- ✅ Enhanced Expression section

### v1.0.0 (Initial Release)
- ✅ Euclidean rhythm generator
- ✅ Polyrhythm engine
- ✅ Markov Chain generator
- ✅ L-System generator
- ✅ Cellular Automaton generator
- ✅ Probabilistic generator
- ✅ Cross-platform support
- ✅ Custom vintage UI

---

## License

MIT License - Free and open source

See [LICENSE](LICENSE) for full details.

---

## Support & Community

- 📖 **Documentation**: This file and [README.md](README.md)
- 🎯 **Roadmap**: See [ENHANCEMENTS.md](ENHANCEMENTS.md)
- 🐛 **Bug Reports**: GitHub Issues
- 💬 **Discussions**: GitHub Discussions
- 📧 **Contact**: Via GitHub

---

**Made with ❤️ for musicians, coders, and explorers of generative music**

*Last Updated: 2025-10-17 (v1.1.0)*
