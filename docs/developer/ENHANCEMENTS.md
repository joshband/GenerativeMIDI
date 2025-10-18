# Generative MIDI - Enhancement Ideas

This document tracks potential enhancements and feature ideas for the GenerativeMIDI plugin.

## Status Legend
- ✅ **Implemented**
- 🚧 **In Progress**
- 📋 **Planned**
- 💡 **Proposed**

## Version History

- **v0.8.0** (2025-10-18) - MIDI Channel Routing & Enhanced UI
  - MIDI channel routing (1-16 channels)
  - Stochastic engine UI controls (Step Size, Momentum, Time Scale)
  - Color-coded generator labels
  - Live playback position visualization
  - Enhanced dynamic UI with context-aware controls
- **v0.7.1** (2025-10-18) - Dynamic UI Updates
  - Context-aware control enabling/disabling
  - Visual feedback with opacity changes
  - Dynamic label updates
- **v0.7.0** (2025-10-18) - Stochastic & Chaos Generators
  - 4 new generators: Brownian Motion, Perlin Noise, Drunk Walk, Lorenz Attractor
  - Full integration with all existing features
- **v0.6.0** (2025-10-18) - Preset Management System
  - Complete save/load functionality
  - Preset browser with category filtering
- **v0.5.0** (2025-10-18) - Ratcheting & Gate Length
  - Probability-based note retriggering
  - Gate length control with legato mode
- **v0.3.0** (2025-01-17) - Gilded Steampunk UI redesign
  - Victorian brass/gold/copper aesthetic
  - Aether crystal pattern visualizer
- **v0.2.0** (2025-01-17) - Scale Quantization & Humanization
  - 16 musical scales
  - Swing engine with groove templates
- **v0.1.0** - Initial release with 6 generator types

---

## Engine Enhancements

### 1. Stochastic/Chaos Modes ✅

Advanced randomization and chaos theory-based generators for organic, evolving patterns.

**Features:**

- ✅ **Brownian Motion Generator** - Random walk with momentum and boundaries (IMPLEMENTED v0.7.0)
- ✅ **Perlin Noise** - Multi-octave smooth organic randomness (IMPLEMENTED v0.7.0)
- ✅ **Drunk Walk** - Step-based random walks with configurable step size (IMPLEMENTED v0.7.0)
- ✅ **Lorenz Attractor** - Deterministic chaos mapped to pitch space (IMPLEMENTED v0.7.0)
- 📋 **Rössler Attractor** - Alternative chaos attractor with different topology
- 📋 **Simplex Noise** - Improved Perlin noise with better isotropy
- ✅ **UI Controls** - Step Size, Momentum, Time Scale parameters (IMPLEMENTED v0.8.0)

**Benefits:** Creates natural-sounding, non-repetitive patterns that evolve organically

**Implementation Complexity:** Medium (COMPLETED)
**Musical Impact:** High

---

### 2. Musical Theory Integration ✅
Constrain and guide generated output using music theory principles.

**Features:**
- ✅ **Scale/Mode Quantization** - Constrain pitch output to musical scales (IMPLEMENTED)
  - Major, Minor (Natural, Harmonic, Melodic)
  - Modes: Dorian, Phrygian, Lydian, Mixolydian, Aeolian, Locrian
  - Pentatonic (Major, Minor)
  - Blues scales
  - Chromatic
  - Custom scale definition
- 📋 **Chord Progression Engine** - Generate harmonic progressions using common cadences
  - I-IV-V-I, ii-V-I, etc.
  - Circle of fifths navigation
  - Jazz voicings
- 📋 **Arpeggiator Mode** - Convert chords into arpeggiated patterns
  - Up, Down, Up-Down, Random directions
  - Octave spreading
- 📋 **Voice Leading** - Ensure smooth melodic movement between notes
  - Nearest note selection
  - Maximum interval constraints

**Benefits:** Ensures generated patterns are musically coherent and harmonically pleasing

**Implementation Complexity:** Medium (quantization), High (progressions/voice leading)
**Musical Impact:** Very High

---

### 3. Temporal Manipulation ✅

Control timing, groove, and rhythmic feel of generated patterns.

**Features:**

- ✅ **Swing/Humanization** - Add timing variations and groove (IMPLEMENTED v0.2.0)
  - Swing percentage (0-100%)
  - 6 swing groove templates
  - Micro-timing offsets (0-50ms)
  - Velocity humanization (0-100%)
  - Timing randomness per step
- ✅ **Ratcheting** - Repeat notes at subdivisions (IMPLEMENTED v0.5.0)
  - Ratchet count (1-16 repeats)
  - Probability-based ratcheting (0-100%)
  - Exponential velocity decay
