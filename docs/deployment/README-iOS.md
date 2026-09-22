# Building Generative MIDI for iOS/iPadOS

This guide explains how to build the Generative MIDI AUv3 plugin for iOS and iPadOS devices.

> **Honesty note:** AUv3 is a configured CMake target useful for sideload / TestFlight-style workflows. It is **not App Store–ready**, and there is **no dedicated touch / accessibility redesign** claimed for the current Victorian desktop UI.

## Requirements

- macOS with Xcode installed (Xcode 14 or later recommended)
- Apple Developer account (free or paid) for code signing
- iOS device or simulator running iOS 13.0 or later
- CMake 3.15 or later
- JUCE linked at `./JUCE`
- `art/` submodule initialized: `git submodule update --init --recursive`

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

Once installed on your device, the Generative MIDI AUv3 plugin will be available in hosts that support AUv3 MIDI effects, for example:

- **GarageBand** - Plugins → Audio Units
- **AUM (Audio Mixer)** - MIDI effect slot
- Other AUv3 MIDI hosts (support varies)

## Features (same DSP / editors as desktop)

The iOS/iPadOS build shares the desktop engine set:

- **9 UI generators** (Euclidean, 4 algorithmic, 4 stochastic)
- **Polyrhythm experimental** in the editor (engine + minimal layer UI)
- Pattern visualization (Euclidean step grid; other modes show an honest empty state)
- Gate / ratchet / scale / humanization parameters
- **Not** store packaging, touch-optimized layout, or claimed full MIDI expression UI

## UI Considerations for iOS

The current UI is the desktop steampunk layout compiled for AUv3:

- Prefer **landscape** on phone-sized screens
- Knobs use JUCE drag gestures (works with touch, but not a mobile-first redesign)
- Expect further layout work before any store submission

## Troubleshooting

### Code Signing Issues
1. Sign in to Xcode with your Apple ID (Settings → Accounts)
2. Select "Automatically manage signing"
3. With a free account, use a unique bundle ID (e.g. `com.yourname.generativemidi`)

### Plugin Not Showing in Host Apps
1. Launch the containing app at least once after install
2. Force quit and restart the host
3. Confirm the host supports AUv3 MIDI effects

### Build Errors
- Product → Clean Build Folder
- Delete `build_ios` and regenerate: `./build_ios.sh clean`
- Confirm `ls -la JUCE` and `git submodule update --init --recursive`

## Advanced Configuration

### Changing the Bundle ID
Edit `CMakeLists.txt`:
```cmake
BUNDLE_ID "com.yourcompany.generativemidi"
```

### Changing Minimum iOS Version
Edit `build_ios.sh`:
```bash
-DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
```

## Distribution

App Store / TestFlight distribution requires a paid Apple Developer account, signing, and product polish beyond this repo’s current AUv3 target. For personal sideloading, free accounts typically expire after a short period.

## Support

- JUCE Forum: https://forum.juce.com
- Apple AUv3 docs: https://developer.apple.com/documentation/audiotoolbox/audio_unit_v3_plug-ins
- Project status: [STATUS.md](../../STATUS.md)
