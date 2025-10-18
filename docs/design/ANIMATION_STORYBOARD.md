# SYNAPTIK - Animation & Effects Storyboard

**Steampunk motion: mechanical precision meets ethereal energy**

---

## 🎬 Core Animation Principles

### The "Brass Engine" Philosophy

1. **Mechanical Precision** - Controls move with clockwork accuracy
2. **Steam Tells Stories** - Vapor shows energy transfer and state
3. **Aether Flows** - Cyan particles visualize data movement
4. **Brass Responds** - Metal surfaces react to interaction
5. **Sacred Rhythm** - Animations sync to BPM where appropriate

---

## ⚙️ Knob Rotation Animation

### Scenario: User Adjusts "SWING" Knob

**Frame 0ms - Idle State:**
```
Visual:
  - Knob shows current value (45%)
  - Subtle breathing glow on value arc (2s cycle)
  - Occasional white steam wisps from edges (every 3s)
  - Brass appears aged, with dark patina in grooves
```

**Frame 0-150ms - User Grabs Knob:**
```
Trigger: Mouse down / Touch start

Visual Changes:
  - Brass highlight increases by 20% (polished effect)
  - Steam emission stops (pressure building)
  - Cursor changes to brass hand (closed fist)
  - Subtle scale: 1.0 → 1.05 (appears to lift slightly)
  - Shadow darkens (depth illusion)

Timing: 100ms ease-out
```

**Frame 150-450ms - Rotation to New Value (72%):**
```
Trigger: User drags/rotates

Continuous Effects:
  - Knob rotates smoothly following pointer
  - Value arc extends in real-time (cyan glow)
  - Brass pointer needle follows rotation
  - Engraved scale marks pass under "window"
  - Mechanical sound (optional): ratchet clicks

Particle Effects:
  - Amber sparks emit from moving parts (5-10 particles)
  - Sparks: 1-2px, rise 20-40px, fade in 0.5-1s
  - Origin: Edge of knob body
  - Frequency: Every 45° of rotation

Timing: Variable (follows user input)
Easing: None (direct 1:1 mapping)
```

**Frame 450-600ms - Release & Settle:**
```
Trigger: Mouse up / Touch end

Immediate (0ms):
  - Steam BURST from all edges
  - 10-15 white steam particles (30-60px blobs)
  - Emission velocity: 80px/s upward
  - Opacity: 60% → 0% over 2-3 seconds

Animation (150ms ease-out):
  - Scale: 1.05 → 1.0 (settles back)
  - Brass highlight: Returns to idle
  - Shadow: Returns to normal

Aether Flow (300ms):
  - Cyan particles flow from knob to affected parameters
  - Path: Golden circuit trace
  - Speed: 200px/s
  - Particle size: 2-3px
  - Glow: 4px blur per particle
  - Count: 8-12 particles in sequence

Value Arc:
  - Final pulse: Glow brightens to 90%, then settles to 70%
  - Duration: 400ms
  - Easing: Sine wave
```

**Frame 600ms+ - New Idle State:**
```
Visual:
  - Knob at new position (72%)
  - Value arc showing new value with glow
  - Resume breathing animation (2s cycle)
  - Resume occasional steam wisps (every 3s)
  - Brass returns to aged appearance
```

---

## 📊 Slider Movement Animation

### Scenario: User Adjusts "VELOCITY" Slider from 30% to 80%

**Idle State:**
```
Visual:
  - Hexagonal crystal thumb at 30% position
  - Filled area (bottom to 30%): Cyan-teal gradient
  - Rising aether particles in filled region (3-5 visible)
  - White steam at current value level (gentle wisp every 4s)
  - Brass rails polished but aged
```

**Grab Thumb (0-100ms):**
```
Effects:
  - Crystal brightness: 60% → 100%
  - Glow expansion: 12px → 18px blur
  - Thumb scale: 1.0 → 1.1
  - Brass rails: Slight highlight increase
  - Steam emission: Pauses

Timing: 80ms ease-out
```

**Drag to New Position (100-400ms):**
```
Continuous:
  - Thumb follows mouse/touch Y position
  - Filled gradient extends/contracts in real-time
  - Aether particles spawn faster (during movement)
  - Value number updates (digital readout)

Trail Effect:
  - Cyan glow trail behind moving thumb
  - Trail fades over 200ms
  - Creates sense of speed

Particles (If Moving Up):
  - Cyan sparks "pulled" upward with thumb
  - 15-20 particles
  - Speed: Matches thumb velocity
  - Size: 2-4px

Timing: Direct (no easing, follows input)
```