- ✅ **Gate Length Control** - Control note duration (IMPLEMENTED v0.4.0)
  - Global gate length percentage (1-200%)
  - Legato mode toggle
  - Sample-accurate duration
- 📋 **Tempo Curves** - Dynamic tempo changes
  - Accelerando/ritardando over time
  - Tempo LFO modulation

**Benefits:** Adds human feel and groove, reduces mechanical/robotic sound

**Implementation Complexity:** Medium (MOSTLY COMPLETED)
**Musical Impact:** Very High

---

### 4. Pattern Evolution & Morphing 💡
Make patterns evolve and transform over time.

**Features:**
- **Mutation/Evolution** - Gradually mutate patterns using genetic algorithms
  - Mutation rate control
  - Fitness functions based on musical rules
  - Generation counter
- **Pattern Interpolation** - Morph between two saved patterns
  - Crossfade control (0-100%)
  - Morphing speed
- **Conditional Triggers** - "Play this pattern only every Nth repetition"
  - Fill patterns every N bars
  - Variation patterns
- **Probability Per Step** - Each step has individual probability
  - Per-step probability sliders
  - Probability curves

**Benefits:** Creates evolving, non-static compositions

**Implementation Complexity:** Medium-High
**Musical Impact:** High

---

### 5. Advanced Rhythm Features 💡
Extended rhythmic capabilities beyond current implementation.

**Features:**
- **Groove Templates** - Load/save micro-timing patterns
  - Import from MIDI files
  - Extract from audio (advanced)
- **Tuplet Support** - Triplets, quintuplets, septuplets
  - Triplet swing
  - Mixed tuplet divisions
- **Polymetric Modes** - Different time signatures per layer
  - 3/4 over 4/4
  - Custom polymeters
- **Randomized Subdivision** - Steps can be different note values
  - Mix 8ths, 16ths, 32nds in same pattern
  - Probability-based subdivision selection

**Benefits:** Expands rhythmic vocabulary significantly

**Implementation Complexity:** High
**Musical Impact:** Medium-High

---

## General Improvements

### 6. Preset Management 📋
Save, load, and organize patterns and settings.

**Features:**
- Factory preset library (20-50 starting presets)
- User preset save/load
- Preset browser UI with categories
- Preset tagging and search
- Preset morphing (interpolate between presets)
- Random preset generation
- Import/export preset banks

**Benefits:** Essential for workflow and sharing

**Implementation Complexity:** Medium
**Musical Impact:** High (usability)

---

### 7. MIDI I/O Enhancements 💡
Expand MIDI input/output capabilities.

**Features:**
- **MIDI Input Processing** - Transform incoming MIDI
  - Harmonizer (add harmony to input notes)
  - Rhythm quantizer
  - Velocity scaling
- **Pattern Recording** - Capture MIDI input into patterns
  - Overdub mode
  - Step recording
- **MIDI Learn** - Map hardware controllers to parameters
  - One-click learn mode
  - Save controller mappings
- ✅ **MIDI Channel Routing** - Send to specific MIDI channels (IMPLEMENTED v0.8.0)
  - Global channel selector (1-16)
  - UI integration
- 📋 **Per-Layer Channel Assignment** - Different channels per polyrhythm layer
  - Per-layer channel assignment
  - Multi-timbral support

**Benefits:** Better integration with hardware and DAWs

**Implementation Complexity:** Medium-High
**Musical Impact:** Medium

---

### 8. Modulation & Automation 💡
Dynamic parameter control over time.

**Features:**
- **LFOs** - Modulate any parameter
  - Sine, Triangle, Saw, Square, Random waveforms
  - Sync to tempo or free-running
  - Bipolar/Unipolar modes
- **Envelope Followers** - Modulate based on audio input
  - Attack/Release controls
  - Threshold and gain
- **Parameter Recording** - Record knob movements as automation
  - Playback recorded automation
  - Edit automation curves
- **Cross-modulation** - Use one generator to modulate another
  - Modulation matrix

**Benefits:** Creates dynamic, evolving performances

**Implementation Complexity:** High
**Musical Impact:** High

---

### 9. UI/UX Improvements 📋
Make the plugin more intuitive and efficient to use.

**Features:**
- **Pattern Copy/Paste** - Quick pattern duplication
- **Undo/Redo** - Pattern editing history (20+ steps)
- **Pattern Matrix** - Grid view for editing polyrhythm layers
  - Click to toggle steps
  - Drag to paint patterns
- **Piano Roll Display** - Alternative visualization showing pitch over time
- **Tooltips** - Contextual help for all parameters
- **A/B Comparison** - Quick snapshot/recall for tweaking
- **Accessibility** - Screen reader support, keyboard navigation

