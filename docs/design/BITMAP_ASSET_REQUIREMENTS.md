# Bitmap Asset Requirements for Full Image-Based UI

## Overview
This document specifies all bitmap assets needed for a complete image-based UI with no procedural drawing. All elements should be cutout PNG files with proper alpha channels.

## Asset Categories

### 1. Background Panels (Full-Size Textures)
**Purpose**: Main background for each section

| Asset Name | Size | Description | Alpha Channel |
|------------|------|-------------|---------------|
| `panel_main_background.png` | 1400x750 | Full plugin background - aged brass with verdigris | No (opaque) |
| `panel_section_dark.png` | variable | Dark steel/brass panel for control sections | Semi-transparent |
| `panel_section_light.png` | variable | Light brass panel variant | Semi-transparent |

**Current Status**: ✓ Have `panelVerdigris` from SynaptikUIToolkit
**Action**: Use existing, tile/stretch as needed

---

### 2. Section Frames (Borders & Bezels)
**Purpose**: Ornate frames around PATTERN DISPLAY, GENERATOR, EXPRESSION, ADVANCED sections

| Asset Name | Size | Description | Alpha Channel |
|------------|------|-------------|---------------|
| `frame_section_horizontal.png` | 1350x200 | Art Deco horizontal frame with corners | Yes (cutout) |
| `frame_corner_tl.png` | 64x64 | Top-left corner ornament | Yes (cutout) |
| `frame_corner_tr.png` | 64x64 | Top-right corner ornament | Yes (cutout) |
| `frame_corner_bl.png` | 64x64 | Bottom-left corner ornament | Yes (cutout) |
| `frame_corner_br.png` | 64x64 | Bottom-right corner ornament | Yes (cutout) |

**Current Status**: ✓ Have `frameArtDeco` from SynaptikUIToolkit
**Action**: Use existing frame, create corner variants by cropping/mirroring

---

### 3. Title Plates (Section Labels)
**Purpose**: Brass nameplates for PATTERN DISPLAY, GENERATOR, etc.

| Asset Name | Size | Description | Alpha Channel |
|------------|------|-------------|---------------|
| `titleplate_pattern.png` | 300x40 | "PATTERN DISPLAY" engraved brass plate | Yes (cutout) |
| `titleplate_generator.png` | 200x40 | "GENERATOR" engraved brass plate | Yes (cutout) |
| `titleplate_expression.png` | 200x40 | "EXPRESSION" engraved brass plate | Yes (cutout) |
| `titleplate_advanced.png` | 200x40 | "ADVANCED" engraved brass plate | Yes (cutout) |

**Current Status**: ✓ Have `labelBrassPlate` from SynaptikUIToolkit
**Action**: Use as base, overlay text or create variants

---

### 4. Knobs (Rotary Controls)
**Purpose**: Victorian brass dials for tempo, steps, pulses, swing, etc.

| Asset Name | Frames | Description | Alpha Channel |
|------------|--------|-------------|---------------|
| `knob_ornate_filmstrip.png` | 64 frames | Ornate filigree knob, rotates 0-360° | Yes (round cutout) |
| `knob_concentric_filmstrip.png` | 64 frames | Concentric ring knob | Yes (round cutout) |
| `knob_simple_filmstrip.png` | 64 frames | Simple brass dial | Yes (round cutout) |

**Current Status**: ✓ Have static knob images (img_8150, img_8183, img_8200)
**Action**: Create 64-frame filmstrips by rotating each knob image, or use static + procedural pointer

**Alternative Approach**: Use static knob + overlay rotating pointer image

---

### 5. Sliders (Vertical & Horizontal)
**Purpose**: Velocity range, pitch range controls

| Asset Name | Size | Description | Alpha Channel |
|------------|------|-------------|---------------|
| `slider_rail_vertical.png` | 60x150 | Brass vertical slider rail | Yes (cutout) |
| `slider_thumb_aether.png` | 60x40 | Aether staff slider thumb | Yes (cutout) |
| `slider_rail_horizontal.png` | 150x60 | Brass horizontal slider rail | Yes (cutout) |

**Current Status**: ✓ Have `sliderVerticalRail` and `sliderPointer` from SynaptikUIToolkit
**Action**: Use existing, create horizontal variant if needed

---

### 6. Buttons & Toggles
**Purpose**: Generator selector, scale selector, legato, etc.

| Asset Name | Size | Description | Alpha Channel |
|------------|------|-------------|---------------|
| `button_brass_off.png` | 100x40 | Brass button unpressed state | Yes (cutout) |
| `button_brass_on.png` | 100x40 | Brass button pressed/active state | Yes (cutout) |
| `toggle_brass_off.png` | 60x30 | Toggle switch OFF position | Yes (cutout) |
| `toggle_brass_on.png` | 60x30 | Toggle switch ON position | Yes (cutout) |

**Current Status**: ✓ Have `buttonOrnate` from SynaptikUIToolkit
**Action**: Create ON/OFF states by layering glow effects

