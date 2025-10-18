# GenerativeMIDI - Asset Catalog

**Created**: 2025-10-18
**Total Assets**: 147 high-resolution JPG images
**Location**: `conceptArt/assets/`
**Style**: Victorian steampunk, gilded brass, verdigris patina, Art Deco

---

## Asset Collection Overview

This catalog documents 147 AI-generated steampunk assets suitable for UI integration into the GenerativeMIDI plugin. All assets follow the gilded steampunk aesthetic with brass, copper, gold, and verdigris patina elements.

### Asset Categories (By Type)

Based on visual analysis of the collection, assets fall into these categories:

1. **Rotary Knobs & Dials** (~30 assets)
   - Ornate brass knobs with filigree
   - Astronomical clock faces
   - Concentric ring designs
   - Gear-based controls
   - Victorian instrument knobs

2. **Buttons & Switches** (~15 assets)
   - Toggle switches
   - Push buttons
   - Ornamental crosses/markers
   - Decorative caps

3. **Frames & Borders** (~25 assets)
   - Ornate rectangular frames
   - Circular bezels
   - Corner brackets
   - Art Deco borders
   - Panel edges with rivets

4. **Sliders & Linear Controls** (~10 assets)
   - Vertical sliders with brass rails
   - Steampunk thermometer-style displays
   - Linear faders with verdigris

5. **Displays & Windows** (~15 assets)
   - Glass display areas
   - Numerical displays
   - Brass-framed screens
   - Pattern visualizer frames

6. **Decorative Elements** (~20 assets)
   - Gears and cogs
   - Filigree ornaments
   - Corner embellishments
   - Brass rivets and bolts
   - Floral/organic motifs

7. **Complete Panels** (~10 assets)
   - Full interface mockups
   - Control surface assemblies
   - Reference designs

8. **Pointers & Indicators** (~8 assets)
   - Needle indicators
   - Brass pointers
   - Aether crystal staffs
   - Ornate arrows

9. **Texture Elements** (~10 assets)
   - Brass plate textures
   - Verdigris patina
   - Honeycomb patterns
   - Engraved surfaces

10. **Pre-Isolated Elements** (~4 assets)
    - Already cut out with transparency
    - Ready to drop into UI

---

## Priority Assets for Immediate Use

### 🏆 TIER 1: Ready-to-Use (Already Isolated)

**IMG_8148.JPG** - Pre-isolated brass panel with golden frame
- **Use For**: Label plates, parameter displays, section headers
- **Status**: ✅ Already has transparency
- **Dimensions**: Rectangular, ~600×200px
- **Integration**: Drop-in ready

### 🥇 TIER 2: High-Priority Knobs (Excellent for Rotary Controls)

**IMG_8150.JPG** - Ornate circular frame with filigree
- **Use For**: Main knobs (tempo, steps, pulses, etc.)
- **Isolation**: Easy (clean dark background)
- **Center**: Glass/crystal center for value display
- **Rings**: 4 concentric brass rings with ornate decorations

**IMG_8183.JPG** - Concentric brass rings (astrolabe style)
- **Use For**: Primary rotary controls, tempo knob
- **Isolation**: Very easy (black background, high contrast)
- **Detail Level**: Extremely detailed engraving
- **Center**: Perfect for numeric value display

**IMG_8200.JPG** - Astronomical clock face
- **Use For**: Tempo display, pattern visualizer background
- **Isolation**: Easy (black background)
- **Features**: Numbers, tick marks, concentric scales
- **Aesthetic**: Perfect match for current UI

### 🥈 TIER 3: Decorative Frames & Bezels

**IMG_8125.JPG** - Art Deco panel with corner screws
- **Use For**: Section panel borders, combo box frames
- **Isolation**: Moderate (turquoise background needs masking)
- **Features**: Greek key pattern, brass corner screws, number wheels
- **Center Area**: Good for parameter displays

