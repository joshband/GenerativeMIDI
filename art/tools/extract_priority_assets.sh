#!/bin/bash
#
# extract_priority_assets.sh
# Extracts and processes the top 10 priority assets for GenerativeMIDI UI
#
# Usage: ./extract_priority_assets.sh
#
# Requirements:
#   - ImageMagick (brew install imagemagick)
#
# Output:
#   - PNG files with alpha channel in conceptArt/extracted/
#   - Multiple resolutions (64, 128, 256px)
#

set -e  # Exit on error

echo "==================================="
echo "GenerativeMIDI Asset Extractor"
echo "==================================="
echo ""

# Check for ImageMagick (v7 uses 'magick' command)
if ! command -v magick &> /dev/null; then
    echo "❌ Error: ImageMagick not found"
    echo "Install with: brew install imagemagick"
    exit 1
fi

CONVERT_CMD="magick"  # ImageMagick 7

# Create output directories
echo "📁 Creating output directories..."
mkdir -p art/extracted/{knobs,frames,buttons,sliders,decorative}
mkdir -p art/ui-ready/{knobs,frames,buttons,sliders}

echo "✅ Directories created"
echo ""

# Function to extract and resize an asset
extract_asset() {
    local input_file="$1"
    local output_name="$2"
    local category="$3"
    local fuzz_percent="${4:-15}"  # Default 15% fuzz

    echo "🔧 Processing: $output_name"

    # Extract with transparency
    $CONVERT_CMD "art/original/${category}/$input_file" \
        -fuzz ${fuzz_percent}% \
        -transparent black \
        "art/extracted/${category}/${output_name}_raw.png"

    # Create 128px version (primary size for UI)
    $CONVERT_CMD "art/extracted/${category}/${output_name}_raw.png" \
        -resize 128x128 \
        -background none \
        -gravity center \
        -extent 128x128 \
        "art/ui-ready/${category}/${output_name}_128.png"

    # Create 256px version (high-res displays)
    $CONVERT_CMD "art/extracted/${category}/${output_name}_raw.png" \
        -resize 256x256 \
        -background none \
        -gravity center \
        -extent 256x256 \
        "art/ui-ready/${category}/${output_name}_256.png"

    # Create 64px version (small buttons/icons)
    $CONVERT_CMD "art/extracted/${category}/${output_name}_raw.png" \
        -resize 64x64 \
        -background none \
        -gravity center \
        -extent 64x64 \
        "art/ui-ready/${category}/${output_name}_64.png"

    echo "  ✅ Created: 64px, 128px, 256px versions"
}

echo "🎨 Extracting TIER 1 & 2 Priority Assets..."
echo ""

# TIER 1: Already isolated assets
echo "📦 TIER 1: Pre-isolated assets"
extract_asset "IMG_8148.JPG" "label_brass_plate" "frames" 10
echo ""

# TIER 2: High-priority knobs
echo "🎛️ TIER 2: Primary rotary controls"
extract_asset "IMG_8150.JPG" "knob_ornate_filigree" "knobs" 15
extract_asset "IMG_8183.JPG" "knob_astrolabe_rings" "knobs" 15
extract_asset "IMG_8200.JPG" "knob_clock_face" "knobs" 15
echo ""

# TIER 3: Frames and bezels
echo "🖼️ TIER 3: Frames and bezels"
extract_asset "IMG_8135.JPG" "frame_oval_victorian" "frames" 15
extract_asset "IMG_8125.JPG" "frame_art_deco_panel" "frames" 20
echo ""

# TIER 4: Buttons
echo "🔘 TIER 4: Buttons and switches"
extract_asset "IMG_8168.JPG" "button_cross_ornate" "buttons" 10
echo ""

# TIER 5: Sliders
echo "🎚️ TIER 5: Sliders and linear controls"
extract_asset "IMG_8250.JPG" "slider_brass_vertical" "sliders" 10
extract_asset "IMG_8180.JPG" "pointer_aether_staff" "sliders" 10
echo ""

# TIER 6: Decorative
echo "✨ TIER 6: Decorative elements"
extract_asset "IMG_8213.JPG" "texture_honeycomb_brass" "decorative" 10
echo ""

echo "==================================="
echo "✅ Extraction Complete!"
echo "==================================="
echo ""
echo "📊 Summary:"
echo "  • Assets extracted: 10"
echo "  • Resolutions per asset: 3 (64px, 128px, 256px)"
echo "  • Total files created: 30+"
echo ""
echo "📁 Output locations:"
echo "  • Raw extracted: art/extracted/"
echo "  • UI-ready assets: art/ui-ready/"
echo ""
echo "🚀 Next steps:"
echo "  1. Review extracted assets in art/ui-ready/"
echo "  2. Copy to Resources/assets/ for build embedding"
echo "  3. Test loading in JUCE with ImageCache"
echo "  4. Integrate into CustomLookAndFeel.cpp"
echo "  5. Run: cmake --build build --config Release"
echo ""
echo "📖 See art/catalog/ASSET_CATALOG.md for full documentation"
echo ""
