# Building Generative MIDI for iOS/iPadOS

This guide explains how to build the Generative MIDI AUv3 plugin for iOS and iPadOS devices.

> **Honesty note:** AUv3 is a configured CMake target useful for sideload / TestFlight-style workflows. It is **not App Store–ready**, and there is **no dedicated touch / accessibility redesign** claimed for the current Victorian desktop UI.

## Requirements

- macOS with Xcode installed (Xcode 14 or later recommended)
- Apple Developer account (free or paid) for code signing
- iOS device or simulator running iOS 15.0 or later / iPadOS 15.0 or later
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

- **10 UI generators** (Euclidean, Polyrhythm experimental, 4 algorithmic, 4 stochastic)
- Pattern visualization (Euclidean step grid; other modes show activity fallback)
- Gate / ratchet / scale / humanization / expression / LFO MVP parameters
- **Not** store packaging or a dedicated mobile-first redesign

## UI Considerations for iOS

The editor is the desktop brass + cyan-on-navy layout with cross-format usability tweaks:

- Default / minimum sizes are DAW-friendly; content scrolls vertically when the host frame is shorter than the preferred height
- Prefer **landscape** on phone-sized screens
- Touch: slightly larger hit targets (`JUCE_IOS`) for buttons, popup items, and preset rows; knobs still use drag gestures
- AX titles remain on key controls
- Expect further layout work before any store submission

## Format notes (macOS vs iOS)

| Format | Where it builds | Notes |
|--------|-----------------|-------|
| AU / VST3 / Standalone | macOS CMake (`build/`) | Primary desktop hosts; editor scrolls in short frames |
| AUv3 | **iOS** CMake (`build_ios.sh` / `CMAKE_SYSTEM_NAME=iOS`) | JUCE sets `JucePlugin_Build_AUv3=0` on desktop macOS MIDI-FX configs; use the iOS target. iOS also builds **Standalone** for Simulator editor QA (embeds `.appex`). |
| AUv3 Simulator QA | iPad Simulator (see `docs/qa/logs/auv3_sim_qa.txt`) | Build + launch + layout + MIDI Log live events verified 2026-09-22 |
| AUv3 device QA | Physical iPad/iPhone | Remaining human step when a signed device is available — not claimed store-ready |

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
- Current Xcode may require **iOS 15.0+** deployment target (see `build_ios.sh`)

## Advanced Configuration

### Changing the Bundle ID
Edit `CMakeLists.txt`:
```cmake
BUNDLE_ID "com.yourcompany.generativemidi"
```

### Changing Minimum iOS Version
Edit `build_ios.sh`:
```bash
-DCMAKE_OSX_DEPLOYMENT_TARGET=15.0 \
```

## Distribution

App Store / TestFlight distribution requires a paid Apple Developer account, signing, and product polish beyond this repo’s current AUv3 target. For personal sideloading, free accounts typically expire after a short period.

## Support

- JUCE Forum: https://forum.juce.com
- Apple AUv3 docs: https://developer.apple.com/documentation/audiotoolbox/audio_unit_v3_plug-ins
- Project status: [STATUS.md](../../STATUS.md)