**Benefits:** Faster workflow, lower learning curve

**Implementation Complexity:** Medium
**Musical Impact:** Medium (usability)

---

### 10. Performance Features 💡
Live performance and arrangement tools.

**Features:**
- **Scene/Pattern Chains** - Sequence multiple patterns
  - Define pattern order (A→B→C→A)
  - Loop points
- **MIDI Program Change** - Switch presets via MIDI
  - Program change → preset mapping
- **Global Mute/Solo** - Per-layer control in polyrhythm mode
- **Momentary Effects** - Hold-to-activate transformations
  - Momentary reverse
  - Momentary double speed
- **Fill Patterns** - One-shot variations for transitions
  - Fill trigger button/MIDI

**Benefits:** Better live performance capabilities

**Implementation Complexity:** Medium
**Musical Impact:** Medium-High (for performers)

---

### 11. Export & Integration 💡
Share and integrate with other tools.

**Features:**
- **MIDI File Export** - Save generated patterns as .mid files
  - Export current pattern
  - Export entire performance
- **Pattern Sharing** - Export/import patterns as JSON/XML
  - Community pattern library
- **DAW Integration Improvements**
  - Better host automation support
  - Transport sync enhancements
- **Sync Options** - Multiple clock division modes
  - External MIDI clock
  - Ableton Link support

**Benefits:** Easier collaboration and integration

**Implementation Complexity:** Low-Medium
**Musical Impact:** Medium

---

### 12. Advanced Algorithmic Features 💡
Cutting-edge generative techniques.

**Features:**
- **Multi-voice Counterpoint** - Generate harmonically aware multiple voices
  - Species counterpoint rules
  - Independent melodic lines
- **Constraint Satisfaction** - Define musical rules
  - Avoid parallel fifths/octaves
  - Voice range constraints
  - Custom rule definition
- **Style Transfer** - Analyze MIDI input and generate in similar style
  - Machine learning-based (advanced)
  - Pattern analysis and replication
- **Feedback Loops** - Use recent output to influence future generation
  - Self-modifying patterns
  - Emergent behavior

**Benefits:** State-of-the-art generative capabilities

**Implementation Complexity:** Very High
**Musical Impact:** High (for advanced users)

---

## Priority Recommendations

### Phase 1 (Current) ✅
Focus on immediate musical impact with reasonable complexity.

1. ✅ **Scale Quantization** - Makes output immediately more musical (DONE)
2. ✅ **Swing/Humanization** - Adds groove and feel (DONE)
3. 📋 **Gate Length Control** - Crucial for rhythmic expression
4. 📋 **Preset Management** - Essential for usability
5. 📋 **Probability Per Step** - Huge creative flexibility

### Phase 2 (Next) 📋
Expand creative possibilities.

6. Pattern morphing/evolution
7. Ratcheting
8. MIDI learn
9. Pattern chains
10. LFO modulation

### Phase 3 (Future) 💡
Advanced features for power users.

- Chaos engines
- MIDI file export
- Advanced music theory features
- Multi-voice counterpoint
- Machine learning integration

---

## Implementation Notes

### Scale Quantization ✅

- Created `ScaleQuantizer` class in `Source/Core/ScaleQuantizer.h`
- Supports 16 common scales with MIDI note mapping
- Root note selection (C-B) via UI combo box
- Octave-aware quantization to nearest scale degree
- Applied to all generator types (Euclidean, Polyrhythm, Algorithmic)
- UI: Two combo boxes in Expression section (Root + Scale Type)

### Swing/Humanization ✅

- Created `SwingEngine` class in `Source/Core/SwingEngine.h`
- Swing percentage: affects timing of off-beat notes (0-100%)
- Timing randomness: +/- milliseconds per note (0-50ms)
- Velocity humanization: randomize within velocity range (0-100%)
- Integrates with `EventScheduler` for sample-accurate timing
- Applied to all generator types
- UI: Three rotary knobs in Expression section (Swing, Timing, Vel Var)

---

## Contributing

Have ideas for enhancements? Please:
1. Open a GitHub issue with the "enhancement" label
2. Describe the feature and its musical benefit
3. Include examples or references if applicable

---

## References

**Academic:**
- Toussaint, G. (2005). The Euclidean Algorithm Generates Traditional Musical Rhythms
- Roads, C. (1996). The Computer Music Tutorial - Chapter on Algorithmic Composition
- Cope, D. (1996). Experiments in Musical Intelligence

**Software:**
- Ableton Live - Groove templates, MIDI effects
- Native Instruments Reaktor - Modulation routing
- Arturia BeatStep Pro - Polyrhythm sequencing
- Make Noise René - Cartesian sequencer concepts

---

*Last Updated: 2025-10-17*
