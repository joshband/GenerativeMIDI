# GenerativeMIDI Art Assets

Visual assets for the Victorian steampunk UI of GenerativeMIDI, including 147 AI-generated designs and processed UI-ready graphics.

---

## 📂 Directory Structure

### `original/` - Source Assets (147 JPG files)
AI-generated steampunk designs organized by type:

- **`panels/`** (3 files) - Complete control panel mockups for inspiration
- **`knobs/`** (3 files) - Rotary control designs (ornate filigree, astrolabe rings, clock faces)
- **`frames/`** (4 files) - Decorative frames and bezels (Art Deco, Victorian oval, brass labels)
- **`sliders/`** (2 files) - Linear control designs (brass vertical sliders, ornate pointers)
- **`buttons/`** (1 file) - Button and switch designs (ornate brass cross)
- **`decorative/`** (134 files) - Ornamental elements, gears, textures, and uncategorized designs

**Note**: Original JPGs should never be modified - they are the source material.

### `extracted/` - Raw PNG Extractions
Processed assets with background removed, organized by category:
- Transparency applied (alpha channel)
- Full resolution preserved
- Ready for further processing

### `ui-ready/` - Production Assets
Final assets at multiple resolutions (64px, 128px, 256px):
- **`knobs/`** - Rotary slider graphics
- **`frames/`** - Borders and label plates
- **`buttons/`** - Button overlays
- **`sliders/`** - Slider tracks and thumbs
- **`decorative/`** - Background textures and accents

**These are used in**: `Resources/assets/` (embedded in binary)

### `icons/` - App Icons and SVG Elements
- **Generator Icons** (6 SVG files):
  - `icon-euclidean.svg` / `icon-euclidean-brass.svg`
  - `icon-markov.svg` / `icon-markov-brass.svg`
  - `icon-lsystem.svg`
  - `icon-swing-brass.svg`
- App icons and branding elements
- Platform-specific icons (macOS, iOS, Windows)

### `mockups/` - Design References and UI Mockups
- **SYNAPTIK Mockups** (10 image files):
  - `synaptyk_01.png` through `synaptyk_12.jpeg`
  - Original AI-generated UI design concepts
- Full UI mockups and design explorations
- Color palette references
- Layout studies

### `tools/` - Asset Processing Scripts
- **`extract_priority_assets.sh`** - Bash script using ImageMagick
- **`extract_priority_assets.py`** - Python script using Pillow
- **`README.md`** - Tool documentation and usage

### `catalog/` - Asset Documentation
- **`ASSET_CATALOG.md`** - Complete reference for all 147 assets
- **`QUICK_START.md`** - 15-minute integration guide
- **`JUCE_INTEGRATION_EXAMPLE.cpp`** - Code examples for using assets
- **`ORIGINAL_CONCEPT_README.md`** - Archived SYNAPTIK concept documentation

---

## 🎨 Asset Workflow

```
1. Original JPG → 2. Extract (remove background) → 3. Resize (64/128/256px) → 4. Use in UI
   art/original/      art/extracted/                  art/ui-ready/              Resources/assets/
```

---

## 🚀 Quick Start

### Extract New Assets
```bash
# Using Python (requires: pip install Pillow)
python3 art/tools/extract_priority_assets.py

# Using Bash (requires: brew install imagemagick)
./art/tools/extract_priority_assets.sh
```

### Use Assets in JUCE
See [catalog/QUICK_START.md](catalog/QUICK_START.md) for 15-minute integration guide.

---

## 📊 Asset Statistics

- **Total Original Assets**: 147 JPG files
- **Categorized Assets**: 13 priority assets sorted by type
- **UI-Ready Assets**: 30+ PNG files at 3 resolutions each
- **Total Output Files**: 90+ processed assets

---

## 🎨 Style Guide

**Color Palette**:
- Brass Aged: `#B8860B`
- Gold Temple: `#FFD700`
- Copper Steam: `#B87333`
- Aether Cyan: `#00CED1`
- Verdigris Green: `#43B3AE`
- Patina Blue: `#3A6B7C`

**Aesthetic**: Victorian steampunk, gilded brass, verdigris patina, Art Deco details

---

## 📚 Documentation

- **Complete Catalog**: [catalog/ASSET_CATALOG.md](catalog/ASSET_CATALOG.md)
- **Quick Start Guide**: [catalog/QUICK_START.md](catalog/QUICK_START.md)
- **JUCE Integration**: [catalog/JUCE_INTEGRATION_EXAMPLE.cpp](catalog/JUCE_INTEGRATION_EXAMPLE.cpp)
- **Tool Documentation**: [tools/README.md](tools/README.md)

---

## 🔄 Future Categorization

The 134 uncategorized assets in `original/decorative/` can be further sorted into:
- Gears and mechanical elements
- Ornamental filigree and corners
- Texture overlays and backgrounds
- Indicators and pointers
- Display frames and screens

See [catalog/ASSET_CATALOG.md](catalog/ASSET_CATALOG.md) for detailed asset descriptions.

---

**Last Updated**: 2025-10-18