**Release (400-600ms):**
```
Immediate:
  - STEAM BURST from new value level
  - Bilateral emission (left and right from slider)
  - 12-18 white steam particles
  - Opacity: 70% → 0% over 2.5s

Crystal Effect (200ms):
  - Bright flash (100% white)
  - Return to 80% cyan glow
  - Hexagonal facets catch light
  - Small rainbow refraction (optional)

Settling (300ms ease-out):
  - Thumb scale: 1.1 → 1.0
  - Glow: 18px → 12px
  - Rails: Return to idle

Aether Flow (New Value):
  - Particles resume normal slow rise
  - Density matches new value (80% → more particles)
  - Each particle: 2-3 second lifespan
```

---

## 🔘 Button Press Animation

### Scenario: Toggle "DRIFT" Button (Off → On)

**Idle Off State:**
```
Visual:
  - Brass cap: Aged, no glow
  - Bezel: Dark bronze
  - Text: "DRIFT" engraved dark
  - No steam
```

**Hover (0-200ms):**
```
Effects:
  - Brass cap: +10% brightness
  - Cyan outline: 0% → 30% opacity
  - Faint steam wisps from corners (2-3 particles)
  - Cursor: Brass hand pointer

Timing: 150ms ease-in
```

**Press Down (200-300ms):**
```
Physical:
  - Button translates Y +2px (mechanical depression)
  - Brass darkens 10% (shadow from pressing)
  - Bezel edge highlights (light source above)

Steam:
  - Strong steam burst from all sides
  - 15-20 white particles
  - Radial explosion pattern
  - Fast initial velocity (150px/s)

Sound: Mechanical "CLUNK" (optional)

Timing: 50ms snap (instant feel)
```

**Transition to On (300-600ms):**
```
Electrical Spark:
  - Amber lightning arc across button surface
  - Duration: 120ms flash
  - Origin: Top-left to bottom-right
  - 3-5 branching paths
  - Glow: Intense 20px blur

Cyan Crystal Activation:
  - Center of button illuminates
  - Hexagonal cyan crystal appears
  - Faceted glow radiates outward
  - Opacity: 0% → 90% over 200ms

Bezel Glow:
  - Cyan backlight from recess
  - Glow: 0% → 70% opacity, 8px blur
  - Creates "powered on" appearance

Timing: 300ms total, ease-out
```

**Active On State (600ms+):**
```
Visual:
  - Brass cap: Polished gold appearance
  - Crystal: Pulsing cyan glow (2s cycle: 70% ↔ 90%)
  - Bezel: Sustained cyan glow (70%)
  - Steam: Continuous gentle venting (1-2 particles/s)
  - Amber sparks: Occasional (every 3-5s, 2-3 particles)

Idle Behavior:
  - Button appears "alive" and powered
  - Energy flows into/out of button via circuit traces
```

---

## 🌀 Radial Sequencer Animation

### Scenario: Pattern Playing at 120 BPM (16 steps)

**BPM Timing:**
```
Beat Duration: 500ms (120 BPM)
Step Duration: 125ms (16 steps per 4 beats)
```

**Idle State (Before Play):**
```
Visual:
  - 16 hexagonal cells arranged radially
  - Active steps: Glowing cyan crystals
  - Inactive steps: Dark brass outlines
  - Center hub: Gentle amber pulse (2s cycle)
  - Brass orrery rings: Static
  - Occasional steam from center (every 5s)
```

**Playback Start (0ms):**
```
Center Hub:
  - Bright amber flash (200ms)
  - Golden energy wave radiates outward
  - Wave reaches outer ring in 400ms
  - Steam burst from center hub

Playhead:
  - Ornate brass pointer appears (fade in 200ms)
  - Positioned at step 1
  - Amber glow at tip

Timing: Preparation phase
```

**Per-Step Animation (Every 125ms):**
```
Step N Triggers:

Immediate (0ms):
  - Current step cell: FLASH bright (100% white)
  - Aether crystal: Maximum glow
  - Steam burst from cell (small, 3-5 particles)
  - If note triggered: Electrical spark to output

Playhead Rotation (0-125ms):
  - Brass pointer rotates to next step
  - Smooth continuous rotation
  - Amber glow trail (motion blur)
  - Speed: Constant (matches BPM)
  - Easing: Linear (mechanical precision)

Step N Glows (0-500ms):
  - Flash: 100% white (0-50ms)
  - Settle: 100% → 70% cyan (50-200ms)
  - Sustain: 70% cyan (200-400ms)
  - Fade: 70% → 60% (400-500ms)

Energy Flow (If Active Step):
  - Golden circuit trace lights up
  - Path: From center hub to step cell
  - Duration: 80ms (fast)
  - Cyan aether particles flow outward
  - Particle count: 5-8
  - Speed: 300px/s

Center Hub Pulse:
  - Pulses in sync with beat (every 4 steps)
  - Amber glow: 60% → 95% → 60%
  - Duration: 200ms (quick pulse)
  - Steam burst on downbeat

Orrery Rings (Optional):
  - Rings rotate slowly opposite to playhead
  - Full rotation: 8-16 seconds
  - Gives sense of cosmic motion
  - Brass gears between rings turn

Timing: Synchronized to 125ms step clock
```

