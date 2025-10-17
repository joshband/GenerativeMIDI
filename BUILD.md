# Build Instructions

## Quick Start - CMake Build (Recommended)

This is the fastest way to build the plugin:

```bash
cd /Users/noisebox/Repos/GenerativeMIDI

# Create build directory
mkdir build && cd build

# Configure (Release build)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release

# Or for faster parallel build
cmake --build . --config Release -j8
```

### Build outputs will be in:
- **Standalone**: `build/GenerativeMIDI_artefacts/Release/Standalone/Generative MIDI.app`
- **AU**: `build/GenerativeMIDI_artefacts/Release/AU/Generative MIDI.component`
- **VST3**: `build/GenerativeMIDI_artefacts/Release/VST3/Generative MIDI.vst3`
- **AUv3**: `build/GenerativeMIDI_artefacts/Release/AUv3/Generative MIDI.appex`

## Alternative Method - Projucer + Xcode

### 1. Open in Projucer
```bash
open -a ~/JUCE/Projucer.app GenerativeMIDI.jucer
```

### 2. Configure JUCE Module Paths
In Projucer:
- Click "File" → "Global Paths..."
- Set "Path to JUCE" to `/Users/noisebox/JUCE`
- Or use the symlink that's already created in the project

### 3. Save and Open in Xcode
- Click "File" → "Save Project and Open in IDE..." (or Cmd+P)
- This will generate the Xcode project and open it

### 4. Build in Xcode
- Select target: "GenerativeMIDI - All" or specific format (AU, VST3, Standalone)
- Build configuration: "Release"
- Click Product → Build (or Cmd+B)

## Troubleshooting

### Error: JUCE not found
```bash
cd /Users/noisebox/Repos/GenerativeMIDI
ln -s ~/JUCE JUCE
```

### Error: Missing module paths in Projucer
- Open Projucer's global settings
- Set JUCE path to `/Users/noisebox/JUCE`
- Resave the project

### Compiler Errors
Make sure you have:
- Xcode 14+ installed
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

For development with debugging symbols:

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

### macOS (Current Platform)
- **AU** (Audio Units): Native macOS format
- **AUv3**: iOS/macOS App Extension format
- **VST3**: Cross-platform format
- **Standalone**: Runs independently

### Windows (Cross-compile or native)
- Use Visual Studio 2022
- Build VST3 and Standalone
- AU/AUv3 not available

### Linux
- Use Linux Makefile exporter in Projucer
- Or use CMake directly
- VST3 and Standalone supported

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
- **Clean build**: ~2-3 minutes
- **Incremental build**: 10-30 seconds
- **Parallel build (-j8)**: ~1 minute

## Additional Resources

- [JUCE Documentation](https://docs.juce.com/)
- [CMake with JUCE](https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md)
- [Project README](README.md)
