# SYNAPTIK Component Specifications

Visual design specifications for UI components

---

## Neural Knob (Rotary Control)

### Dimensions
- **Small:** 60px diameter
- **Medium:** 80px diameter
- **Large:** 100px diameter

### Visual Layers (bottom to top)

1. **Neural Glow Aura**
   - Radius: knob radius + 12px
   - Gradient: Synapse Blue (40% center → 0% edge)
   - Effect: Radial blur, soft edges

2. **Circuit Trace Ring**
   - Stroke: 2px
   - Color: Circuit Trace (#2A3447)
   - Position: Outer edge + 2px

3. **Temple Steel Body**
   - Fill: Gradient (Temple Steel bright → dark)
   - Direction: Top to bottom
   - Shape: Perfect circle

4. **Sacred Geometry Ticks**
   - Count: 12 (360° / 30° each)
   - Position: 75-90% of radius
   - Stroke: 1.5px, Circuit Trace
   - Length: 15% of radius

5. **Active Arc Indicator**
   - Path: From min angle to current value
   - Stroke: 3px, Synapse Blue (60% opacity)
   - Radius: 85% of knob
   - Caps: Rounded

6. **Neural Node Center**
   - Inner glow: Synapse Blue (80% → 0%)
   - Core: Temple Steel (brighter)
   - Size: 20% of radius
   - Highlight: Quantum Teal dot

7. **Bioluminescent Pointer**
   - Glow trail: Synapse Blue (60%)
   - Core beam: Synapse Blue (100%)
   - Width: 3px
   - Length: 70% of radius
   - Tip highlight: White (80%)

### States

| State | Glow Opacity | Pointer Color | Behavior |
|-------|--------------|---------------|----------|
| **Idle** | 30% | Synapse Blue | Subtle breathing (optional) |
| **Hover** | 50% | Quantum Teal | Scale 1.05x |
| **Active** | 70% | Synapse Blue | Particle bloom |
| **Modulated** | Pulsing 40-70% | Mystic Violet | Pulse at LFO rate |

---

## Quantum Slider (Linear Control)

### Dimensions
- **Track Width:** 10px
- **Track Height:** 80-150px (vertical)
- **Thumb Size:** 14px

### Visual Layers

1. **Neural Channel Background**
   - Fill: Neural Void
   - Border: Circuit Trace (1.5px)
   - Corners: Fully rounded (5px radius)

2. **Energy Flow Fill**
   - Gradient: Synapse Blue (30% → 80%)
   - Direction: Bottom to top (filled portion)
   - Corners: Match track

3. **Diamond Thumb**
   - Shape: 4-point diamond (sacred geometry)
   - Glow: Synapse Blue (50%), 1.2x size
   - Fill: Synapse Blue
   - Highlight: White dot (80%), off-center top

### Specifications
```
Diamond Points:
  Top:    (x, y - 0.7h)
  Right:  (x + 0.7w, y)
  Bottom: (x, y + 0.7h)
  Left:   (x - 0.7w, y)

Where h = w = thumb size (14px)
```

---

## Synapse Dropdown (ComboBox)

### Dimensions
- **Height:** 28-32px
- **Width:** Auto (min 120px)
- **Border Radius:** 4px

### Visual Design

**Closed State:**
```
┌─────────────────────────┐
│ Background: Temple Steel│
│ Border: Synapse Blue 1.5px
│ Text: Quantum Teal      ▼│
└─────────────────────────┘
```

**Chevron:**
- Stroke: 2px, Quantum Teal
- Shape: V pointing down
- Size: 12x8px
- Position: Right margin + 6px

**Dropdown Panel:**
- Background: Temple Steel (95% opacity)
- Border: Synapse Blue glow (2px)
- Shadow: Neural Void (80%), 8px blur
- Item height: 28px
- Item hover: Synapse Blue (20% bg)
- Item selected: Synapse Blue (40% bg)

---

## Neural Button (Toggle/Push)

### Dimensions
- **Small:** 60x24px
- **Medium:** 80x32px
- **Large:** 100x40px

### Visual States

**Idle (Off):**
- Background: Temple Steel gradient
- Border: Synapse Blue (30% opacity), 2px
- Text: Quantum Teal

**Hover:**
- Background: Temple Steel (brighter 20%)
- Border: Synapse Blue (50% opacity)
- Glow: Synapse Blue aura (4px expand)

**Active (On):**
- Background: Temple Steel (brighter 50%)
- Border: Synapse Blue (80% opacity)
- Glow: Synapse Blue aura (6px expand)
- Text: Neural Void (high contrast)

**Pressed:**
- Background: Synapse Blue (60%)
- Border: Quantum Teal (100%)
- Scale: 0.97x (slight squish)

---

## Circuit Panel (Container)

### Specifications

**Background:**
```css
gradient: Temple Steel (bright 5% → dark 10%)
direction: top to bottom
```

**Border:**
- Stroke: 1.5px
- Color: Synapse Blue (40% opacity)
- Radius: 6px
- Glow: Synapse Blue (5%), 2px expand

**Header Label:**
- Symbol: ◈ (diamond)
- Font: Bold, 14px
- Color: Quantum Teal
- Padding: 12px left, 6px top

**Spacing:**
- Padding: 16px
- Margin: 8px between panels
- Gap: 12px between items

---

## Radial Sequencer (Pattern Display)

### Dimensions
- **Outer Radius:** 120-180px
- **Step Count:** 8, 16, 32, or 64
- **Ring Width:** 20px

### Visual Design

**Center Core:**
- Size: 60px diameter
- Fill: Neural node with pulse
- Glow: Synapse Blue (70%)

**Step Cells:**
- Active: Synapse Blue (100%)
- Inactive: Circuit Trace (20%)
- Current: Quantum Teal + pulse
- Width: (360° / stepCount)
- Padding: 2° between cells

**Playhead:**
- Type: Rotating beam
- Color: Quantum Teal (60%)
- Width: 3px
- Length: Full radius
- Shadow: Glow trail

**Interaction:**
- Click to toggle: Step flips state
- Drag to paint: Multiple steps
- Right-click: Step menu (future)

### Animation
```
Pulse Rate: BPM / 60 * 1000 ms
Rotation: 360° per pattern length
Ease: Linear (constant speed)
```

---

## Typography Specifications

### Font Stack

**Headers:**
```css
font-family: 'Cinzel Decorative', serif;
font-weight: 700;
letter-spacing: 0.1em;
text-transform: uppercase;
```

**Values/Numbers:**
```css
font-family: 'Orbitron', 'Courier New', monospace;
font-weight: 500;
font-variant-numeric: tabular-nums;
```

**Body/Labels:**
```css
font-family: 'Inter', -apple-system, sans-serif;
font-weight: 400;
line-height: 1.5;
```

### Type Scale

| Style | Font | Size | Weight | Use |
|-------|------|------|--------|-----|
| **Hero** | Cinzel | 48px | Bold | Plugin title |
| **H1** | Cinzel | 24px | Bold | Section headers |
| **H2** | Orbitron | 18px | Medium | Subsections |
| **Body** | Inter | 14px | Regular | Labels |
| **Caption** | Orbitron | 12px | Regular | Values |
| **Micro** | Inter | 10px | Regular | Hints |

---

## Spacing System

**Base Unit:** 8px

```
4px  = 0.5 unit (tight)
8px  = 1 unit (base)
12px = 1.5 units (snug)
16px = 2 units (comfortable)
24px = 3 units (loose)
32px = 4 units (section)
48px = 6 units (major)
```

**Component Margins:**
- Knobs: 16px horizontal, 12px vertical
- Sliders: 24px vertical
- Panels: 8px between, 16px padding
- Buttons: 8px gap in groups

---

## Animation Timing

### Easing Functions

```javascript
// Smooth in-out
cubic-bezier(0.4, 0.0, 0.2, 1.0)

// Snap to position
cubic-bezier(0.0, 0.0, 0.2, 1.0)

// Bounce
cubic-bezier(0.68, -0.55, 0.265, 1.55)

// Elastic
cubic-bezier(0.175, 0.885, 0.32, 1.275)
```

### Duration Guidelines

| Interaction | Duration | Ease |
|-------------|----------|------|
| **Hover** | 200ms | Smooth in-out |
| **Click** | 100ms | Snap |
| **Panel open** | 350ms | Smooth in-out |
| **Fade** | 250ms | Linear |
| **Pulse** | 1000ms | Elastic |
| **Bloom** | 600ms | Ease-out |

---

## Iconography Grid

### Grid System
- **Base:** 24x24px
- **Safe Area:** 20x20px (2px padding)
- **Stroke:** 2px (standard), 1.5px (detail)
- **Corners:** 2px radius
- **Export:** SVG, PNG @1x @2x @3x

### Icon Categories

1. **Generators** (geometric glyphs)
   - Euclidean: Circle segments
   - Markov: Chain links
   - L-System: Branching tree
   - Cellular: Grid cells
   - Probabilistic: Scatter dots

2. **Expression** (waveforms)
   - Swing: Yin-yang phase
   - Timing: Clock with jitter
   - Velocity: Crescendo wave
   - Scale: Ascending stairs
   - Quantize: Grid snap

3. **Modulation** (orbits)
   - LFO: Rotating orbit
   - Chaos: Butterfly attractor
   - Envelope: ADSR curve
   - Random: Dice

4. **Memory** (sacred geometry)
   - Crystal: Hexagon gem
   - Lock: Circuit padlock
   - Tablet: Stone slab
   - Snapshot: Camera glyph

---

## Implementation Notes

### CSS Custom Properties

```css
/* Component spacing */
--knob-size-sm: 60px;
--knob-size-md: 80px;
--knob-size-lg: 100px;
--slider-width: 10px;
--button-height: 32px;

/* Animation */
--anim-fast: 100ms;
--anim-normal: 200ms;
--anim-slow: 350ms;
--ease-smooth: cubic-bezier(0.4, 0.0, 0.2, 1.0);

/* Glow effects */
--glow-sm: 0 0 8px;
--glow-md: 0 0 14px;
--glow-lg: 0 0 24px;
```

---

**Last Updated:** 2025-10-17
**Version:** 1.0
**Status:** Reference for implementation