**Pattern Completion (After 16 Steps):**
```
End of Loop:
  - All steps briefly flash in sequence (wave effect)
  - Golden energy wave from outer ring → center
  - Duration: 300ms
  - Center hub: Strong pulse
  - Steam burst from hub

Loop Restart:
  - Playhead continues seamlessly
  - Pattern repeats
  - No interruption in motion

Timing: Smooth continuous loop
```

**Playback Stop:**
```
Deceleration (0-500ms):
  - Playhead slows rotation (ease-out)
  - Final position: Snap to nearest step
  - Golden traces fade out (300ms)

Center Hub (500-800ms):
  - Amber glow fades: 70% → 40%
  - Steam emission slows
  - Returns to idle 2s pulse

Step Cells:
  - Active: Cyan glow reduces 70% → 50%
  - Inactive: Remain dark brass
  - All: No more flashing

Steam:
  - Final burst from center (dissipates over 3s)
  - Returns to occasional wisps

Timing: Graceful slowdown over 800ms
```

---

## 💨 Steam Behavior Specifications

### White Steam (General Purpose)

**Spawn Parameters:**
```javascript
{
  particle: {
    shape: "blob",
    size: [20, 80],  // Random between 20-80px
    color: "#FFFFFF",
    opacity: [0.4, 0.7],  // Initial range
  },
  motion: {
    velocity: [50, 100],  // px/s upward
    turbulence: 0.3,  // Perlin noise strength
    drift: [-20, 20],  // Horizontal drift px/s
  },
  lifespan: {
    duration: [2000, 4000],  // ms
    fade: "linear",
    opacityEnd: 0,
  },
  visual: {
    blur: [30, 60],  // px
    blurIncrease: 10,  // Blur increases with age
  }
}
```

**Emission Scenarios:**
```
Gentle Idle Venting:
  - Rate: 1 particle per 3-5 seconds
  - Origin: Component edges, vents
  - Purpose: Ambient "alive" feeling

Component Adjustment:
  - Rate: 10-15 particles in 200ms burst
  - Origin: All edges of component
  - Purpose: Energy release, state change

Continuous Operation:
  - Rate: 2-4 particles per second
  - Origin: Specific vents (e.g., active button)
  - Purpose: Shows component is powered/active
```

---

## ⚡ Aether Particle Behavior

### Cyan Energy Flow

**Data Stream Particles:**
```javascript
{
  particle: {
    shape: "dot",
    size: [2, 4],  // px
    color: "#00CED1",
    glow: 4,  // px blur radius
  },
  motion: {
    path: "bezier_curve",  // Follows circuit traces
    speed: 200,  // px/s constant
    easing: "linear",
  },
  spawn: {
    interval: 150,  // ms between particles
    count: 8,  // Total in sequence
    trigger: "value_change",
  },
  visual: {
    trail: true,
    trailLength: 8,  // px motion blur
    fadeIn: 50,  // ms
    fadeOut: 100,  // ms at end of path
  }
}
```

**Rising Aether Mist (In Sliders):**
```javascript
{
  particle: {
    shape: "wisp",
    size: [4, 12],  // px
    color: "#00CED1",
    opacity: 0.5,
    glow: 6,  // px
  },
  motion: {
    velocity: 40,  // px/s upward
    drift: [-10, 10],  // Slight horizontal waver
    turbulence: 0.2,
  },
  spawn: {
    rate: 3,  // particles per second
    origin: "filled_region",  // Only in filled area
  },
  lifespan: {
    duration: 2500,  // ms
    fade: "ease-out",
  }
}
```

---

## 🔥 Spark & Ember Effects

### Amber Sparks (Hot Components)

**Emission on Interaction:**
```javascript
{
  particle: {
    shape: "point",
    size: [1, 3],  // px
    color: "#FFBF00",  // Amber
    colorEnd: "#FF6347",  // Fades to orange-red
    glow: 6,  // Bright glow
  },
  motion: {
    velocity: [80, 150],  // px/s upward
    gravity: -20,  // Slight upward acceleration
    drift: [-40, 40],  // Random horizontal
  },
  spawn: {
    count: [5, 15],  // Per burst
    trigger: "knob_rotation",  // or button_press
    pattern: "radial",  // Explode outward
  },
  lifespan: {
    duration: [500, 1500],  // ms
    fade: "ease-out",
    scaleEnd: 0.3,  // Shrinks as it fades
  }
}
```

