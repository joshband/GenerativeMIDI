#!/usr/bin/env python3
"""
extract_priority_assets.py
Extracts and processes the top 10 priority assets for GenerativeMIDI UI

Requirements: pip install Pillow
Usage: python3 extract_priority_assets.py
"""

import os
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("❌ Error: Pillow not installed")
    print("Install with: pip3 install Pillow")
    sys.exit(1)

# Asset definitions
PRIORITY_ASSETS = [
    # (filename, output_name, category, bg_color, threshold)
    ("IMG_8148.JPG", "label_brass_plate", "frames", (180, 170, 160), 40),
    ("IMG_8150.JPG", "knob_ornate_filigree", "knobs", (10, 20, 30), 30),
    ("IMG_8183.JPG", "knob_astrolabe_rings", "knobs", (5, 5, 5), 25),
    ("IMG_8200.JPG", "knob_clock_face", "knobs", (10, 15, 20), 30),
    ("IMG_8135.JPG", "frame_oval_victorian", "frames", (5, 5, 5), 25),
    ("IMG_8125.JPG", "frame_art_deco_panel", "frames", (60, 100, 100), 50),
    ("IMG_8168.JPG", "button_cross_ornate", "buttons", (180, 180, 180), 40),
    ("IMG_8250.JPG", "slider_brass_vertical", "sliders", (200, 195, 190), 40),
    ("IMG_8180.JPG", "pointer_aether_staff", "sliders", (170, 170, 170), 40),
    ("IMG_8213.JPG", "texture_honeycomb_brass", "decorative", (170, 170, 170), 40),
]

SIZES = [64, 128, 256]

def remove_background(img, bg_color, threshold):
    """Remove background color from image with threshold"""
    img = img.convert("RGBA")
    datas = img.getdata()

    new_data = []
    for item in datas:
        # Calculate color distance
        distance = sum(abs(item[i] - bg_color[i]) for i in range(3))

        # If pixel is close to background color, make it transparent
        if distance < threshold * 3:  # threshold per channel
            new_data.append((255, 255, 255, 0))
        else:
            new_data.append(item)

    img.putdata(new_data)
    return img

def resize_with_padding(img, size):
    """Resize image to target size while maintaining aspect ratio"""
    # Calculate new size maintaining aspect ratio
    img.thumbnail((size, size), Image.Resampling.LANCZOS)

    # Create new image with transparent background
    new_img = Image.new('RGBA', (size, size), (0, 0, 0, 0))

    # Paste resized image in center
    offset = ((size - img.size[0]) // 2, (size - img.size[1]) // 2)
    new_img.paste(img, offset, img)

    return new_img

def extract_asset(input_file, output_name, category, bg_color, threshold):
    """Extract single asset and create multiple resolutions"""
    input_path = Path("art/original") / input_file

    if not input_path.exists():
        print(f"  ⚠️  Skipping {input_file} - file not found")
        return False

    print(f"🔧 Processing: {output_name}")

    try:
        # Load and process image
        img = Image.open(input_path)
        img_alpha = remove_background(img, bg_color, threshold)

        # Save raw extracted version
        raw_dir = Path("art/extracted") / category
        raw_dir.mkdir(parents=True, exist_ok=True)
        img_alpha.save(raw_dir / f"{output_name}_raw.png")

        # Create multiple resolutions
        for size in SIZES:
            output_dir = Path("art/ui-ready") / category
            output_dir.mkdir(parents=True, exist_ok=True)

            resized = resize_with_padding(img_alpha.copy(), size)
            output_path = output_dir / f"{output_name}_{size}.png"
            resized.save(output_path, "PNG", optimize=True)

        print(f"  ✅ Created: {', '.join(str(s) + 'px' for s in SIZES)} versions")
        return True

    except Exception as e:
        print(f"  ❌ Error: {e}")
        return False

def main():
    print("=" * 50)
    print("GenerativeMIDI Asset Extractor (Python)")
    print("=" * 50)
    print()

    # Create base directories
    print("📁 Creating output directories...")
    for category in ["knobs", "frames", "buttons", "sliders", "decorative"]:
        Path("art/extracted").mkdir(parents=True, exist_ok=True)
        (Path("art/ui-ready") / category).mkdir(parents=True, exist_ok=True)
    print("✅ Directories created")
    print()

    # Process assets
    print("🎨 Extracting TIER 1 & 2 Priority Assets...")
    print()

    success_count = 0
    total_count = len(PRIORITY_ASSETS)

    for asset_info in PRIORITY_ASSETS:
        if extract_asset(*asset_info):
            success_count += 1
        print()

    # Summary
    print("=" * 50)
    print("✅ Extraction Complete!")
    print("=" * 50)
    print()
    print(f"📊 Summary:")
    print(f"  • Assets extracted: {success_count}/{total_count}")
    print(f"  • Resolutions per asset: {len(SIZES)} ({', '.join(str(s) + 'px' for s in SIZES)})")
    print(f"  • Total files created: {success_count * len(SIZES)}+")
    print()
    print("📁 Output locations:")
    print("  • Raw extracted: art/extracted/")
    print("  • UI-ready assets: art/ui-ready/")
    print()
    print("🚀 Next steps:")
    print("  1. Review extracted assets in art/ui-ready/")
    print("  2. Test loading in JUCE with ImageCache")
    print("  3. Integrate into CustomLookAndFeel.cpp")
    print("  4. Run: cmake --build build --config Release")
    print()
    print("📖 See art/catalog/ASSET_CATALOG.md for full documentation")
    print()

if __name__ == "__main__":
    main()