---

### 7. Combo Boxes (Dropdown Selectors)
**Purpose**: Generator type, MIDI channel, scale root, scale type

| Asset Name | Size | Description | Alpha Channel |
|------------|------|-------------|---------------|
| `combobox_brass_closed.png` | 150x30 | Dropdown in closed state | Yes (cutout) |
| `combobox_brass_open.png` | 150x30 | Dropdown in open state | Yes (cutout) |
| `combobox_arrow_down.png` | 20x20 | Brass chevron pointing down | Yes (cutout) |
| `combobox_item_normal.png` | 150x25 | Menu item normal state | Semi-transparent |
| `combobox_item_hover.png` | 150x25 | Menu item hover state | Semi-transparent |

**Current Status**: ✗ Need to create
**Action**: Design brass dropdown components

---

### 8. Pattern Visualizer (Sequencer Display)
**Purpose**: Crystal LEDs showing Euclidean pattern

| Asset Name | Size | Description | Alpha Channel |
|------------|------|-------------|---------------|
| `led_off.png` | 40x40 | Inactive step - dark brass socket | Yes (cutout) |
| `led_active_gold.png` | 40x40 | Active step - golden crystal glow | Yes (cutout with glow) |
| `led_active_green.png` | 40x40 | Active step - verdigris crystal | Yes (cutout with glow) |
| `led_active_violet.png` | 40x40 | Active step - violet crystal | Yes (cutout with glow) |
| `led_current.png` | 40x40 | Current playback step - bright amber | Yes (cutout with glow) |
| `visualizer_frame.png` | 1350x100 | Brass frame around LED array | Yes (cutout) |

**Current Status**: ✗ Need to create
**Action**: Design crystal LED components

---

### 9. Decorative Elements
**Purpose**: Corner ornaments, dividers, flourishes

| Asset Name | Size | Description | Alpha Channel |
|------------|------|-------------|---------------|
| `ornament_corner_01.png` | 64x64 | Victorian corner flourish (4 rotations) | Yes (cutout) |
| `divider_horizontal.png` | 400x20 | Brass horizontal divider | Yes (cutout) |
| `divider_vertical.png` | 20x200 | Brass vertical divider | Yes (cutout) |
| `gear_decoration_01.png` | 40x40 | Small brass gear | Yes (cutout) |
| `rivet_brass.png` | 10x10 | Brass rivet/bolt | Yes (cutout) |

**Current Status**: ✓ Have `cornerOrnament` from SynaptikUIToolkit
**Action**: Use existing decorative assets

---

### 10. Text Labels & Values
**Purpose**: Parameter names and numeric displays

| Asset Name | Size | Description | Alpha Channel |
|------------|------|-------------|---------------|
| `textbox_value_brass.png` | 80x20 | Brass plate for numeric values | Yes (cutout) |
| `label_background.png` | variable | Engraved brass label background | Yes (cutout) |

**Current Status**: ✓ Have `labelBrassPlate` from SynaptikUIToolkit
**Action**: Use existing label backgrounds

---

## Implementation Priority

### Phase 1: Core Layout (Use Existing Assets)
1. Background: Use `panelVerdigris` ✓
2. Section frames: Use `frameArtDeco` ✓
3. Knobs: Use existing static knobs with pointer overlay ✓
4. Sliders: Use `sliderVerticalRail` + `sliderPointer` ✓

### Phase 2: Missing Critical Elements (Generate/Create)
1. LED components for pattern visualizer
2. Combo box brass components
3. Button ON/OFF states
4. Title plate variants

### Phase 3: Polish (Optional Enhancements)
1. Animated knob filmstrips (64 frames each)
2. Additional decorative flourishes
3. Glow/highlight overlays for active states

---

## Asset Generation Notes

### For AI Image Generation
When generating missing assets externally:

1. **Style**: Victorian steampunk, aged brass, verdigris patina
2. **Colors**: Gold (#FFD700), aged brass (#B8860B), verdigris (#50C878), aether cyan (#00CED1)
3. **Details**: Engravings, gears, rivets, Art Deco geometric patterns
4. **Alpha**: All cutout elements need transparent backgrounds
5. **Resolution**: Minimum 2x size for Retina displays (e.g., 128x128 for 64x64 display size)

### Extraction from SynaptikUIToolkit
You already have 536+ decorative elements! Many required assets may already exist:
- Check `/decorative/` for ornaments
- Check `/frames/` for panel borders
- Check `/buttons/` for toggle states
- Check `/panels/` for backgrounds

---

## Next Steps

1. **Audit existing SynaptikUIToolkit**: Find all usable assets
2. **Create missing asset list**: Identify gaps
3. **Generate critical assets**: Focus on LEDs and combo boxes first
4. **Implement BitmapUIComponent**: Build the layering system
5. **Replace procedural drawing**: Swap all gradients/shapes with bitmaps

