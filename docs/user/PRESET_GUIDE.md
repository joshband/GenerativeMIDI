# Preset System Guide

**GenerativeMIDI v0.6.1**

The preset system allows you to save, load, and share your favorite configurations.

---

## Quick Start

### Creating Your First Preset

1. **Configure your sound**: Adjust all parameters (tempo, steps, pulses, velocity, pitch, etc.) to your liking
2. **Open Preset Browser**: Click the "Presets" button in the top-right corner of the plugin
3. **Save**: Click the "Save" button in the preset browser
4. **Fill in details**:
   - **Name**: Give your preset a descriptive name (e.g., "Ambient Drone")
   - **Author**: Your name (pre-filled with system username)
   - **Category**: Choose or type a category (Euclidean, Polyrhythm, Algorithmic, Rhythmic, Ambient, Experimental)
   - **Description**: Brief description of what the preset does
5. **Confirm**: Click OK to save

Your preset is now saved to disk and will appear in the preset list!

---

## Using Presets

### Loading a Preset

**Method 1: Double-click**
- Open the Presets browser
- Double-click any preset in the list

**Method 2: Navigation buttons**
- Use the `<` and `>` buttons to cycle through presets
- Current preset name shows below the navigation buttons

**Current Preset Display**:
- The main UI shows the current preset name in the top-right area
- Updates automatically when you load a new preset

---

## Managing Presets

### Categories

Filter presets by category using the dropdown at the top of the preset browser:
- **All** - Show all presets
- **Euclidean** - Euclidean rhythm patterns
- **Polyrhythm** - Multi-layer polyrhythmic sequences
- **Algorithmic** - Markov/L-System/Cellular automata
- **Rhythmic** - Drum and percussion patterns
- **Ambient** - Slow, sparse, atmospheric
- **Experimental** - Unusual or creative configurations

### Deleting Presets

1. Select a preset in the list
2. Click the "Delete" button
3. Confirm deletion

**Note**: You can only delete user presets. Factory presets are protected.

---

## Sharing Presets

### Exporting

1. Select a preset in the list
2. Click "Export"
3. Choose a destination (automatically adds .gmpreset extension)
4. Share the .gmpreset file with others!

### Importing

1. Click "Import" in the preset browser
2. Select a .gmpreset file
3. The preset is automatically copied to your preset directory and appears in the list

---

## Preset Storage Locations

Presets are stored in platform-specific directories:

**macOS**:
```
~/Library/Application Support/GenerativeMIDI/Presets/
```

**Windows**:
```
%APPDATA%\GenerativeMIDI\Presets\
```

**Linux**:
```
~/.GenerativeMIDI/Presets/
```

You can manually copy .gmpreset files to these directories, and they'll appear after restarting the plugin (or the preset browser auto-refreshes every 2 seconds).

---

## Preset File Format

Presets are stored as XML files with a `.gmpreset` extension:

```xml
<GenerativeMIDIPreset name="My Preset" author="Your Name"
                      category="Euclidean" description="..." version="1.0">
  <GenerativeMIDI>
    <PARAM id="tempo" value="0.5"/>
    <PARAM id="euclideanSteps" value="0.25"/>
    <!-- ... all 26 parameters ... -->
  </GenerativeMIDI>
</GenerativeMIDIPreset>
```

All parameter values are stored as normalized 0.0-1.0 values.

---

## Tips and Tricks

### Organizing Your Presets

- Use descriptive names: "Ambient Drone C Minor" instead of "Preset 1"
- Use consistent category names for easy filtering
- Add detailed descriptions to remember what makes each preset special

### Creating Preset Collections

- Export your favorite presets to a folder
- Share preset collections with collaborators
- Create themed collections (e.g., "Drum Patterns", "Melodic Sequences")

### Preset Workflow

1. Start with default settings or load a similar preset
2. Make adjustments
3. Save as a new preset (don't overwrite!)
4. Experiment and iterate
5. Export your best presets for backup

---

## Troubleshooting

### Presets Not Appearing

- Check that .gmpreset files are in the correct directory
- Restart the plugin
- Open and close the preset browser to trigger a refresh

### Preset Won't Load

- Make sure the .gmpreset file is valid XML
- Check that the file has the correct structure (see format above)
- Try re-exporting the preset from the source

### Current Preset Label Doesn't Update

- The label updates every 30ms via the editor timer
- Make sure you've actually loaded a preset (double-click in the list)
- Check that the preset browser window closed properly after loading

---

## Known Limitations (v0.6.1)

- **No Factory Presets**: Factory presets temporarily disabled pending refactor
- **No Preset Preview**: Can't audition presets before loading
- **No Preset Tags**: Only one category per preset (no multiple tags)
- **No Preset Search**: Must browse or filter by category

These will be addressed in future updates!

---

## Future Enhancements

Planned for upcoming versions:

- **Factory Presets** (v0.6.2): 20+ included presets showcasing all features
- **Preset Preview** (v0.7.0): Audition presets before loading
- **Preset Tags** (v0.7.0): Multiple tags per preset for better organization
- **Preset Search** (v0.7.0): Search by name, author, description, or tags
- **Preset Banks** (v0.8.0): Group related presets into banks
- **Randomize/Mutate** (v0.8.0): Generate variations of existing presets

---

## Questions?

Check the main [README.md](../../README.md) for general plugin documentation.

For development questions, see [docs/developer/](../developer/).

---

**Last Updated**: 2025-10-18
**Version**: v0.6.1