**IMG_8135.JPG** - Ornate oval frame
- **Use For**: Pattern visualizer frame, large display area
- **Isolation**: Easy (black background)
- **Features**: Victorian filigree, multiple layers
- **Center**: Large empty area perfect for content

**IMG_8243.JPG** - Ornate rosette with layers
- **Use For**: Large central display element, pattern visualizer
- **Isolation**: Easy (neutral background)
- **Features**: Multiple concentric layers, floral motifs, brass knobs
- **Colors**: Steel blue, brass, copper, rose accents

### 🥉 TIER 4: Sliders & Linear Controls

**IMG_8250.JPG** - Steampunk vertical slider
- **Use For**: Velocity/pitch range sliders
- **Isolation**: Easy (neutral background)
- **Features**: Brass pipes, verdigris patina, measurement scales
- **Style**: Perfect match for current aesthetic

**IMG_8180.JPG** - Ornate brass pointer/staff
- **Use For**: Slider thumb, knob pointer indicator
- **Isolation**: Very easy (neutral background)
- **Features**: Aether crystal top, brass filigree, perfect for pointers
- **Size**: Tall vertical element

### 🎖️ TIER 5: Decorative & Accent Elements

**IMG_8138.JPG** - Steampunk panel with gears
- **Use For**: Background texture, inspiration for layouts
- **Isolation**: Moderate (needs gear extraction)
- **Features**: Multiple gears, corner brackets, ornate bezels
- **Use**: Extract individual gears and ornaments

**IMG_8168.JPG** - Brass decorative cross
- **Use For**: Button caps, toggle indicators
- **Isolation**: Very easy (neutral background)
- **Features**: Ornate filigree, brass with dark accents
- **Size**: Medium, perfect for button overlays

**IMG_8213.JPG** - Brass honeycomb grille
- **Use For**: Texture overlay, speaker grille effect, pattern backgrounds
- **Isolation**: Easy (neutral background)
- **Features**: Hexagonal pattern, riveted brass frame
- **Use**: Overlay texture for depth

### 🎨 TIER 6: Reference Panels (Full Interfaces)

**IMG_8119.JPG** - SYNAPTIK complete panel
- **Use For**: Layout reference, inspiration
- **Isolation**: Complex (many individual elements)
- **Features**: Complete steampunk control panel with multiple knobs, displays
- **Use**: Reference for spatial layout and design language

**IMG_8120.JPG** - SYNNATIKS front panel
- **Use For**: Layout reference, color palette
- **Isolation**: Complex
- **Features**: Central circular display, side parameter screens, bottom knobs
- **Use**: Inspiration for three-section layout

---

## Asset Extraction Requirements

### Tools Needed
- **Photoshop/GIMP**: For complex masking and color selection
- **ImageMagick**: For batch processing and format conversion
- **Python/PIL**: For automated batch processing
- **JUCE**: Image class for runtime rendering

### Extraction Process

#### Method 1: Simple Background Removal (Black/Neutral Backgrounds)
```bash
# Using ImageMagick for simple background removal
convert IMG_8150.JPG -fuzz 15% -transparent black knob_ornate_01.png

# For white/neutral backgrounds
convert IMG_8168.JPG -fuzz 10% -transparent white button_cross_01.png
```

#### Method 2: Layer-Based Extraction (Complex Elements)
1. Open in Photoshop/GIMP
2. Use Magic Wand with tolerance 15-25
3. Create layer mask
4. Refine edge with 1-2px feather
5. Export as PNG with transparency

#### Method 3: Multi-Element Extraction (Gears, Ornaments)
1. Isolate each element separately
2. Save as numbered sequence (gear_01.png, gear_02.png, etc.)
3. Maintain original lighting and shadows
4. Export at multiple sizes (64px, 128px, 256px)

### Output Specifications

**Format**: PNG with alpha channel
**Bit Depth**: 32-bit RGBA
**Resolution**: Multiple sizes per asset
- Small: 64×64px (buttons, small icons)
- Medium: 128×128px (knobs, controls)
- Large: 256×256px (displays, frames)
- XLarge: 512×512px (main display elements)

