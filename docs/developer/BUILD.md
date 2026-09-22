# Build Instructions

**Canonical build system: CMake.** The `GenerativeMIDI.jucer` Projucer project is retained only as a historical reference and is **deprecated** — do not use it for new work.

## Prerequisites

1. **Clone with submodules** (required for Victorian UI bitmaps under `art/`):

```bash
git clone --recurse-submodules https://github.com/joshband/GenerativeMIDI.git
cd GenerativeMIDI
```

If you already cloned without submodules:

```bash
git submodule update --init --recursive
```

2. **JUCE**: symlink or place a JUCE checkout at `./JUCE` (see Troubleshooting).

## Quick Start — CMake (canonical)

```bash
cd GenerativeMIDI   # repository root

# Create build directory
mkdir -p build && cd build

# Configure (Release build)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release -j8
```

### Build outputs

- **Standalone**: `build/GenerativeMIDI_artefacts/Release/Standalone/Generative MIDI.app`
- **AU**: `build/GenerativeMIDI_artefacts/Release/AU/Generative MIDI.component`
- **VST3**: `build/GenerativeMIDI_artefacts/Release/VST3/Generative MIDI.vst3`
- **AUv3**: `build/GenerativeMIDI_artefacts/Release/AUv3/Generative MIDI.appex` (configured; not App Store–ready)

### Unit tests

From the build directory (after configure):

```bash
ctest --output-on-failure
```

CI runs macOS, iOS, Windows, and Linux jobs (including pluginval on VST3). See [CI.md](CI.md).

## Projucer path (deprecated)

`GenerativeMIDI.jucer` may still open in Projucer for archaeology, but **CMake is the supported path**. Module paths, exporters, and CI all assume CMake. Prefer fixing CMakeLists over regenerating from Projucer.

If you must open it locally:

```bash
open -a Projucer.app GenerativeMIDI.jucer
```

Set Projucer’s global JUCE path to your local JUCE install (or rely on the in-tree `JUCE` symlink), then prefer switching back to CMake for actual builds.

## Troubleshooting

### Error: JUCE not found

```bash
cd GenerativeMIDI   # repository root
ln -s ~/JUCE JUCE   # adjust path to your JUCE checkout
```

### Error: Missing Victorian UI bitmaps

```bash
git submodule update --init --recursive
```

Without the `art/` submodule, the LookAndFeel falls back to procedural drawing.

### Compiler Errors

Make sure you have:

- Xcode 14+ installed (macOS)
- Command Line Tools: `xcode-select --install`
- macOS deployment target set correctly

## Installing the Plugin

### Manual Installation

```bash
# Copy to system plugin folders
cp -r "build/GenerativeMIDI_artefacts/Release/AU/Generative MIDI.component" \
    ~/Library/Audio/Plug-Ins/Components/

cp -r "build/GenerativeMIDI_artefacts/Release/VST3/Generative MIDI.vst3" \
    ~/Library/Audio/Plug-Ins/VST3/
```

### Using COPY_PLUGIN_AFTER_BUILD

The plugin is configured to automatically copy after building if you set:

```cmake
COPY_PLUGIN_AFTER_BUILD TRUE
```

This will automatically install to your system folders.

## Development Build (Debug)

```bash
mkdir build-debug && cd build-debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

Debug builds include:

- Full debug symbols
- No optimization
- Assertions enabled
- Easier to debug with LLDB/GDB

## Clean Build

```bash
cd build
cmake --build . --target clean
# Or remove and recreate
cd ..
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## Platform-Specific Notes

### macOS (primary)

- **AU** (Audio Units): Native macOS format
- **AUv3**: iOS/macOS App Extension format (sideload / TestFlight path; not store-ready)
- **VST3**: Cross-platform format
- **Standalone**: Runs independently

### Windows (cross-compile or native)

- Use Visual Studio 2022
- CMake selects **VST3** and **Standalone** only (AU/AUv3 are Apple-only)
- Covered in CI (`build-windows`); see [CI.md](CI.md)

### Linux

- Use CMake directly; install ALSA/X11/FreeType (and optional JACK) headers
- CMake selects **VST3** and **Standalone**
- Covered in CI (`build-linux`); see [CI.md](CI.md)

## Next Steps

After building:

1. **Test the standalone app**

   ```bash
   open "build/GenerativeMIDI_artefacts/Release/Standalone/Generative MIDI.app"
   ```

2. **Validate AU plugin**

   ```bash
   auval -v aumi Osrc Gmid
   ```

3. **Test in your DAW**
   - Load in Ableton Live, Logic Pro, etc.
   - Insert as MIDI effect
   - Route to a synthesizer

## Build Performance

Typical build times on Apple Silicon:

- **Clean build**: ~2–3 minutes
- **Incremental build**: 10–30 seconds
- **Parallel build (-j8)**: ~1 minute

## Additional Resources

- [JUCE Documentation](https://docs.juce.com/)
- [CMake with JUCE](https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md)
- [Project README](../../README.md)
