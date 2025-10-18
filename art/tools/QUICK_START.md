# Quick Start: Hybrid UI Generator

Get started generating UI elements in 5 minutes!

## Step 1: Check What You Have (10 seconds)

```bash
cd /Users/noisebox/Repos/GenerativeMIDI
python3 art/tools/hybrid_ui_generator.py --status
```

**Current Status:**
- ✅ You have 30 UI-ready files already!
- ⚠️  Missing: panels, indicators
- ⚠️  Incomplete: buttons (only 3/4 variants)

## Step 2: Choose Your Approach

### Option A: FREE (ImageMagick Processing)
**Use when**: You have concept art to process

```bash
# 1. Add images to art/original/{category}/
# 2. Process them
python3 art/tools/hybrid_ui_generator.py --category knobs
```

**Cost**: $0 (FREE)

### Option B: PAID (DALL·E Generation)
**Use when**: You need new elements

```bash
# 1. Set API key
export OPENAI_API_KEY="sk-your-key-here"

# 2. Generate missing elements
python3 art/tools/hybrid_ui_generator.py --category panels --use-dalle
```

**Cost**: ~$0.04 per image

### Option C: HYBRID (Recommended)
**Use when**: Mix existing + generate missing

```bash
# 1. Process existing (FREE)
python3 art/tools/hybrid_ui_generator.py

# 2. Generate only what's missing (PAID)
export OPENAI_API_KEY="sk-your-key-here"
python3 art/tools/hybrid_ui_generator.py --use-dalle
```

## Step 3: Generate Missing Elements

Based on your current status, you're missing:

### Missing: Panels (4 variants)
```bash
export OPENAI_API_KEY="sk-your-key-here"
python3 art/tools/hybrid_ui_generator.py --category panels --use-dalle
```

**Cost**: ~$0.16 (4 images × $0.04)

### Missing: Indicators (4 variants)
```bash
python3 art/tools/hybrid_ui_generator.py --category indicators --use-dalle
```

**Cost**: ~$0.16 (4 images × $0.04)

### Total Cost Estimate
- **Panels**: $0.16
- **Indicators**: $0.16
- **Total**: $0.32

## Step 4: Use in Your JUCE Plugin

Generated images are ready to use:

```cpp
// In CustomLookAndFeel.h
auto panelImage = juce::ImageCache::getFromFile(
    File("art/ui-ready/panels/background_panel_256.png")
);

auto indicatorActive = juce::ImageCache::getFromFile(
    File("art/ui-ready/indicators/active_indicator_128.png")
);
```

## What Gets Generated

Each element generates 4 sizes automatically:

```
art/ui-ready/panels/
  ├── background_panel_64.png    (icon size)
  ├── background_panel_128.png   (standard)
  ├── background_panel_256.png   (large)
  └── background_panel_512.png   (retina)
```

## Example: Generate Complete Set

Want a full set of UI elements? Here's the command:

```bash
# Set your API key once
export OPENAI_API_KEY="sk-your-key-here"

# Generate everything missing
python3 art/tools/hybrid_ui_generator.py --use-dalle

# Estimated cost:
# - Panels: 4 variants × $0.04 = $0.16
# - Indicators: 4 variants × $0.04 = $0.16
# - Buttons: 1 missing variant × $0.04 = $0.04
# Total: ~$0.36
```

## Get Your OpenAI API Key

1. Go to: https://platform.openai.com/api-keys
2. Sign in or create account
3. Click "Create new secret key"
4. Copy the key (starts with `sk-`)
5. Set it:
   ```bash
   export OPENAI_API_KEY="sk-your-key-here"
   ```

## Troubleshooting

### "ImageMagick not found"
```bash
brew install imagemagick
```

### "OpenAI library not found"
```bash
pip3 install openai requests
```

### "Invalid API key"
- Make sure key starts with `sk-`
- Check you have credits at: https://platform.openai.com/usage
- Try setting key directly:
  ```bash
  python3 art/tools/hybrid_ui_generator.py --api-key "sk-your-key" --use-dalle
  ```

## Next Steps

1. **Test first**: Generate one category to test
   ```bash
   python3 art/tools/hybrid_ui_generator.py --category indicators --use-dalle
   ```

2. **Review quality**: Check `art/ui-ready/indicators/`

3. **Tweak prompts** if needed: Edit `art/tools/hybrid_ui_generator.py`

4. **Generate rest**: Run full generation when satisfied

5. **Integrate**: Use in JUCE plugin UI

## Tips

- 💡 **Start small**: Test with one category first
- 💡 **Review images**: Check quality before mass generation
- 💡 **Use FREE first**: Process any existing concept art
- 💡 **Budget wisely**: Each image costs ~$0.04
- 💡 **Iterate prompts**: First result might not be perfect

## Full Documentation

See `HYBRID_GENERATOR_GUIDE.md` for:
- Complete feature documentation
- Advanced usage
- Custom prompts
- Cost management
- Integration examples
