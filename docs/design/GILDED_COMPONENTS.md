# SYNAPTIK - Gilded Steampunk Components

**Detailed specifications for brass, steam, and glowing Victorian circuitry**

---

## 🎛️ Brass Knob (Rotary Control)

### Physical Design - Victorian Instrument Style

**Bezel (Outer Ring):**
```
Material: Aged brass with engraved markings
Diameter: 80px (medium), 60px (small), 100px (large)
Thickness: 8px visible edge
Pattern: Engraved tick marks (0-100 or custom scale)
Finish: Brushed brass with dark patina in recesses
Shadow: 4px drop shadow (60% black)
Highlight: Gold rim on top edge (2px, #FFD700)
```

**Knob Body:**
```
Material: Burnished copper or aged brass
Shape: Cylindrical with beveled top
Gradient: Light brass top → dark bronze bottom
Texture: Concentric grooves (lathe-turned appearance)
Center: Polished brass hub (20% of diameter)
Rivets: 6-12 tiny brass dots around perimeter
```

**Pointer/Indicator:**
```
Style: Engraved line OR brass pointer needle
Color: Dark bronze engraving OR polished gold needle
Length: 70% of radius
Glow: Cyan aether glow follows current value arc
Width: 3px for engraving, 2px needle
Tip: Small brass rivet or aether crystal
```

**Value Arc (Active Indicator):**
```
Path: From min position to current value
Stroke: 4px glowing cyan aether
Inner Ring: Subtle brass tick marks
Glow Effect: Cyan (#00CED1) at 70% opacity, 8px blur
Steam Effect: Subtle cyan mist rising from active arc
Animation: Pulse gently at 2-second intervals
```

**Center Hub:**
```
Design: Ornate brass medallion
Pattern: Sacred geometry symbol OR gear teeth
Material: Polished gold highlight
Size: 25% of knob diameter
Screw: Vintage flathead screw detail (optional)
Glow: When active, subtle amber glow from behind
```

### Steampunk Details

