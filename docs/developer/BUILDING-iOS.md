# Building for iOS/iPadOS - Setup Guide

To build the Generative MIDI AUv3 plugin for iOS/iPadOS, you have two options:

## Option 1: Install Xcode (Recommended)

This is the standard approach for iOS development.

### Prerequisites
1. **Install Xcode** from the Mac App Store (free, but ~15GB download)
2. **Switch developer tools** to use Xcode instead of Command Line Tools:
   ```bash
   sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
   ```
3. **Accept Xcode license**:
   ```bash
   sudo xcodebuild -license accept
   ```

### Building
Once Xcode is installed:
```bash
./build_ios.sh
```

This will generate an Xcode project in `build_ios/`. Open it and build for your device.

See `README-iOS.md` for complete instructions.

---

## Option 2: Use GitHub Actions (Cloud Build)

If you don't want to install Xcode locally, you can use GitHub Actions to build in the cloud.

### Setup Steps

1. **Create GitHub repository** (if you haven't already):
   ```bash
   cd /Users/noisebox/Repos/GenerativeMIDI
   git init
   git add .
   git commit -m "Initial commit"
   gh repo create GenerativeMIDI --public --source=. --push
   ```

2. **Add iOS build workflow** - Create `.github/workflows/ios-build.yml`:
   ```yaml
   name: iOS Build

   on:
     push:
       branches: [ main ]
     workflow_dispatch:

   jobs:
     build-ios:
       runs-on: macos-latest

       steps:
       - uses: actions/checkout@v3
         with:
           submodules: recursive

       - name: Setup Xcode
         uses: maxim-lobanov/setup-xcode@v1
         with:
           xcode-version: latest-stable

       - name: Install JUCE
         run: |
           git clone https://github.com/juce-framework/JUCE.git ~/JUCE
           ln -s ~/JUCE JUCE

       - name: Generate Xcode Project
         run: |
           mkdir build_ios
           cd build_ios
           cmake .. -G Xcode \
             -DCMAKE_SYSTEM_NAME=iOS \
             -DCMAKE_OSX_SYSROOT=iphoneos \
             -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0

       - name: Build for iOS
         run: |
           cd build_ios
           xcodebuild -scheme GenerativeMIDI_AUv3 \
             -configuration Release \
             -destination 'generic/platform=iOS' \
             CODE_SIGN_IDENTITY="" \
             CODE_SIGNING_REQUIRED=NO \
             CODE_SIGNING_ALLOWED=NO

       - name: Upload IPA
         uses: actions/upload-artifact@v3
         with:
           name: GenerativeMIDI-iOS
           path: build_ios/**/*.app
   ```

3. **Push to GitHub**:
   ```bash
   git add .github/workflows/ios-build.yml
   git commit -m "Add iOS build workflow"
   git push
   ```

4. **Download built app** from GitHub Actions artifacts tab

### Note on Code Signing
The GitHub Actions build won't be code-signed, so you'll need to:
- Sideload using a tool like AltStore or Sideloadly
- Or: Set up proper code signing in the workflow with your Apple Developer certificates

---

## Option 3: Projucer (JUCE Native Tool)

JUCE includes the Projucer tool which can create iOS projects without CMake.

### Steps

1. **Download and run Projucer**:
   ```bash
   ~/JUCE/extras/Projucer/Builds/MacOSX/build/Debug/Projucer.app
   ```

2. **Create new Audio Plug-in project**:
   - File > New Project
   - Select "Audio Plug-In"
   - Set project name to "GenerativeMIDI"
   - Choose save location

3. **Configure project settings**:
   - Plugin Formats: Enable AUv3
   - Plugin Characteristics: Enable "Plugin is a MIDI Effect"
   - iOS Deployment Target: 13.0
   - Bundle Identifier: `com.opensource.generativemidi`

4. **Add source files** from this project's `Source/` directory

5. **Add module paths** to point to JUCE modules

6. **Select iOS target** and click "Save and Open in IDE"

7. **Build in Xcode**

---

## Current Status

Your system currently has:
- ✅ Command Line Tools installed
- ❌ Full Xcode installation (required for iOS building)

To check your current setup:
```bash
xcode-select -p
```

Should show: `/Applications/Xcode.app/Contents/Developer`
Currently shows: `/Library/Developer/CommandLineTools`

---

## Quick Decision Guide

**Choose Option 1 if:**
- You have 15GB+ free disk space
- You plan to do regular iOS development
- You want the fastest local builds
- You have good internet for initial Xcode download

**Choose Option 2 if:**
- Limited disk space
- Only occasionally need iOS builds
- Comfortable with Git/GitHub
- Don't mind waiting for cloud builds (~10-15 min)

**Choose Option 3 if:**
- You prefer JUCE's native workflow
- Want more control over project configuration
- Already familiar with Projucer

---

## Next Steps

1. Choose your preferred option above
2. Follow the setup instructions
3. Build the project
4. Transfer to your iOS device
5. Use in your favorite AUv3 host (GarageBand, AUM, Cubasis, etc.)

For detailed usage instructions once built, see `README-iOS.md`.