**Naming Convention**:
```
element-type_style_variant_size.png

Examples:
knob_ornate_filigree_128.png
frame_oval_victorian_256.png
slider_brass_vertical_128.png
button_cross_ornamental_64.png
```

---

## Color Palette Extraction

### Primary Colors (From Assets)
```
Brass Aged:      #B8860B (Dark Goldenrod)
Gold Temple:     #FFD700 (Gold)
Copper Steam:    #B87333 (Copper)
Bronze Gothic:   #614E1A (Dark Bronze)
Steel Obsidian:  #2C3539 (Dark Steel Gray)
Aether Cyan:     #00CED1 (Dark Turquoise)
Verdigris Green: #43B3AE (Teal)
Patina Blue:     #3A6B7C (Steel Blue)
Rose Accent:     #CC6666 (Dusty Rose)
Amber Tesla:     #FFBF00 (Amber)
```

### Texture Overlays Needed
1. **Brass Scratches**: Light wear marks
2. **Verdigris Oxidation**: Teal-green patina
3. **Dust & Grime**: Subtle aging
4. **Polish Highlights**: Reflective spots
5. **Shadow Maps**: For depth

---

## Integration Priority Matrix

### Phase 1: Core Controls (Week 1)
**Priority: CRITICAL**

| Asset | Use | Difficulty | Impact |
|-------|-----|------------|--------|
| IMG_8150 | Rotary knobs | Easy | HIGH |
| IMG_8183 | Large circular knobs | Easy | HIGH |
| IMG_8168 | Button caps | Easy | MEDIUM |
| IMG_8148 | Label plates | None (ready) | MEDIUM |

**Estimated Time**: 4-6 hours
**Tools Required**: ImageMagick, GIMP
**Expected Output**: 10-15 PNG assets

### Phase 2: Frames & Borders (Week 2)
**Priority: HIGH**

| Asset | Use | Difficulty | Impact |
|-------|-----|------------|--------|
| IMG_8125 | Panel borders | Medium | HIGH |
| IMG_8135 | Display frame | Easy | HIGH |
| IMG_8243 | Center rosette | Medium | MEDIUM |
| IMG_8213 | Honeycomb texture | Easy | LOW |

**Estimated Time**: 6-8 hours
**Tools Required**: Photoshop/GIMP for complex masks
**Expected Output**: 15-20 PNG assets

### Phase 3: Sliders & Advanced Controls (Week 3)
**Priority: MEDIUM**

| Asset | Use | Difficulty | Impact |
|-------|-----|------------|--------|
| IMG_8250 | Slider rails | Medium | MEDIUM |
| IMG_8180 | Slider thumb | Easy | MEDIUM |
| IMG_8200 | Clock face display | Easy | MEDIUM |

**Estimated Time**: 4-6 hours
**Expected Output**: 8-12 PNG assets

### Phase 4: Decorative Elements (Week 4)
**Priority: LOW (Polish)

**

| Asset | Use | Difficulty | Impact |
|-------|-----|------------|--------|
| IMG_8138 | Background gears | Hard | LOW |
| Gears extracted | Animated elements | Medium | LOW |
| Various ornaments | Corner decorations | Easy | LOW |

**Estimated Time**: 6-10 hours
**Expected Output**: 20-30 PNG assets

---

## JUCE Integration Notes

### Image Loading (C++)
```cpp
// In CustomLookAndFeel.h
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        // Load knob assets
        knobOrnate = juce::ImageCache::getFromFile(
            juce::File("Resources/knob_ornate_filigree_128.png"));
        knobAstrolabe = juce::ImageCache::getFromFile(
            juce::File("Resources/knob_astrolabe_rings_128.png"));

        // Load frame assets
        frameLabelPlate = juce::ImageCache::getFromFile(
            juce::File("Resources/frame_label_brass_256.png"));
    }

private:
    juce::Image knobOrnate;
    juce::Image knobAstrolabe;
    juce::Image frameLabelPlate;
};
```