---

## ⚡ Electrical Arc (Lightning)

### Activation Spark

**Trigger:** Button press (off → on), Value snap to position

**Behavior:**
```javascript
{
  arc: {
    type: "branching_line",
    startPoint: [x1, y1],
    endPoint: [x2, y2],
    branches: [2, 5],  // Random count
    thickness: [1, 2],  // px
  },
  visual: {
    color: {
      start: "#FFBF00",  // Amber
      end: "#00CED1",  // Cyan
      gradient: true,
    },
    glow: 20,  // px intense bloom
    opacity: 1.0,
  },
  animation: {
    duration: 150,  // ms total
    phases: {
      flash: [0, 20],  // Instant appearance
      hold: [20, 80],  // Hold at peak
      fade: [80, 150],  // Quick fade out
    },
    easing: "linear",
  },
  path: {
    jaggedness: 0.7,  // How erratic the path is
    segments: [8, 16],  // Line complexity
    wobble: 2,  // px randomness
  }
}
```

---

## 🌟 Glow & Light Animations

### Breathing Pulse (Idle Components)

**Usage:** Value arcs, crystals, center hubs

```javascript
{
  glow: {
    opacityMin: 0.6,
    opacityMax: 0.85,
    blurMin: 10,  // px
    blurMax: 14,  // px
  },
  animation: {
    duration: 2500,  // ms per cycle
    easing: "sine",  // Smooth breathing
    loop: true,
  },
  trigger: "idle",  // When no user interaction
}
```

### Active Glow (Component Being Used)

```javascript
{
  glow: {
    opacity: 0.9,
    blur: 16,  // px
    colorShift: true,  // Slight hue rotation
  },
  particles: {
    emit: true,
    type: "ambient_sparkle",
    count: 2,  // particles per second
    size: 1,  // px
  }
}
```

---

## 🎵 BPM-Synchronized Effects

### Neural Pulse (At Tempo)

**When Playing at 120 BPM:**

**Every Beat (500ms):**
```
Center Hub:
  - Amber flash: 60% → 100% → 60%
  - Duration: 150ms
  - Easing: Ease-out

Energy Rings:
  - Concentric waves radiate from center
  - Speed: 400px/s
  - Opacity: 40% → 0%
  - Duration: 300ms
  - Color: Golden (#FFD700)

Steam Pulse:
  - Small burst from center
  - 3-5 white particles
  - Synchronized to downbeat

Orrery Gears:
  - Advance one notch per beat
  - Mechanical precision
  - Brass reflection highlight

Visual Metronome:
  - Entire interface subtly pulses
  - All glows slightly brighter on beat
  - Creates unified rhythm
```

---

## 🎨 Color State Transitions

### Component State Glow Colors

**Idle → Hover:**
```
Transition: 200ms ease-in
Glow Color: Same color, +30% opacity
Glow Radius: +2px
Additional: Faint steam wisps begin
```

**Hover → Active/Pressed:**
```
Transition: 100ms ease-out
Glow Color: Intensify +50% opacity
Glow Radius: +4px
Additional: Steam burst, amber sparks
```

**Off → On (Toggle):**
```
Transition: 300ms ease-out
Glow Color: Dark → Cyan full intensity
Includes: Electrical arc (150ms)
Steam: Major burst
Crystal: Illuminates from dark
```

**Modulated (By LFO/Automation):**
```
Behavior: Continuous pulsing at mod rate
Glow Color: Violet (#9370DB) overlay
Opacity: Follows modulation depth
Steam: Pulsing emission
```

---

## 📐 Performance Optimization

### Particle Budget

**Maximum Simultaneous Particles:**
```
Steam: 50 particles max
Aether: 30 particles max
Sparks: 20 particles max
Arcs: 3 effects max
Total Budget: ~100 active particles

If budget exceeded:
  - Oldest particles fade out early
  - New spawns are queued
  - Particle lifespan reduced 20%
```

### GPU Acceleration

**Use Hardware Rendering For:**
- Blur effects (shader-based)
- Particle systems (GPU compute)
- Glow effects (bloom shader)
- Rotation/transforms (GPU transform)

**Fallback for Low-End Systems:**
- Reduce particle counts by 50%
- Simplify blur (smaller radius)
- Disable ambient steam
- Reduce glow layers

---

**Animation Philosophy:**
*"The interface is a living brass engine - steam shows energy, aether carries data, and every interaction resonates through the sacred geometry of Victorian circuitry."*

---

**Last Updated:** 2025-10-17
**Version:** 2.0 - Gilded Steampunk
**Status:** Implementation Guide
**Frame Rate Target:** 60 FPS