**Engraved Text:**
- Font: Serif, compressed (Victorian style)
- Color: Dark bronze (#665D1E)
- Size: 8-10px
- Position: Inside bezel ring
- Content: Parameter name (DEPTH, SWING, etc.)

**Steam Emission:**
- Source: Edges of knob when adjusted
- Color: White-cyan gradient
- Opacity: 30-50%
- Motion: Rising and dissipating
- Duration: 1-2 seconds after adjustment

**Aether Flow:**
- Type: Circuit traces from knob to connected parameters
- Color: Cyan (#00CED1)
- Animation: Flowing particles (1-2px) along path
- Speed: 0.5-1 second travel time
- Trigger: When value changes

---

## 💎 Brass Slider (Linear Control)

### Physical Design - Organ Pipe / Steam Gauge Style

**Track (Vertical Channel):**
```
Material: Dark teal panel with brass rails
Width: 12px
Height: 100-150px
Rails: Polished brass on both sides (2px each)
Background: Recessed dark teal (#1A2F3A)
Markings: Engraved hash marks every 10%
Shadow: Inner shadow (brass casts shadow on teal)
Border: Ornate brass border (Art Nouveau curves)
```

**Thumb (Slider Handle):**
```
Shape: Hexagonal crystal OR brass gear
Size: 20px wide, 16px tall
Material: Glowing aether crystal set in brass bezel
Center: Faceted cyan crystal (appears 3D)
Glow: 60% opacity cyan, 12px blur radius
Frame: Aged brass hexagonal setting
Details: 6 tiny brass rivets at corners
Shadow: Heavy drop shadow (simulates depth)
```

**Fill Indicator (Energy Level):**
```
From Bottom: Teal panel → cyan glow gradient
Gradient: #1A2F3A (0%) → #00CED1 (100%)
Particles: Rising cyan sparks in filled area
Steam: White mist at current value level
Animation: Pulsing brightness at 1.5s intervals
Overflow: Subtle steam "leak" at maximum
```

**Ornamental Frame:**
```
Top Cap: Brass finial with gear/crystal motif
Bottom Cap: Matching brass base
Side Filigree: Engraved circuit patterns
Material: Temple gold (#FFD700) for emphasis
Style: Art Nouveau organic + circuit board hybrid
Symmetry: Perfect left-right mirror
```

### Steam & Energy Effects

**Steam Vents (At Value Position):**
- Position: Small gaps in brass rails
- Emission: White steam puffs
- Frequency: Every 3-5 seconds (idle)
- Burst: Strong puff when value changes
- Direction: Perpendicular to slider

**Aether Particles (In Filled Region):**
- Type: Tiny glowing dots (1-2px)
- Color: Cyan (#00CED1)
- Behavior: Rising slowly, fade out
- Density: 5-10 particles visible at once
- Speed: 2-3 seconds to travel full height

---

## 🔘 Brass Button (Toggle/Momentary)

### Design - Mechanical Footswitch Style

**Button Cap:**
```
Shape: Rounded rectangle OR circular
Size: 60x30px (small), 80x40px (medium)
Material: Polished brass with beveled edges
Gradient: Light gold top → darker brass bottom
Engraving: Parameter name in Victorian font
Style: Convex (domed) surface
Highlight: Bright gold edge on top (2px)
Shadow: Progressive darkening toward bottom
```

**Bezel/Housing:**
```
Material: Dark bronze (#665D1E)
Depth: 6px visible frame
Pattern: Engraved geometric border
Rivets: 4 corner brass rivets
Recess: Button sits in recessed panel
Glow: When active, cyan backlight from recess
```

**States:**

**Idle (Off):**
- Cap: Aged brass, no glow
- Bezel: Dark bronze
- Text: Dark engraving
- Steam: None

**Hover:**
- Cap: Slight brightness increase (10%)
- Bezel: Subtle cyan outline (30% opacity)
- Steam: Faint wisps from edges
- Cursor: Changes to brass hand pointer

**Active (On):**
- Cap: Bright polished gold
- Bezel: Strong cyan glow (70% opacity, 8px blur)
- Steam: Active venting from all sides
- Crystal: Cyan aether crystal illuminated in center
- Particles: Amber sparks occasionally

**Pressed:**
- Cap: Darker (mechanical depression)
- Position: Translate Y +2px (pressed down)
- Steam: Burst emission on press
- Sound: Mechanical "click" (if audio enabled)
- Glow: Flash bright then sustain

### Indicator Crystal (Optional):**
```
Position: Center of button cap
Type: Small hexagonal crystal (6px)
Color (Off): Dark gray
Color (On): Glowing cyan OR amber
Facets: 3D appearance with highlights
Bezel: Tiny brass setting around crystal
Glow: Radiates outward when active
```

---

## 📺 Waveform Display (Oscilloscope Style)

### Victorian Scientific Instrument Aesthetic

**Frame:**
```
Material: Ornate brass frame (Art Deco)
Style: Rectangular with chamfered corners
Width: 200-400px
Height: 80-120px
Border: 8px brass frame with engraved pattern
Corner Details: Decorative brass medallions
Shadow: Heavy drop shadow (depth illusion)
```

**Display Screen:**
```
Background: Deep teal-black gradient (#0A1628 → #1A2F3A)
Texture: Subtle scan lines (CRT monitor feel)
Border: 2px dark bronze inset
Material: Appears as thick glass/crystal
Reflection: Subtle highlight on "glass"
Vignette: Darker at edges, lighter at center
```

**Waveform:**
```
Color: Bright cyan (#00CED1) OR amber (#FFBF00)
Style: Continuous line, 2-3px stroke
Glow: Strong glow (80% opacity, 6px blur)
Phosphor Trail: Fading tail effect (persistence)
Grid: Subtle brass grid lines (10% opacity)
Scan Line: Moving horizontal line (optional)
Particles: Cyan sparkles along waveform peaks
```

**Measurement Markings:**
```
Grid: Brass-colored division lines
Labels: Victorian numerals (I, II, III, IV, etc.)
Font: Serif, small (8px)
Color: Dark gold (#B8860B)
Position: Along edges of display
Style: Engraved appearance
```

**Indicators/LEDs:**
```
Type: Small brass-bezeled crystals
Colors: Green (safe), Amber (warn), Red (peak)
Position: Top or bottom of frame
Behavior: Light up based on signal level
Glow: Appropriate color with soft glow
Housing: Tiny brass settings
```

---

## ⚙️ Radial Sequencer (Central Pattern Display)

### Brass Orrery / Astrolabe Design

**Outer Frame:**
```
Style: Ornate brass orrery rings
Diameter: 300-400px
Rings: 3-5 concentric brass circles
Pattern: Engraved zodiac OR musical symbols
Material: Temple gold (#FFD700) primary
Accents: Aged brass (#B8860B) secondary
Rivets: Brass studs at key positions (12, 16, or 32)
Gears: Small decorative gears between rings
Steam Vents: Tiny holes emit steam periodically
```

**Inner Display (Pattern Area):**
```
Background: Deep navy (#0A1628) or dark teal
Steps: Radial segments (8, 16, 32, or 64)
Active Cells: Glowing cyan aether crystals
Inactive Cells: Dark brass outlines
Current Step: Bright amber/gold highlight
Connecting Lines: Golden circuit traces between steps
Center: Large ornate brass hub with sacred symbol
Rotation: Entire display rotates OR playhead rotates
```

**Step Indicators (Cells):**
```
Shape: Hexagonal crystals OR circular gems
Active: Glowing cyan crystal (#00CED1)
Inactive: Dark teal with brass outline
Current: Pulsing amber orb (#FFBF00)
Size: 8-12px per cell
Spacing: 2-4px gap between cells
Bezel: Each cell has tiny brass setting
Glow: Active cells have 60% glow, 10px blur
```

**Center Hub (Neural Core):**
```
Design: Ornate brass mandala
Size: 60-80px diameter
Layers:
  - Outer ring: Rotating brass gears
  - Mid ring: Pulsing cyan aether
  - Inner core: Glowing amber orb
Animation: Pulses at BPM rate
Steam: Rises from center continuously
Particles: Amber sparks emanate outward
Sacred Symbol: Engraved geometry (flower of life, etc.)
```

**Energy Flow Paths:**
```
Type: Golden circuit traces radiating from center
Color: Temple gold (#FFD700) with cyan glow
Animation: Aether particles flow outward when triggered
Speed: 0.5-1 second from center to edge
Branching: Paths split to multiple steps
Glow: Cyan glow follows particle movement
Steam: Cyan mist flows along paths
```

**Playhead/Cursor:**
```
Style: Ornate brass pointer like clock hand
Material: Polished gold
Length: Full radius
Width: 3px at base, 1px at tip
Glow: Amber glow trail (motion blur)
Tip: Small amber crystal
Rotation: Smooth 360° per pattern cycle
Shadow: Brass casts shadow on display
```

---

## 🎚️ Dropdown Menu (Brass ComboBox)

### Victorian Nameplate Style

**Closed State:**
```
Background: Dark teal panel (#1A2F3A)
Border: Brass frame (3px, aged brass)
Height: 32-36px
Width: 120-200px (auto-fit text)
Corners: Slightly rounded (4px) or beveled
Text: Temple gold (#FFD700) engraved style
Font: Victorian serif, 12-14px
Icon: Brass chevron (down arrow)
Shadow: Inset shadow (recessed plate)
Highlight: Gold rim on top edge
```

**Engraved Label:**
```
Style: Appears carved into brass plate
Color: Dark bronze (#665D1E) for shadows
Highlight: Light gold (#FFD700) on raised edges
Effect: Bevel/emboss to simulate engraving
Font: All caps, letter-spaced
Example: "E U C L I D E A N"
```

**Dropdown Icon (Chevron):**
```
Shape: V pointing down
Material: Polished brass
Size: 10x8px
Stroke: 2px
Color: Temple gold (#FFD700)
Position: Right side, 8px from edge
Animation: Rotates 180° when opened
Glow: Subtle cyan glow on hover
```

**Opened Panel:**
```
Background: Darker teal (#152430)
Border: Glowing cyan outline (2px, 60% opacity)
Shadow: Heavy drop shadow (8px blur, 80% black)
Backdrop: Brass decorative frame extends
Steam: White steam wisps from edges
Items: List of options
Max Height: 200px with scroll if needed
```

**Menu Items:**
```
Height: 30px each
Padding: 8px horizontal
Text: Light gold (#DAA520)
Hover BG: Cyan glow (20% opacity)
Selected BG: Cyan glow (40% opacity)
Border: Subtle brass divider between items
Icon: Small brass bullet or symbol
Font: Same as closed state
Transition: Fade in/out (200ms)
```

---

## 📊 Gothic Panel (Container/Section)

### Cathedral Architecture Style

**Frame Design:**
```
Style: Gothic pointed arch OR Art Deco rectangle
Material: Ornate brass filigree
Pattern: Circuit board traces as decorative elements
Thickness: 6-12px border
Corners: Brass medallions OR Gothic finials
Shadow: Multiple layers of depth
Rivets: Decorative brass studs at key points
Engraving: Sacred geometry or musical symbols
```

**Panel Background:**
```
Base: Dark teal gradient (#1A2F3A → #0F1F2A)
Texture: Subtle brushed metal texture
Inset: Recessed from frame (6-8px depth)
Vignette: Darker edges, lighter center
Pattern: Very subtle circuit traces (5% opacity)
Reflection: Faint highlights suggesting glass/lacquer
```

**Header Section:**
```
Position: Top of panel, integrated with frame
Background: Darker bronze (#665D1E)
Height: 32-40px
Text: Temple gold all-caps title
Symbol: Decorative brass icon on left
Style: Nameplate attached to frame
Shadow: Casts shadow on panel below
Glow: Subtle cyan backlight when active
```

**Content Area:**
```
Padding: 12-20px all sides
Layout: Grid or vertical stack of controls
Spacing: 12px between elements
Dividers: Brass lines (1px) if needed
Depth: Controls appear to sit in recessed panel
Steam: Subtle white wisps from top corners
```

**Decorative Elements:**
```
Corner Flourishes: Art Nouveau brass curves
Side Columns: Miniature brass pilasters (optional)
Top Finial: Ornate brass crown or gear
Bottom Base: Brass foundation plate
Pipes: Decorative brass tubes (non-functional)
Crystals: Small cyan gems at key positions
Steam Vents: Functional detail releasing steam
```

---

## ⚡ Steam & Particle Effects Specifications

### Rising Steam (White/Neutral)
```
Particle System:
  - Color: White (#FFFFFF) → Transparent
  - Size: 20-80px blobs (varies)
  - Opacity: 60% → 0% (fade out)
  - Blur: 30-60px (very soft)
  - Speed: 50-100px/second (slow rise)
  - Lifespan: 2-4 seconds
  - Motion: Turbulent (perlin noise)
  - Spawn Rate: 1-3 particles/second
  - Origin: Component edges, vents
```

### Cyan Aether Mist
```
Particle System:
  - Color: #00CED1 at 40% opacity
  - Size: 10-40px wisps
  - Glow: Soft glow (10px blur)
  - Speed: 30-80px/second
  - Motion: Follows circuit paths
  - Lifespan: 1-3 seconds
  - Spawn: When data flows
  - Behavior: Snake along traces
```

### Amber Sparks/Embers
```
Particle System:
  - Color: #FFBF00 → #FF6347 (gradient)
  - Size: 1-3px points
  - Glow: Bright core (100% → 0%)
  - Speed: 80-150px/second (rising)
  - Motion: Upward with drift
  - Lifespan: 0.5-1.5 seconds
  - Spawn Rate: 5-15 particles/second
  - Origin: Hot components (active knobs)
  - Fade: Quick fade at end of life
```

### Data Stream Particles
```
Particle System:
  - Color: #00CED1 solid
  - Size: 2-4px dots
  - Speed: 200-400px/second (fast)
  - Motion: Linear along paths
  - Lifespan: Path length / speed
  - Trail: Motion blur (4-8px)
  - Pattern: Morse code-like pulses
  - Spawn: When value changes
  - Glow: Each particle glows (4px)
```

### Electrical Arc (Lightning)
```
Effect:
  - Type: Branching line
  - Color: #FFBF00 → #00CED1 gradient
  - Duration: 100-200ms flash
  - Branches: 2-5 random branches
  - Thickness: 1-2px
  - Glow: Intense bloom (20px)
  - Trigger: Button press, value snap
  - Path: Random jagged line
  - Fade: Instant on, quick fade out
```

---

## 🎨 Animation Timing

### Steampunk Timing Guidelines

**Mechanical Movement (Knobs, Sliders):**
- Rotation/Slide: 150-300ms
- Easing: Cubic-bezier(0.4, 0.0, 0.2, 1) - mechanical smoothness
- Overshoot: None (precise mechanical)
- Steam Burst: Triggered at end of movement

**Steam Effects:**
- Rise Speed: 50-100px/second
- Fade Duration: 2-4 seconds
- Turbulence: Perlin noise, subtle
- Spawn Delay: 50-200ms between particles

**Glow Pulse (Idle):**
- Duration: 2-3 seconds per cycle
- Intensity: 60% → 80% → 60%
- Easing: Sine wave (smooth breathing)

**Electrical Spark:**
- Flash On: Instant (0ms)
- Hold: 50-100ms
- Fade Out: 100-200ms
- Easing: Linear fade

**Data Flow:**
- Travel Time: 500-1000ms per path
- Particle Speed: Constant
- Spawn Interval: 100-300ms between particles

---

**Design Philosophy:**
*"Every control is a precision instrument from a Victorian inventor's workshop. Brass speaks of craftsmanship, steam carries energy, and aether flows through circuits inscribed with sacred geometry."*

---

**Last Updated:** 2025-10-17
**Version:** 2.0 - Gilded Steampunk
**Status:** Implementation Guide
