#!/bin/bash

# Build script for iOS/iPadOS AUv3
# This creates an Xcode project that you can open and build for your device

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build_ios"

echo "=========================================="
echo "Generative MIDI - iOS/iPadOS AUv3 Builder"
echo "=========================================="
echo ""

# Check if Xcode is installed
if ! command -v xcodebuild &> /dev/null; then
    echo "ERROR: Xcode is not installed or not found in PATH"
    echo ""
    echo "To build for iOS, you need to:"
    echo "1. Install Xcode from the Mac App Store"
    echo "2. Run: sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer"
    echo ""
    exit 1
fi

# Check if command line tools are pointing to Xcode
DEVELOPER_DIR=$(xcode-select -p)
if [[ "$DEVELOPER_DIR" == *"CommandLineTools"* ]]; then
    echo "WARNING: xcode-select is pointing to Command Line Tools instead of Xcode"
    echo "Please run: sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer"
    echo ""
    exit 1
fi

# Clean previous build if requested
if [ "$1" == "clean" ]; then
    echo "Cleaning previous build..."
    rm -rf "${BUILD_DIR}"
fi

# Create build directory
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo "Generating Xcode project for iOS..."
echo "Using Xcode at: $DEVELOPER_DIR"
echo ""

# Generate Xcode project for iOS
# Note: Using simpler approach that lets Xcode handle most iOS-specific settings
cmake .. -G Xcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    "-DCMAKE_OSX_SYSROOT=iphoneos" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DCMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH=NO \
    -DCMAKE_IOS_INSTALL_COMBINED=YES

echo ""
echo "=========================================="
echo "Xcode project generated successfully!"
echo "=========================================="
echo ""
echo "Next steps:"
echo "1. Open the Xcode project:"
echo "   open ${BUILD_DIR}/GenerativeMIDI.xcodeproj"
echo ""
echo "2. In Xcode:"
echo "   - Select the 'GenerativeMIDI_AUv3' target"
echo "   - Set your Development Team in Signing & Capabilities"
echo "   - Select your iOS device or simulator"
echo "   - Build and run (Cmd+R)"
echo ""
echo "3. On your device:"
echo "   - The app will install and launch"
echo "   - The AUv3 plugin will be available in GarageBand, AUM, Cubasis, etc."
echo ""
echo "Note: You'll need to configure code signing with your Apple Developer account"
echo "      in Xcode before building."
echo ""