### Rotary Slider Rendering with Image
```cpp
void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                     float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                     juce::Slider& slider) override
{
    // Draw knob base image
    auto knobBounds = juce::Rectangle<int>(x, y, width, height)
        .reduced(10).toFloat();

    g.drawImage(knobOrnate, knobBounds,
                juce::RectanglePlacement::centred);

    // Calculate rotation angle
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Draw pointer/indicator on top
    auto center = knobBounds.getCentre();
    auto radius = knobBounds.getWidth() * 0.35f;

    juce::Path pointer;
    pointer.addTriangle(0, -radius, -3, -radius + 8, 3, -radius + 8);
    pointer.applyTransform(juce::AffineTransform::rotation(angle)
                          .translated(center.x, center.y));

    g.setColour(juce::Colour(AETHER_CYAN));
    g.fillPath(pointer);
}
```

### Resource Embedding
```cmake
# In CMakeLists.txt
juce_add_binary_data(GenerativeMIDI_Assets
    SOURCES
        Resources/knob_ornate_filigree_128.png
        Resources/knob_astrolabe_rings_128.png
        Resources/frame_label_brass_256.png
        Resources/slider_brass_vertical_128.png
        Resources/button_cross_ornamental_64.png
)

target_link_libraries(GenerativeMIDI
    PRIVATE
        GenerativeMIDI_Assets
)
```

---

## Batch Processing Scripts

### Extract All Black Backgrounds
```bash
#!/bin/bash
# extract_black_bg.sh

for img in conceptArt/assets/IMG_*.JPG; do
    filename=$(basename "$img" .JPG)
    convert "$img" \
        -fuzz 15% \
        -transparent black \
        "conceptArt/extracted/${filename}_alpha.png"
done
```

### Resize to Multiple Resolutions
```bash
#!/bin/bash
# resize_assets.sh

SIZES=(64 128 256 512)

for png in conceptArt/extracted/*.png; do
    filename=$(basename "$png" .png)
    for size in "${SIZES[@]}"; do
        convert "$png" \
            -resize "${size}x${size}" \
            -background none \
            -gravity center \
            -extent "${size}x${size}" \
            "Resources/${filename}_${size}.png"
    done
done
```

### Python Batch Processor
```python
#!/usr/bin/env python3
# process_assets.py

from PIL import Image
import os

def remove_background(input_path, output_path, bg_color=(0, 0, 0), threshold=30):
    """Remove solid background color from image"""
    img = Image.open(input_path).convert("RGBA")
    datas = img.getdata()

    new_data = []
    for item in datas:
        # Check if pixel is close to background color
        if (abs(item[0] - bg_color[0]) < threshold and
            abs(item[1] - bg_color[1]) < threshold and
            abs(item[2] - bg_color[2]) < threshold):
            new_data.append((255, 255, 255, 0))  # Transparent
        else:
            new_data.append(item)

    img.putdata(new_data)
    img.save(output_path, "PNG")

def process_all_assets():
    input_dir = "conceptArt/assets"
    output_dir = "conceptArt/extracted"

    os.makedirs(output_dir, exist_ok=True)

    for filename in os.listdir(input_dir):
        if filename.endswith(".JPG"):
            input_path = os.path.join(input_dir, filename)
            output_path = os.path.join(output_dir,
                                      filename.replace(".JPG", "_alpha.png"))

            print(f"Processing {filename}...")
            remove_background(input_path, output_path)

if __name__ == "__main__":
    process_all_assets()
```

---

## Asset Organization Plan

