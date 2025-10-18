# Building Generative MIDI for iOS/iPadOS

This guide explains how to build the Generative MIDI AUv3 plugin for iOS and iPadOS devices.

## Requirements

- macOS with Xcode installed (Xcode 14 or later recommended)
- Apple Developer account (free or paid) for code signing
- iOS device or simulator running iOS 13.0 or later
- CMake 3.15 or later

## Quick Start

1. **Generate the Xcode project:**
   ```bash
   ./build_ios.sh
   ```

2. **Open the Xcode project:**
   ```bash
   open build_ios/GenerativeMIDI.xcodeproj
   ```

3. **Configure in Xcode:**
   - Select the `GenerativeMIDI_AUv3` target from the scheme dropdown
   - Click on the project in the navigator
   - Select the `GenerativeMIDI_AUv3` target
   - Go to "Signing & Capabilities" tab
   - Select your Development Team from the dropdown
   - Xcode will automatically manage provisioning profiles

4. **Build and Run:**
   - Connect your iOS/iPadOS device via USB (or select a simulator)
   - Select your device from the device dropdown
   - Press Cmd+R to build and run

## Using the AUv3 Plugin

Once installed on your device, the Generative MIDI AUv3 plugin will be available in:

- **GarageBand** - Tap the Plugins button, go to Audio Units
- **AUM (Audio Mixer)** - Add as a MIDI effect
- **Cubasis** - MIDI FX section
- **Beatmaker 3** - Plugin chain
- **Any other AUv3 host app**

## Features

The iOS/iPadOS version includes:

- Full Euclidean rhythm generation
- Polyrhythmic sequencing (6 generators available)
- Algorithmic pattern generation (Markov, L-Systems, Cellular Automata)
- Real-time parameter control with touch-optimized UI
- Pattern visualization
- MIDI CC and expression support
- Works in both portrait and landscape orientations

## UI Considerations for iOS

The UI has been designed to work on iOS devices:

- **iPhone**: Best viewed in landscape orientation (900x600 minimum)
- **iPad**: Works in all orientations, optimal in landscape
- Touch controls are sized for finger input
- All knobs respond to drag gestures
- Sliders use standard iOS touch interactions

## Troubleshooting

### Code Signing Issues
If you encounter code signing errors:
1. Make sure you're logged into Xcode with your Apple ID (Preferences > Accounts)
2. Select "Automatically manage signing" in the target settings
3. If using a free account, change the bundle ID to something unique (e.g., `com.yourname.generativemidi`)

### Plugin Not Showing in Host Apps
1. Make sure the app successfully installed and launched at least once
2. Force quit and restart your host app (GarageBand, AUM, etc.)
3. Check that the host app supports AUv3 MIDI effects (not all do)

### Build Errors
- Clean the build: In Xcode, go to Product > Clean Build Folder
- Delete the `build_ios` folder and regenerate: `./build_ios.sh clean`
- Make sure JUCE is properly linked: `ls -la JUCE` should show a symlink to your JUCE installation

## Advanced Configuration

### Changing the Bundle ID
Edit `CMakeLists.txt` line 32:
```cmake
BUNDLE_ID "com.yourcompany.generativemidi"
```

### Changing Minimum iOS Version
Edit `build_ios.sh` line 28:
```bash
-DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
```

### Building for Specific Architecture
To build only for device (arm64):
```bash
cmake .. -G Xcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_ARCHITECTURES=arm64
```

## Testing in Simulator

The iOS Simulator can be used for UI testing, but note:
- Audio routing may not work perfectly in simulator
- MIDI functionality is limited
- For full testing, use a real device

## Distribution

To distribute your app:
1. You'll need a paid Apple Developer account ($99/year)
2. Archive the app in Xcode (Product > Archive)
3. Distribute to TestFlight for beta testing
4. Or submit to the App Store

For personal use only, you can sideload using a free account, but apps expire after 7 days.

## Support

For issues specific to iOS building, check:
- JUCE Forum: https://forum.juce.com
- Apple Developer Documentation: https://developer.apple.com/documentation/audiotoolbox/audio_unit_v3_plug-ins
