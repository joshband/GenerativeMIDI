# UI Element Generation Tools

Hybrid system for generating and processing UI elements for the GenerativeMIDI plugin.

## 🚀 Quick Start

**See**: [QUICK_START.md](QUICK_START.md) - Get started in 5 minutes!

## 📚 Documentation

- **[QUICK_START.md](QUICK_START.md)** - Fast setup and first generation
- **[HYBRID_GENERATOR_GUIDE.md](HYBRID_GENERATOR_GUIDE.md)** - Complete documentation

## 🛠️ Tools

### `hybrid_ui_generator.py`

Main tool combining ImageMagick and DALL·E for UI element generation.

**Features:**
- ✅ Process existing concept art (FREE)
- ✅ Generate new elements with DALL·E (PAID)
- ✅ Automatic multi-size variants (64, 128, 256, 512px)
- ✅ Transparency and alpha channel processing
- ✅ Organized output structure
- ✅ Element cataloging

## 💰 Cost Breakdown

### FREE (ImageMagick)
- Process existing images: **$0**
- Resize, transparency, variants: **$0**
- **Use this first!**

### PAID (DALL·E API)
- Generate new image: **~$0.04** per image
- Complete UI set (~20 images): **~$0.80**
- Budget-friendly incremental generation

## 📁 Current Status

Run this to see what you have:

```bash
python3 art/tools/hybrid_ui_generator.py --status
```

**Your Current Inventory:**
- ✅ 30 UI-ready files
- ⚠️  Missing: panels (4 variants), indicators (4 variants)
- 💰 Cost to complete: ~$0.32

## 🎨 Supported Elements

1. **Knobs** - Rotary controls (9 files ✅)
2. **Sliders** - Linear controls (6 files ✅)
3. **Buttons** - Interactive elements (3 files ⚠️)
4. **Panels** - Backgrounds & frames (0 files ❌)
5. **Decorative** - Ornaments (3 files ✅)
6. **Frames** - Borders & labels (9 files ✅)
7. **Indicators** - Lights & status (0 files ❌)

## 🔧 Installation

### Required (Already Installed ✅)
- ImageMagick (`magick`)
- Python 3 with OpenAI library
- Pillow for image processing

### Optional (For DALL·E)
- OpenAI API key from platform.openai.com
- Credits in your OpenAI account

## 📖 Basic Usage

### Check Status
```bash
python3 art/tools/hybrid_ui_generator.py --status
```

### Process Existing Images (FREE)
```bash
python3 art/tools/hybrid_ui_generator.py --category knobs
```

### Generate Missing Elements (PAID)
```bash
export OPENAI_API_KEY="sk-your-key-here"
python3 art/tools/hybrid_ui_generator.py --category panels --use-dalle
```

### Generate Everything Missing
```bash
export OPENAI_API_KEY="sk-your-key-here"
python3 art/tools/hybrid_ui_generator.py --use-dalle
```

## 🎯 Recommended Workflow

1. **Check status** - See what you have
2. **Process existing** - Use ImageMagick (FREE)
3. **Identify gaps** - What's missing?
4. **Generate needed** - Use DALL·E selectively (PAID)
5. **Integrate** - Use in JUCE plugin

## 📊 Quality Standards

All generated images include:
- ✅ Transparent backgrounds
- ✅ PNG format with alpha channel
- ✅ 4 size variants (64, 128, 256, 512px)
- ✅ Victorian steampunk aesthetic
- ✅ Brass/gold/copper color palette
- ✅ Professional quality

## 🔗 Integration with JUCE

```cpp
// Load generated assets in your plugin
auto knobImage = juce::ImageCache::getFromFile(
    File("art/ui-ready/knobs/large_knob_256.png")
);
```

## 💡 Tips

- Start with FREE ImageMagick processing
- Generate missing elements incrementally
- Review quality before mass generation
- Tweak prompts for better results
- Budget: ~$0.04 per image

## 📝 Files

- `hybrid_ui_generator.py` - Main generator script
- `QUICK_START.md` - Quick setup guide
- `HYBRID_GENERATOR_GUIDE.md` - Complete documentation
- `README.md` - This file

## 🆘 Help

See [HYBRID_GENERATOR_GUIDE.md](HYBRID_GENERATOR_GUIDE.md) for:
- Troubleshooting
- Advanced usage
- Custom prompts
- Cost management
- Integration examples
