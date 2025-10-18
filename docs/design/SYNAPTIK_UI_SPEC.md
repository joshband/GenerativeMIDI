# SYNAPTIK – UI Design Specification

**Where circuits dream — and music thinks.**

---

## Table of Contents

- [Design Philosophy](#design-philosophy)
- [Visual Identity](#visual-identity)
- [Color Palette](#color-palette)
- [Layout Architecture](#layout-architecture)
- [Component Library](#component-library)
- [Animation System](#animation-system)
- [Interaction Patterns](#interaction-patterns)
- [Typography](#typography)
- [Iconography](#iconography)
- [Implementation Roadmap](#implementation-roadmap)

---

## Design Philosophy

### Core Concept: "Circuit Temple – Neural Precision meets Sacred Geometry"

**Synaptik** represents the convergence of:
- **Ancient wisdom** (sacred geometry, temple architecture)
- **Neural networks** (biological synapses, brain activity)
- **Modular synthesis** (signal flow, patch cables)
- **Futuristic mysticism** (bioluminescence, quantum aesthetics)

### Design Principles

1. **Radial Organization** - Information flows from center outward
2. **Organic Technology** - Blend biological and mechanical aesthetics
3. **Reactive Feedback** - Every action creates visible energy flow
4. **Sacred Precision** - Mathematical beauty in layout and spacing
5. **Depth Through Light** - Glow, shadow, and luminescence create hierarchy

---

## Visual Identity

### Structure: Quadrant Architecture

```
┌─────────────────────────────────────────────┐
│                                             │
│     MODULATION  ┃    ◉ CORE ◉    ┃  TONE    │
│    (Chaos/LFO)  ┃   Neural Hub   ┃ (Pitch)  │
│                 ┃                ┃          │
│ ────────────────╋────────────────╋───────── │
│                 ┃                ┃          │
│      SPACE      ┃   Radial       ┃ DYNAMICS │
│   (Humanize)    ┃   Sequencer    ┃ (Evolve) │
│                                             │
└─────────────────────────────────────────────┘
```

### Central Core

**The Neural Hub:**
- Pulsating orb representing the "brain" of Synaptik
- Radiates energy to all four quadrants
- Visualizes overall system state:
  - **Pulse rate** = BPM
  - **Glow intensity** = Complexity/density
  - **Color shift** = Modulation depth

**Radial Sequencer:**
- Circular step sequencer surrounding the core
- Expands outward in concentric rings like temple architecture
- Active steps pulse with bioluminescent glow

---

## Color Palette

### Primary Colors

| Color Name | Hex Code | RGB | Usage |
|------------|----------|-----|-------|
| **Neural Void** | `#0A0E1A` | 10, 14, 26 | Background, deep space |
| **Temple Steel** | `#1A1F2E` | 26, 31, 46 | Panels, structure |
| **Circuit Trace** | `#2A3447` | 42, 52, 71 | Borders, dividers |
| **Synapse Blue** | `#00D4FF` | 0, 212, 255 | Primary accent, active states |
| **Mystic Violet** | `#9D4EDD` | 157, 78, 221 | Secondary accent, modulation |
| **Quantum Teal** | `#00FFC8` | 0, 255, 200 | Success, quantization |
| **Chaos Orange** | `#FF6B35` | 255, 107, 53 | Warning, stochastic |
| **Evolution Gold** | `#FFB627` | 255, 182, 39 | Pattern evolution, memory |

### Glow Effects

| Effect | Color | Opacity | Blur |
|--------|-------|---------|------|
| **Active Glow** | Synapse Blue | 60% | 12px |
| **Hover Glow** | Mystic Violet | 40% | 8px |
| **Pulse Glow** | Quantum Teal | 70% | 16px |
| **Warning Glow** | Chaos Orange | 50% | 10px |

### Gradient Overlays

```css
/* Neural Gradient (Core) */
radial-gradient(circle, #00D4FF 0%, #9D4EDD 50%, #0A0E1A 100%)

/* Temple Gradient (Panels) */
linear-gradient(135deg, #1A1F2E 0%, #2A3447 100%)

/* Energy Flow (Active paths) */
linear-gradient(90deg, #00D4FF 0%, #00FFC8 50%, #9D4EDD 100%)
```

---

## Layout Architecture

### Grid System

**Base Unit:** 8px
**Canvas Size:** 1200x800px (scalable 800x533 to 1920x1280)

### Quadrant Breakdown

#### Top-Left: MODULATION
**Purpose:** Control chaos, randomness, and time-based modulation

**Components:**
- LFO controls (future)
- Chaos engine selector
- Stochastic parameters
- Attractor visualization

**Dimensions:** 400x300px

---

#### Top-Right: TONE
**Purpose:** Pitch, scale, and harmonic control

**Components:**
- Scale root selector (wheel interface)
- Scale type dropdown with glyphs
- Pitch range sliders (vertical)
- Quantization on/off toggle

**Dimensions:** 400x300px

---

#### Bottom-Left: SPACE
**Purpose:** Temporal and dynamic humanization

**Components:**
- Swing knob (yin-yang symbol)
- Timing humanization knob
- Velocity variation knob
- Gate length control (future)

**Dimensions:** 400x300px

---

#### Bottom-Right: DYNAMICS
**Purpose:** Pattern evolution and memory

**Components:**
- Probability/density knob
- Pattern morph slider (future)
- Memory slots (crystalline capsules)
- A/B compare toggle

**Dimensions:** 400x300px

---

### Central Hub: 400x400px

**Layers (bottom to top):**
1. Neural core orb (120px diameter)
2. Radial step sequencer (outer rings)
3. Generator type selector (surrounding the core)
4. Pattern display (active step visualization)

---

## Component Library

### 1. Neural Knob

**Design:**
- Circular control with engraved tick marks
- Inner ring shows current value (0-100%)
- Outer ring glows on hover
- Center displays numeric value

**States:**
- **Idle:** Dim glow, 30% opacity
- **Hover:** Bright glow, pulse animation
- **Active:** Full glow, parameter bloom effect
- **Modulated:** Pulsing outline (if LFO assigned)

**Variants:**
- Small (60px): Swing, timing, velocity
- Medium (80px): Density, tempo
- Large (100px): Master controls

---

### 2. Quantum Slider

**Design:**
- Vertical track with sacred geometry markers
- Glowing thumb handle (diamond shape)
- Neural trace follows thumb position
- Min/max labels with futuristic glyphs

**States:**
- **Idle:** Subtle glow on track
- **Hover:** Track brightens
- **Drag:** Energy particles flow along track
- **Snapped:** Quick flash at quantized positions

**Variants:**
- Velocity range (dual thumb)
- Pitch range (dual thumb, note names)
- Single value (gate length, etc.)

---

### 3. Synapse Dropdown

**Design:**
- Rounded rectangle with circuit pattern border
- Icon on left, text in center, chevron on right
- Dropdown panel has semi-transparent dark background
- Options have glowing hover states

**States:**
- **Closed:** Minimal glow
- **Hover:** Border intensifies
- **Open:** Panel slides down with fade-in
- **Selected:** Check glyph appears, glow pulse

**Variants:**
- Generator type (with algorithm icons)
- Scale root (note names: C, C#, D, etc.)
- Scale type (with interval notation)

---

### 4. Radial Sequencer

**Design:**
- Concentric circles divided into steps
- Each step is a glowing cell
- Active steps: Full brightness
- Inactive steps: 20% opacity
- Current playhead: Rotating indicator beam

**Animation:**
- Pulse on beat (BPM-synced)
- Step activation fades in
- Playhead rotates smoothly
- Probability affects brightness per step

**Interaction:**
- Click step to toggle on/off
- Drag to paint pattern
- Right-click for per-step menu (future)

**Variants:**
- Small (8 steps)
- Medium (16 steps)
- Large (32/64 steps)

---

### 5. Neural Core Orb

**Design:**
- Sphere with wireframe neural network overlay
- Nodes pulse at subdivision intervals
- Energy flows from center to perimeter
- Color shifts based on modulation

**States:**
- **Playing:** Pulsing at BPM
- **Stopped:** Slow breathing animation
- **High Complexity:** Faster node firing
- **Modulation Active:** Color spectrum shift

**Data Visualization:**
- **Size:** Activity level
- **Color:** Modulation depth
- **Pulse Rate:** BPM
- **Node Count:** Pattern complexity

---

### 6. Memory Crystal Slot

**Design:**
- Hexagonal capsule shape
- Contains miniature frozen pattern preview
- Glows when populated
- Empty slots show faint outline

**States:**
- **Empty:** Dark, dashed outline
- **Stored:** Bright glow, pattern preview visible
- **Active:** Full brightness, pulsing
- **Hover:** Tooltip shows snapshot details

**Interaction:**
- Click to store current state
- Right-click to load
- Drag to reorder (future)

---

## Animation System

### Core Animations

#### 1. Neural Pulse
**Trigger:** BPM clock, continuously
**Effect:** Radial energy wave from core outward

```
Duration: Beat duration (60000ms / BPM)
Easing: ease-out
Path: Radial from center
Opacity: 100% → 0%
Scale: 1.0 → 2.0
```

---

#### 2. Parameter Bloom
**Trigger:** User adjusts control
**Effect:** Glowing particles emanate from control

```
Duration: 600ms
Easing: ease-out
Particle Count: 8-12
Particle Life: fade-out over 400ms
Direction: Radial outward
Color: Control's accent color
```

---

#### 3. Synapse Mode
**Trigger:** Spacebar or button click
**Effect:** Reveals modulation paths as glowing neuron links

```
Duration: 1200ms (entrance), 800ms (exit)
Easing: cubic-bezier(0.4, 0.0, 0.2, 1)
Effect:
  - Time dilation (UI slows 50%)
  - Modulation paths draw from source → target
  - Connected parameters pulse in sync
  - Background dims 40%
```

---

#### 4. Morph Transition
**Trigger:** Pattern evolution, preset change
**Effect:** Fractal reconfiguration of sequencer

```
Duration: 1000ms
Easing: ease-in-out
Effect:
  - Old pattern fades out
  - New pattern fades in
  - Intermediate steps show morph blend
  - Core orb pulses brightly during change
```

---

#### 5. Cognitive Drift
**Trigger:** "Drift" button toggle
**Effect:** Unstable synaptic sparks jump between nodes

```
Duration: Continuous while active
Interval: Random 200-800ms
Effect:
  - Random parameter micro-shifts (±5%)
  - Spark particles jump between controls
  - Affected controls glow briefly
  - Core orb shows chaotic activity
```

---

#### 6. Audio-Reactive Spectrum
**Trigger:** MIDI output activity
**Effect:** Spectrum visualization shaped like MRI cross-section

```
Update Rate: 30 FPS
Frequency Bins: 32
Color Map: Cyan → Violet
Display Mode:
  - Amplitude Mode: Bar height = note velocity
  - Complexity Mode: Bar pattern = algorithmic density
```

---

### Micro-Interactions

| Interaction | Animation | Duration |
|-------------|-----------|----------|
| **Button Hover** | Glow increase + scale 1.05 | 200ms |
| **Button Press** | Scale 0.95 + brightness spike | 100ms |
| **Knob Rotate** | Trail particles | 300ms |
| **Slider Drag** | Energy flow along track | Continuous |
| **Toggle On** | Light-up + pulse | 400ms |
| **Toggle Off** | Fade-out | 300ms |
| **Panel Open** | Slide down + fade in | 350ms |
| **Panel Close** | Fade out + slide up | 250ms |

---

## Interaction Patterns

### Synapse Mode (Modulation Visualization)

**Activation:**
- **Keyboard:** Spacebar
- **UI Button:** "◈ Synapse" button (top-right)

**Behavior:**
1. Background dims to 40%
2. UI slows to 50% speed (time dilation effect)
3. Modulation paths draw as glowing neuron links
4. Source → Target connections pulse
5. Unmodulated controls fade to 30%
6. Press again to exit

**Use Case:**
- Understand modulation routing
- Debug complex patches
- Visual performance element

---

### Cognitive Drift (Controlled Chaos)

**Activation:**
- **UI Button:** "∿ Drift" toggle (bottom-left)

**Behavior:**
1. All modulatable parameters drift randomly
2. Drift amount: ±5% per interval
3. Drift interval: Random 200-800ms
4. Visual: Unstable sparks between nodes
5. Core orb shows chaotic firing pattern

**Use Case:**
- Generative unpredictability
- Live performance variation
- "Happy accidents" discovery

---

### Memory Lock (Freeze Subsystems)

**Activation:**
- **UI Element:** Lock glyph on each quadrant header

**Behavior:**
1. Click lock icon to freeze that quadrant
2. Locked parameters turn amber/gold
3. Locked parameters ignore:
   - Preset changes (stay frozen)
   - Randomization
   - Drift effects
4. Unlock restores normal operation

**Use Case:**
- Isolate evolving modules
- Lock rhythm while changing harmony
- Preserve perfect settings during experimentation

---

### Brain Scan Meter (Complexity Visualization)

**Location:** Top-center, above core

**Display Modes:**
1. **Amplitude Mode:**
   - Shows rhythmic energy distribution
   - Bar height = note velocity
   - Color = MIDI channel

2. **Complexity Mode:**
   - Shows pattern density
   - Bar pattern = algorithmic activity
   - Color = generator type

**Interaction:**
- Click to toggle mode
- Hover for detailed readout

---

## Typography

### Font Stack

#### Headers / Labels
**Primary:** Cinzel Decorative
- **Usage:** Quadrant titles, section headers
- **Style:** Bold, uppercase, letter-spacing 0.1em
- **Fallback:** `'Cinzel Decorative', serif`

#### Parameters / Numbers
**Primary:** Orbitron
- **Usage:** Value displays, numeric readouts
- **Style:** Medium weight, tabular numerals
- **Fallback:** `'Orbitron', 'Courier New', monospace`

#### Supporting Text
**Primary:** Inter
- **Usage:** Tooltips, descriptions, help text
- **Style:** Regular, 14px base size
- **Fallback:** `'Inter', -apple-system, sans-serif`

### Type Scale

| Style | Font | Size | Weight | Use Case |
|-------|------|------|--------|----------|
| **Hero** | Cinzel | 48px | Bold | Plugin title |
| **H1** | Cinzel | 24px | Bold | Quadrant titles |
| **H2** | Orbitron | 18px | Medium | Section headers |
| **Body** | Inter | 14px | Regular | Labels, tooltips |
| **Caption** | Orbitron | 12px | Regular | Value displays |
| **Micro** | Inter | 10px | Regular | Hints, footnotes |

---

## Iconography

### Icon Set: "Neural Glyphs"

All icons follow a unified visual language:
- **Stroke width:** 2px
- **Corner radius:** 2px
- **Grid:** 24x24px
- **Export:** SVG
- **Color:** Synapse Blue (#00D4FF)

### Generator Icons

| Generator | Glyph Motif | Description |
|-----------|-------------|-------------|
| **Euclidean** | Circle segments | Evenly distributed arcs |
| **Polyrhythm** | Overlapping circles | Interlocking rings |
| **Markov** | Chain links | Connected nodes with arrows |
| **L-System** | Branching tree | Fractal growth pattern |
| **Cellular** | Grid cells | Rule-based cell matrix |
| **Probabilistic** | Scatter dots | Random distribution |

### Expression Icons

| Function | Glyph Motif | Description |
|----------|-------------|-------------|
| **Swing** | Yin-yang phase | Curved opposing arcs |
| **Timing** | Clock with offset | Clock face with jitter |
| **Velocity** | Waveform crescendo | Rising amplitude curve |
| **Scale** | Staircase | Ascending steps |
| **Quantize** | Grid snap | Aligned dots to grid |

### Modulation Icons

| Function | Glyph Motif | Description |
|----------|-------------|-------------|
| **LFO** | Rotating orbit | Elliptical path with dot |
| **Chaos** | Lorenz butterfly | 3D attractor wireframe |
| **Envelope** | ADSR curve | Attack-decay-sustain-release |
| **Random** | Dice | Six-sided die |

### Memory / Scene Icons

| Function | Glyph Motif | Description |
|----------|-------------|-------------|
| **Snapshot** | Crystal | Hexagonal gem shape |
| **Recall** | Temple tablet | Stone slab with circuit |
| **Lock** | Padlock | Closed lock with circuit |
| **Unlock** | Open lock | Unlocked state |

---

## Implementation Roadmap

### Phase 1: Foundation (Weeks 1-2)
- ✅ Design specification complete
- ☐ Color palette implementation
- ☐ Typography setup (font loading)
- ☐ Base layout grid (quadrant structure)
- ☐ Dark theme with neural void background

### Phase 2: Core Components (Weeks 3-4)
- ☐ Neural Knob component
- ☐ Quantum Slider component
- ☐ Synapse Dropdown component
- ☐ Component library documentation
- ☐ Interaction states (hover, active, disabled)

### Phase 3: Central Hub (Weeks 5-6)
- ☐ Radial sequencer implementation
- ☐ Neural core orb visualization
- ☐ BPM-synced pulse animation
- ☐ Step editing interaction
- ☐ Pattern preview

### Phase 4: Quadrant Layout (Weeks 7-8)
- ☐ Modulation quadrant (top-left)
- ☐ Tone quadrant (top-right)
- ☐ Space quadrant (bottom-left)
- ☐ Dynamics quadrant (bottom-right)
- ☐ Responsive layout system

### Phase 5: Animation System (Weeks 9-10)
- ☐ Neural pulse animation
- ☐ Parameter bloom effect
- ☐ Synapse mode (modulation paths)
- ☐ Morph transition
- ☐ Audio-reactive spectrum

### Phase 6: Advanced Features (Weeks 11-12)
- ☐ Cognitive drift implementation
- ☐ Memory lock system
- ☐ Brain scan meter
- ☐ Memory crystal slots
- ☐ A/B comparison

### Phase 7: Polish & Optimization (Weeks 13-14)
- ☐ Performance optimization
- ☐ Accessibility improvements
- ☐ Touch/gesture support (iOS)
- ☐ Preset management UI
- ☐ User testing & refinement

---

## Technical Considerations

### JUCE Implementation

**Custom LookAndFeel:**
- Extend `CustomLookAndFeel` class
- Override drawing methods for new aesthetics
- Implement neural glow effects using `Graphics::setGradientFill()`

**Animation Timer:**
- 60 FPS refresh for smooth animations
- Use `AnimatedPosition` for smooth transitions
- Implement custom easing functions

**OpenGL Rendering:**
- Consider OpenGL for complex animations
- Shader effects for neural glow
- Hardware acceleration for 60 FPS performance

### Performance Targets

- **CPU Usage:** < 10% for UI (separate from audio)
- **Frame Rate:** 60 FPS sustained
- **Memory:** < 50MB for UI assets
- **Startup Time:** < 2 seconds to full UI

---

## Mockup Priorities

### For Figma/Framer Prototype

1. **Core View** (Default state)
   - Radial sequencer active
   - All quadrants visible
   - Neural pulse animation

2. **Synapse Mode** (Modulation visualization)
   - Modulation paths glowing
   - Time dilation effect
   - Background dimmed

3. **Cognitive Drift** (Chaos active)
   - Sparks jumping between nodes
   - Core orb chaotic activity
   - Parameters drifting visually

4. **Memory Management** (Snapshot system)
   - Crystal slots populated
   - Hover states
   - Recall animation

---

## Design Files Structure

```
/design
  /assets
    /fonts
      - Cinzel-Bold.ttf
      - Orbitron-Medium.ttf
      - Inter-Regular.ttf
    /icons
      /generators
      /expression
      /modulation
      /memory
    /textures
      - neural-circuit-pattern.svg
      - temple-engraving.png
  /mockups
    - synaptik-default.fig
    - synaptik-synapse-mode.fig
    - synaptik-components.fig
  /prototypes
    - synaptik-interactive.framer
  /specs
    - SYNAPTIK_UI_SPEC.md (this file)
    - color-palette.json
    - typography-scale.json
```

---

## Alignment with Feature Roadmap

| Enhancement | UI Element | Quadrant |
|-------------|------------|----------|
| **LFO System** | Modulation quadrant | Top-Left |
| **Chaos Engines** | Cognitive Drift toggle | Bottom-Left |
| **Scale Quantization** | ✅ Scale wheel & dropdown | Top-Right |
| **Swing/Humanization** | ✅ Space quadrant knobs | Bottom-Left |
| **Pattern Evolution** | Morph slider | Bottom-Right |
| **Preset Management** | Memory crystals | Bottom-Right |
| **Probability Per Step** | Radial sequencer brightness | Center |
| **Gate Length** | Space quadrant | Bottom-Left |

---

## Brand Assets

### Logo Concept
**Synaptik wordmark with neural node icon**

```
     ◉―◉―◉
       │
   SYNAPTIK
```

### Tagline
**"Where circuits dream — and music thinks."**

### Social Media Colors
- **Primary:** Synapse Blue (#00D4FF)
- **Secondary:** Mystic Violet (#9D4EDD)
- **Background:** Neural Void (#0A0E1A)

---

## Conclusion

This specification provides the foundation for transforming **GenerativeMIDI** into **Synaptik** — a next-generation generative MIDI processor with a unique visual identity that bridges ancient mysticism and futuristic technology.

The quadrant architecture, neural aesthetics, and reactive animations create an immersive experience that reflects the complexity and beauty of algorithmic composition.

---

**Document Version:** 1.0
**Last Updated:** 2025-10-17
**Status:** Foundation Complete, Ready for Implementation

---

**Next Steps:**
1. Review and approve design specification
2. Set up Figma/Framer project
3. Create icon library (SVG exports)
4. Begin Phase 1 implementation (color palette + typography)