### Recommended Directory Structure
```
conceptArt/
├── assets/                      # Original 147 JPG files
├── extracted/                   # Processed PNG with alpha
│   ├── knobs/
│   │   ├── knob_ornate_01_128.png
│   │   ├── knob_astrolabe_01_256.png
│   │   └── ...
│   ├── frames/
│   │   ├── frame_oval_victorian_256.png
│   │   ├── frame_label_brass_128.png
│   │   └── ...
│   ├── sliders/
│   │   ├── slider_brass_vertical_128.png
│   │   └── ...
│   ├── buttons/
│   │   ├── button_cross_64.png
│   │   └── ...
│   └── decorative/
│       ├── gear_brass_01_64.png
│       ├── ornament_filigree_01_128.png
│       └── ...
├── references/                  # Full panel mockups (not extracted)
│   ├── IMG_8119.JPG
│   ├── IMG_8120.JPG
│   └── ...
└── ASSET_CATALOG.md            # This file
```

---

## Quick Start Guide

### Step 1: Install Tools
```bash
# macOS
brew install imagemagick

# Python libraries
pip install Pillow
```

### Step 2: Extract High-Priority Assets (30 minutes)
```bash
# Create directories
mkdir -p conceptArt/extracted/{knobs,frames,buttons,sliders}

# Extract 5 priority knobs
convert conceptArt/assets/IMG_8150.JPG -fuzz 15% -transparent black \
    conceptArt/extracted/knobs/knob_ornate_filigree.png

convert conceptArt/assets/IMG_8183.JPG -fuzz 15% -transparent black \
    conceptArt/extracted/knobs/knob_astrolabe_rings.png

# Resize to working resolution
convert conceptArt/extracted/knobs/knob_ornate_filigree.png \
    -resize 128x128 \
    Resources/knob_ornate_128.png
```

### Step 3: Test in JUCE (15 minutes)
```cpp
// In CustomLookAndFeel constructor
auto knobImage = juce::ImageCache::getFromFile(
    juce::File("/path/to/Resources/knob_ornate_128.png"));

if (knobImage.isValid())
{
    DBG("Knob image loaded successfully!");
    // Use in drawRotarySlider()
}
```

---

## Expected Results

### After Phase 1 (Week 1)
- ✅ 10-15 core knob/button assets extracted
- ✅ Basic image-based rendering in CustomLookAndFeel
- ✅ Dramatically improved visual quality
- ✅ Authentic steampunk aesthetic

### After Phase 2 (Week 2)
- ✅ All frames and borders integrated
- ✅ Section panels use asset-based backgrounds
- ✅ Pattern visualizer has ornate frame
- ✅ UI looks 90% production-ready

### After Phase 3 (Week 3)
- ✅ All sliders use asset graphics
- ✅ Advanced controls (clock faces, displays)
- ✅ UI is feature-complete

### After Phase 4 (Week 4)
- ✅ Decorative elements add polish
- ✅ Animated gears in background
- ✅ Full Victorian steampunk experience
- ✅ UI is 100% production-ready

---

## Notes & Recommendations

### Best Practices
1. **Always keep originals**: Never modify original JPG files
2. **Version control assets**: Track PNG files in git (if <5MB each)
3. **Use ImageCache**: JUCE's built-in caching prevents duplicate loading
4. **Test at multiple sizes**: Ensure assets scale well (64px to 512px)
5. **Maintain color consistency**: Use extracted color palette

### Performance Considerations
- **Asset Size**: Keep individual assets under 500KB
- **Total Resources**: Keep embedded resource binary under 10MB
- **Runtime Loading**: Use ImageCache for efficient memory usage
- **Retina/HiDPI**: Provide @2x versions for high-DPI displays

### Future Enhancements
- **Animated elements**: Rotating gears, glowing crystals
- **State-based assets**: Different images for hover/pressed states
- **Seasonal themes**: Winter frost, autumn copper, spring verdigris
- **User customization**: Allow users to swap asset packs

---

**Document Version**: 1.0
**Last Updated**: 2025-10-18
**Maintainer**: Claude Code (Anthropic)
**Status**: READY FOR IMPLEMENTATION
