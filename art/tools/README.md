# Asset Extraction Tools

Tools for processing the 147 AI-generated steampunk assets into UI-ready PNG files.

---

## 🛠️ Available Tools

### 1. Python Script (Recommended)
**File**: `extract_priority_assets.py`

**Requirements**:
```bash
pip3 install Pillow
```

**Usage**:
```bash
python3 extract_priority_assets.py
```

**Features**:
- ✅ No external dependencies (besides Pillow)
- ✅ Cross-platform (macOS, Windows, Linux)
- ✅ Customizable background removal threshold
- ✅ Progress reporting

### 2. Bash Script (ImageMagick)
**File**: `extract_priority_assets.sh`

**Requirements**:
```bash
brew install imagemagick  # macOS
# or
apt-get install imagemagick  # Linux
```

**Usage**:
```bash
./extract_priority_assets.sh
```

**Features**:
- ✅ Faster processing (native C implementation)
- ✅ More accurate color selection
- ✅ Better edge quality
- ✅ Unix-style output

---

## 📦 What Gets Extracted

Both scripts extract the same 10 priority assets:

**Knobs** (3 assets):
- knob_ornate_filigree
- knob_astrolabe_rings
- knob_clock_face

**Frames** (3 assets):
- label_brass_plate
- frame_oval_victorian
- frame_art_deco_panel

**Buttons** (1 asset):
- button_cross_ornate

**Sliders** (2 assets):
- slider_brass_vertical
- pointer_aether_staff

**Decorative** (1 asset):
- texture_honeycomb_brass

---

## 📐 Output Specifications

Each asset is created in **3 resolutions**:

- **64px** - Small buttons and icons
- **128px** - Standard UI controls (recommended)
- **256px** - Large displays and high-DPI screens

**Total Output**: 30+ PNG files

---

## 📂 Output Directories

```
art/
├── extracted/          # Raw extractions (full resolution with alpha)
│   ├── knobs/
│   ├── frames/
│   └── ...
│
└── ui-ready/          # Multi-resolution finals
    ├── knobs/
    │   ├── knob_ornate_filigree_64.png
    │   ├── knob_ornate_filigree_128.png
    │   └── knob_ornate_filigree_256.png
    └── ...
```

---

## 🔧 Customizing Extraction

### Add New Assets (Python)

Edit `extract_priority_assets.py`:

```python
PRIORITY_ASSETS = [
    # (filename, output_name, category, bg_color, threshold)
    ("IMG_8XXX.JPG", "new_asset_name", "knobs", (10, 20, 30), 30),
    # ... add more ...
]
```

### Add New Assets (Bash)

Edit `extract_priority_assets.sh`:

```bash
# Add to extraction section
extract_asset "IMG_8XXX.JPG" "new_asset_name" "knobs" 15
```

### Adjust Background Removal

**Python** - Change threshold value:
```python
("IMG_8150.JPG", "knob_ornate", "knobs", (10, 20, 30), 40)  # Higher = more aggressive
```

**Bash** - Change fuzz percentage:
```bash
extract_asset "IMG_8150.JPG" "knob_ornate" "knobs" 20  # Higher = more aggressive
```

---

## 🎨 Background Colors

Common background colors to remove:

- **Pure Black**: `(0, 0, 0)` - Most panel mockups
- **Dark Gray**: `(10, 20, 30)` - Some knobs and frames
- **Light Gray**: `(180, 180, 180)` - Pre-isolated elements
- **Neutral**: `(170, 170, 170)` - Buttons and decorative

---

## ⚡ Batch Processing

To extract all 147 assets (not just priority 10):

1. Update the asset list in the script
2. Categorize by visual inspection
3. Run extraction
4. Review output quality
5. Adjust thresholds as needed

---

## 🐛 Troubleshooting

### Assets have jagged edges
- **Python**: Increase threshold value
- **Bash**: Increase fuzz percentage

### Too much removed
- **Python**: Decrease threshold value
- **Bash**: Decrease fuzz percentage

### Script fails
- **Python**: Check Pillow installation: `pip3 show Pillow`
- **Bash**: Check ImageMagick: `magick --version`

---

## 📚 See Also

- [Asset Catalog](../catalog/ASSET_CATALOG.md) - Complete asset reference
- [Quick Start Guide](../catalog/QUICK_START.md) - Integration tutorial
- [JUCE Examples](../catalog/JUCE_INTEGRATION_EXAMPLE.cpp) - Code examples

---

**Last Updated**: 2025-10-18
