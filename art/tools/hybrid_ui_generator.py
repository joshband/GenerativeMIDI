#!/usr/bin/env python3
"""
Hybrid UI Element Generator
Combines ImageMagick processing with OpenAI DALL·E generation

Features:
- Extract and process existing concept art using ImageMagick
- Generate new UI elements using DALL·E API when needed
- Create multiple size variants (64px, 128px, 256px, 512px)
- Automatic transparency and alpha channel processing
- Organized output structure
"""

import os
import sys
import json
import subprocess
from pathlib import Path
from typing import List, Dict, Optional
import argparse

try:
    from openai import OpenAI
    OPENAI_AVAILABLE = True
except ImportError:
    OPENAI_AVAILABLE = False
    print("⚠️  OpenAI library not found. DALL·E generation will be disabled.")
    print("   Install with: pip install openai")

try:
    import requests
    REQUESTS_AVAILABLE = True
except ImportError:
    REQUESTS_AVAILABLE = False
    print("⚠️  requests library not found. Image downloading will be disabled.")
    print("   Install with: pip install requests")


class UIElementGenerator:
    """Hybrid UI element generator using ImageMagick and DALL·E"""

    def __init__(self, project_root: str, openai_key: Optional[str] = None):
        self.project_root = Path(project_root)
        self.art_dir = self.project_root / "art"
        self.original_dir = self.art_dir / "original"
        self.ui_ready_dir = self.art_dir / "ui-ready"
        self.catalog_file = self.art_dir / "catalog" / "ui_elements.json"

        # Initialize OpenAI client if key provided
        self.client = None
        if openai_key and OPENAI_AVAILABLE:
            self.client = OpenAI(api_key=openai_key)
            print("✅ OpenAI DALL·E enabled")
        elif openai_key and not OPENAI_AVAILABLE:
            print("❌ OpenAI key provided but library not installed")

        # Standard sizes for UI elements
        self.sizes = [64, 128, 256, 512]

        # UI element categories and their DALL·E prompts
        self.element_catalog = {
            "knobs": {
                "base_prompt": "Victorian steampunk brass rotary knob with verdigris patina, aether crystal center, ornate filigree, top-down view, transparent background, 4k",
                "variants": ["small", "medium", "large", "parameter"]
            },
            "sliders": {
                "base_prompt": "Art Deco brass vertical slider rail with golden highlights, Victorian gothic design, transparent background, 4k",
                "variants": ["vertical", "horizontal", "thumb", "track"]
            },
            "buttons": {
                "base_prompt": "Gilded brass Victorian button with ornate border, steampunk mechanical details, transparent background, 4k",
                "variants": ["round", "square", "ornate", "simple"]
            },
            "panels": {
                "base_prompt": "Steampunk brass panel with Art Deco border, rivets, verdigris patina, gothic Victorian design, transparent background, 4k",
                "variants": ["background", "frame", "inset", "raised"]
            },
            "decorative": {
                "base_prompt": "Victorian steampunk brass decorative element, filigree pattern, golden highlights, transparent background, 4k",
                "variants": ["corner", "divider", "ornament", "pattern"]
            },
            "frames": {
                "base_prompt": "Ornate brass Victorian frame with Art Deco styling, steampunk details, transparent background, 4k",
                "variants": ["rectangular", "circular", "oval", "label_plate"]
            },
            "indicators": {
                "base_prompt": "Aether crystal indicator light, glowing cyan energy, brass Victorian housing, transparent background, 4k",
                "variants": ["active", "inactive", "warning", "ready"]
            }
        }

    def check_imagemagick(self) -> bool:
        """Check if ImageMagick is installed"""
        try:
            result = subprocess.run(['magick', '--version'],
                                  capture_output=True, text=True)
            return result.returncode == 0
        except FileNotFoundError:
            return False

    def process_existing_image(self, input_path: Path, category: str, variant: str) -> List[Path]:
        """Process existing image using ImageMagick"""
        if not self.check_imagemagick():
            print("❌ ImageMagick not found. Cannot process images.")
            return []

        output_dir = self.ui_ready_dir / category
        output_dir.mkdir(parents=True, exist_ok=True)

        processed_files = []
        base_name = f"{variant}_{input_path.stem}"

        for size in self.sizes:
            output_file = output_dir / f"{base_name}_{size}.png"

            # ImageMagick command for processing
            cmd = [
                'magick', str(input_path),
                '-fuzz', '20%',           # Fuzzy color matching
                '-transparent', 'white',   # Make white transparent
                '-background', 'none',     # Transparent background
                '-resize', f'{size}x{size}', # Resize
                '-gravity', 'center',      # Center the image
                '-extent', f'{size}x{size}', # Pad to exact size
                str(output_file)
            ]

            try:
                subprocess.run(cmd, check=True, capture_output=True)
                processed_files.append(output_file)
                print(f"  ✓ Created {output_file.name}")
            except subprocess.CalledProcessError as e:
                print(f"  ✗ Failed to process {input_path.name}: {e}")

        return processed_files

    def generate_with_dalle(self, category: str, variant: str,
                           custom_prompt: Optional[str] = None) -> Optional[str]:
        """Generate new UI element using DALL·E"""
        if not self.client:
            print("❌ DALL·E not enabled. Set OPENAI_API_KEY to use generation.")
            return None

        if not REQUESTS_AVAILABLE:
            print("❌ requests library not available. Cannot download images.")
            return None

        # Build prompt
        if custom_prompt:
            prompt = custom_prompt
        else:
            base_prompt = self.element_catalog.get(category, {}).get("base_prompt", "")
            prompt = f"{base_prompt}, {variant} variant"

        print(f"🎨 Generating with DALL·E: {prompt[:80]}...")

        try:
            response = self.client.images.generate(
                model="dall-e-3",
                prompt=prompt,
                size="1024x1024",
                quality="standard",
                n=1,
            )

            image_url = response.data[0].url
            print(f"  ✓ Generated image URL: {image_url}")

            # Download the image
            output_dir = self.original_dir / category
            output_dir.mkdir(parents=True, exist_ok=True)
            output_file = output_dir / f"dalle_{variant}_1024.png"

            response = requests.get(image_url)
            if response.status_code == 200:
                with open(output_file, 'wb') as f:
                    f.write(response.content)
                print(f"  ✓ Saved to {output_file}")
                return str(output_file)
            else:
                print(f"  ✗ Failed to download image")
                return None

        except Exception as e:
            print(f"  ✗ DALL·E generation failed: {e}")
            return None

    def process_category(self, category: str, use_dalle: bool = False):
        """Process all images in a category"""
        print(f"\n{'='*60}")
        print(f"Processing category: {category}")
        print(f"{'='*60}")

        input_dir = self.original_dir / category

        # Process existing images
        if input_dir.exists():
            image_files = list(input_dir.glob("*.png")) + list(input_dir.glob("*.jpg"))

            if image_files:
                print(f"Found {len(image_files)} existing images")
                for img_file in image_files:
                    variant = img_file.stem.split('_')[0] if '_' in img_file.stem else "default"
                    print(f"\n📸 Processing: {img_file.name}")
                    self.process_existing_image(img_file, category, variant)
            else:
                print("No existing images found")
        else:
            print(f"Directory {input_dir} does not exist")

        # Generate missing variants with DALL·E if enabled
        if use_dalle and category in self.element_catalog:
            print(f"\n🎨 Checking for missing variants...")
            variants = self.element_catalog[category]["variants"]

            for variant in variants:
                # Check if variant already exists
                output_dir = self.ui_ready_dir / category
                existing = list(output_dir.glob(f"*{variant}*.png")) if output_dir.exists() else []

                if not existing:
                    print(f"\n  Missing variant: {variant}")
                    generated = self.generate_with_dalle(category, variant)

                    if generated:
                        # Process the generated image
                        self.process_existing_image(Path(generated), category, variant)
                else:
                    print(f"  ✓ {variant} already exists ({len(existing)} files)")

    def scan_and_catalog(self):
        """Scan all directories and create catalog"""
        catalog = {
            "categories": {},
            "statistics": {
                "total_original": 0,
                "total_ui_ready": 0,
                "categories_count": 0
            }
        }

        for category in self.element_catalog.keys():
            original_dir = self.original_dir / category
            ui_ready_dir = self.ui_ready_dir / category

            original_count = len(list(original_dir.glob("*.png"))) if original_dir.exists() else 0
            ui_ready_count = len(list(ui_ready_dir.glob("*.png"))) if ui_ready_dir.exists() else 0

            catalog["categories"][category] = {
                "original_files": original_count,
                "ui_ready_files": ui_ready_count,
                "variants": self.element_catalog[category]["variants"]
            }

            catalog["statistics"]["total_original"] += original_count
            catalog["statistics"]["total_ui_ready"] += ui_ready_count
            if original_count > 0 or ui_ready_count > 0:
                catalog["statistics"]["categories_count"] += 1

        # Save catalog
        self.catalog_file.parent.mkdir(parents=True, exist_ok=True)
        with open(self.catalog_file, 'w') as f:
            json.dump(catalog, f, indent=2)

        return catalog

    def print_status(self):
        """Print current status of UI elements"""
        catalog = self.scan_and_catalog()

        print(f"\n{'='*60}")
        print("UI ELEMENT STATUS")
        print(f"{'='*60}\n")

        for category, data in catalog["categories"].items():
            print(f"📁 {category.upper()}")
            print(f"   Original: {data['original_files']} files")
            print(f"   UI-Ready: {data['ui_ready_files']} files")
            print(f"   Variants: {', '.join(data['variants'])}")
            print()

        stats = catalog["statistics"]
        print(f"{'='*60}")
        print(f"TOTALS:")
        print(f"  Categories: {stats['categories_count']}")
        print(f"  Original:   {stats['total_original']} files")
        print(f"  UI-Ready:   {stats['total_ui_ready']} files")
        print(f"{'='*60}\n")


