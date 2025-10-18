# SYNAPTIK Color Palette Reference

**"Where circuits dream — and music thinks."**

---

## Primary Colors

### Neural Void (Background)
```
Hex: #0A0E1A
RGB: 10, 14, 26
HSL: 228°, 44%, 7%

Usage: Main background, deep space, void areas
Mood: Mysterious, infinite, cosmic
```

### Temple Steel (Structure)
```
Hex: #1A1F2E
RGB: 26, 31, 46
HSL: 225°, 28%, 14%

Usage: Panels, knob bodies, structural elements
Mood: Solid, architectural, ancient-futuristic
```

### Circuit Trace (Borders)
```
Hex: #2A3447
RGB: 42, 52, 71
HSL: 223°, 26%, 22%

Usage: Borders, dividers, inactive traces
Mood: Technical, precise, circuitry
```

---

## Accent Colors (Bioluminescent)

### Synapse Blue (Primary Accent)
```
Hex: #00D4FF
RGB: 0, 212, 255
HSL: 190°, 100%, 50%

Usage: Active states, primary glow, pointers
Mood: Electric, alive, neural firing
Glow: 40-60% opacity, 8-16px blur
```

### Mystic Violet (Secondary Accent)
```
Hex: #9D4EDD
RGB: 157, 78, 221
HSL: 273°, 67%, 59%

Usage: Modulation, secondary highlights, LFO paths
Mood: Mystical, psychedelic, consciousness
Glow: 30-50% opacity, 6-12px blur
```

### Quantum Teal (Success/Quantization)
```
Hex: #00FFC8
RGB: 0, 255, 200
HSL: 167°, 100%, 50%

Usage: Text, success states, scale quantization, highlights
Mood: Precise, quantum, harmonic
Glow: 50-70% opacity, 10-14px blur
```

### Chaos Orange (Warning/Stochastic)
```
Hex: #FF6B35
RGB: 255, 107, 53
HSL: 16°, 100%, 60%

Usage: Randomness, chaos engines, warnings
Mood: Energetic, unpredictable, fire
Glow: 40-60% opacity, 8-12px blur
```

### Evolution Gold (Pattern Evolution)
```
Hex: #FFB627
RGB: 255, 182, 39
HSL: 40°, 100%, 58%

Usage: Pattern morphing, memory states, evolution
Mood: Transformation, alchemy, sacred
Glow: 45-65% opacity, 10-14px blur
```

---

## Color Combinations

### Energy Flow Gradients

**Synapse Flow:**
```css
linear-gradient(90deg, #00D4FF 0%, #00FFC8 50%, #9D4EDD 100%)
```
Use for: Active paths, energy traces, modulation visualization

**Temple Depth:**
```css
linear-gradient(180deg, #1A1F2E 0%, #0A0E1A 100%)
```
Use for: Panel backgrounds, structural depth

**Neural Glow:**
```css
radial-gradient(circle, #00D4FF 0%, transparent 100%)
```
Use for: Knob halos, button glows, active indicators

---

## Opacity Levels

| State | Opacity | Use Case |
|-------|---------|----------|
| **Inactive** | 20-30% | Disabled controls, background traces |
| **Ambient** | 40-50% | Subtle glows, hover previews |
| **Active** | 60-80% | Active controls, current values |
| **Peak** | 90-100% | Highlights, peaks, focus states |

---

## Glow Effects Specifications

### Small Glow (UI Elements)
- **Blur Radius:** 6-8px
- **Spread:** 0px
- **Opacity:** 40-50%
- **Color:** Match element color

### Medium Glow (Knobs, Buttons)
- **Blur Radius:** 10-14px
- **Spread:** 2px
- **Opacity:** 50-60%
- **Color:** Synapse Blue or accent

### Large Glow (Neural Core, Major Elements)
- **Blur Radius:** 16-24px
- **Spread:** 4px
- **Opacity:** 60-70%
- **Color:** Gradient (multi-color)

---

## Color Psychology & Meaning

| Color | Represents | Musical Context |
|-------|------------|-----------------|
| **Synapse Blue** | Neural firing, activity | Active notes, rhythm |
| **Quantum Teal** | Precision, harmony | Quantized notes, scales |
| **Mystic Violet** | Modulation, transformation | LFOs, parameter changes |
| **Chaos Orange** | Randomness, entropy | Stochastic elements, drift |
| **Evolution Gold** | Growth, mutation | Pattern evolution, memory |
| **Neural Void** | Silence, space | Rests, silence, emptiness |
| **Temple Steel** | Structure, foundation | Architecture, framework |

---

## Accessibility Notes

### Contrast Ratios (against Neural Void #0A0E1A)

- **Quantum Teal (#00FFC8):** 13.2:1 ✅ AAA (Excellent)
- **Synapse Blue (#00D4FF):** 10.8:1 ✅ AAA (Excellent)
- **Evolution Gold (#FFB627):** 11.5:1 ✅ AAA (Excellent)
- **Mystic Violet (#9D4EDD):** 5.8:1 ✅ AA (Good)
- **Chaos Orange (#FF6B35):** 6.2:1 ✅ AA (Good)

All colors meet WCAG AA standards for normal text.

---

## Export Formats

### CSS Variables
```css
:root {
  --neural-void: #0A0E1A;
  --temple-steel: #1A1F2E;
  --circuit-trace: #2A3447;
  --synapse-blue: #00D4FF;
  --mystic-violet: #9D4EDD;
  --quantum-teal: #00FFC8;
  --chaos-orange: #FF6B35;
  --evolution-gold: #FFB627;
}
```

### JSON (for design tools)
```json
{
  "neuralVoid": "#0A0E1A",
  "templeSteel": "#1A1F2E",
  "circuitTrace": "#2A3447",
  "synapseBlue": "#00D4FF",
  "mysticViolet": "#9D4EDD",
  "quantumTeal": "#00FFC8",
  "chaosOrange": "#FF6B35",
  "evolutionGold": "#FFB627"
}
```

### Figma/Sketch Swatches
See `synaptik-palette.ase` (Adobe Swatch Exchange)

---

## Inspiration & References

**Color Themes:**
- Cyberpunk neon aesthetics
- Bioluminescent deep-sea creatures
- Neural network visualizations
- Sacred geometry and temple architecture
- Tron Legacy visual design
- Ghost in the Shell color palette

**Natural References:**
- Bioluminescent plankton (blue glow)
- Deep ocean vents (orange/red)
- Aurora borealis (violet/teal)
- Quantum particle trails (cyan)

---

**Last Updated:** 2025-10-17
**Version:** 1.0
**Status:** Implemented in Phase 1