def main():
    parser = argparse.ArgumentParser(description="Hybrid UI Element Generator")
    parser.add_argument('--project-root', default='/Users/noisebox/Repos/GenerativeMIDI',
                       help='Project root directory')
    parser.add_argument('--category', help='Process specific category only')
    parser.add_argument('--use-dalle', action='store_true',
                       help='Generate missing elements with DALL·E')
    parser.add_argument('--status', action='store_true',
                       help='Show current status only')
    parser.add_argument('--api-key', help='OpenAI API key (or set OPENAI_API_KEY env var)')

    args = parser.parse_args()

    # Get API key from args or environment
    api_key = args.api_key or os.environ.get('OPENAI_API_KEY')

    # Initialize generator
    generator = UIElementGenerator(args.project_root, api_key)

    # Check ImageMagick
    if not generator.check_imagemagick():
        print("❌ ImageMagick is not installed!")
        print("   Install with: brew install imagemagick")
        sys.exit(1)

    print("✅ ImageMagick is available")

    # Show status
    if args.status:
        generator.print_status()
        return

    # Process categories
    if args.category:
        generator.process_category(args.category, args.use_dalle)
    else:
        # Process all categories
        for category in generator.element_catalog.keys():
            generator.process_category(category, args.use_dalle)

    # Update and show final status
    generator.print_status()


if __name__ == "__main__":
    main()
