# Generative MIDI

A powerful open-source generative MIDI processor with 10 algorithmic engines for creating Euclidean rhythms, polyrhythmic sequences, and complex algorithmic patterns. Available as AU, VST3, and standalone application for macOS.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20iOS%20%7C%20iPadOS-lightgrey.svg)](https://github.com/joshband/GenerativeMIDI/releases)
[![Release](https://img.shields.io/github/v/release/joshband/GenerativeMIDI)](https://github.com/joshband/GenerativeMIDI/releases/latest)

![Generative MIDI Interface](docs/design/screenshots/main-interface.png)

---

## 🎵 Quick Start

### Installation (macOS)

1. **Download the latest release:**
   - Go to [Releases](https://github.com/joshband/GenerativeMIDI/releases/latest)
   - Download `GenerativeMIDI-v1.0.0-macOS.dmg`

2. **Mount the DMG:**
   - Double-click the downloaded DMG file
   - The installer will open

3. **Install plugins:**

   **Option A - Terminal (recommended):**
   ```bash
   cd "/Volumes/Generative MIDI v1.0.0"
   cp -R "Generative MIDI.component" ~/Library/Audio/Plug-Ins/Components/
   cp -R "Generative MIDI.vst3" ~/Library/Audio/Plug-Ins/VST3/
   ```

   **Option B - Manual:**
   - Open Finder and press `Cmd+Shift+G`
   - Navigate to `~/Library/Audio/Plug-Ins/`
   - Drag `Generative MIDI.component` to the `Components` folder
   - Drag `Generative MIDI.vst3` to the `VST3` folder

4. **Run standalone app (optional):**
   - Double-click `Generative MIDI.app` in the DMG
   - Or copy it to your Applications folder

5. **Verify installation (AU only):**
   ```bash
   auval -v aumi Gmid Osrc
   ```

### System Requirements

- macOS 10.15 (Catalina) or later
- 64-bit Intel or Apple Silicon (M1/M2/M3)
- Compatible DAW (Logic Pro, Ableton Live, Reaper, GarageBand, etc.)

---

## ✨ Features

### 10 Generative Engines

#### Rhythmic Generators
- **Euclidean Rhythm** - Björklund's algorithm for perfectly distributed rhythmic patterns
- **Polyrhythm** - Multi-layer polyrhythmic sequencer with independent timing divisions

#### Algorithmic Generators
- **Markov Chain** - Melodic generation based on probability matrices
- **L-System** - Fractal pattern evolution using Lindenmayer systems
- **Cellular Automata** - Pattern generation using Wolfram rules
- **Probabilistic** - Stochastic note generation with density control

#### Stochastic/Chaos Generators
- **Brownian Motion** - Random walk with momentum and inertia
- **Perlin Noise** - Smooth, natural randomness
- **Drunk Walk** - Discrete random steps
- **Lorenz Attractor** - Deterministic chaos systems

### Musical Features

- **16 Scale Types** - Major, Minor, Modes, Pentatonic, Blues, Whole Tone, Chromatic
- **MIDI Channel Routing** - Route to any MIDI channel (1-16)
- **Swing & Humanization** - 6 swing groove templates, timing & velocity humanization
- **Full MIDI Expression** - Velocity, aftertouch, CC, pitch bend
- **Gate & Ratcheting** - Gate length control with legato mode, note retriggering
- **Real-time Visualization** - Live playback position and pattern display
- **31 Parameters** - Full DAW automation support

### User Interface

- **Gilded Steampunk Victorian Theme** - Brass, gold, and copper aesthetic
- **Color-Coded Generators** - Visual feedback by engine type
- **Context-Aware Controls** - Relevant controls auto-enable/disable
- **Resizable Window** - 1200x500 to 2000x1000 pixels
- **Touch-Optimized** - Ready for iOS/iPadOS

---

## 🎹 Usage

### Basic Workflow

1. **Load the plugin** in your DAW as a MIDI effect
2. **Select a generator** from the dropdown menu
3. **Adjust parameters:**
   - **Tempo** - Set BPM (20-400)
   - **Steps/Pulses** - Configure pattern length and density
   - **Scale/Root** - Choose musical scale and key
   - **Velocity/Pitch** - Set output ranges
4. **Route to a synth** - Connect MIDI output to any instrument
5. **Play!** - Press play in your DAW

### Generator-Specific Controls

**Euclidean Mode:**
- **Steps** (1-64) - Total pattern length
- **Pulses** (0-64) - Number of active beats
- **Rotation** (0-64) - Pattern offset

**Algorithmic Modes:**
- **Density** (0.0-1.0) - Note generation probability

**Stochastic Modes:**
- **Step Size** (0.01-1.0) - Randomness magnitude
- **Momentum** (0.0-1.0) - Inertia/friction
- **Time Scale** (0.01-10.0) - Evolution speed

### Quick Tips

- Use **Euclidean** for tight, rhythmic patterns
- Use **Markov Chain** for melodic sequences that evolve
- Use **Lorenz Attractor** for unpredictable, chaotic melodies
- Combine multiple instances on different MIDI channels for layered complexity
- Automate parameters in your DAW for evolving patterns

---

## 🛠️ Building from Source

### Prerequisites

- macOS 10.15 or later
- [CMake](https://cmake.org) 3.15+
- [JUCE Framework](https://juce.com) 7.0+
- Xcode Command Line Tools: `xcode-select --install`

### Build Steps

```bash
# Clone the repository
git clone https://github.com/joshband/GenerativeMIDI.git
cd GenerativeMIDI

# Link JUCE (adjust path to your JUCE installation)
ln -s ~/JUCE JUCE

# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build (use -j8 for parallel build)
cmake --build . --config Release -j8

# Plugins are built to:
# - build/GenerativeMIDI_artefacts/Release/AU/Generative MIDI.component
# - build/GenerativeMIDI_artefacts/Release/VST3/Generative MIDI.vst3
# - build/GenerativeMIDI_artefacts/Release/Standalone/Generative MIDI.app
```

### iOS/iPadOS Build

See [docs/developer/BUILDING-iOS.md](docs/developer/BUILDING-iOS.md) for detailed iOS build instructions.

```bash
./build_ios.sh
open build_ios/GenerativeMIDI.xcodeproj
# Configure code signing in Xcode and build to your device
```

---

## 📖 Documentation

Complete documentation is available in the [docs/](docs/) directory:

### User Documentation
- **[Getting Started Guide](docs/user/GETTING_STARTED.md)** - Quick start tutorial
- **[Features Guide](docs/user/FEATURES.md)** - Complete feature reference
- **[Preset Guide](docs/user/PRESET_GUIDE.md)** - Creating and managing presets
- **[REAPER Quick Start](docs/user/REAPER_QUICK_START.md)** - REAPER-specific setup

### Developer Documentation
- **[Build Instructions](docs/developer/BUILD.md)** - Detailed build guide
- **[Enhancement Roadmap](docs/developer/ENHANCEMENTS.md)** - Planned features
- **[Changelog](CHANGELOG.md)** - Version history

### Design Documentation
- **[UI Specification](docs/design/SYNAPTIK_UI_SPEC.md)** - Interface design
- **[Color Palette](docs/design/COLOR_PALETTE.md)** - Visual design system
- **[Component Specs](docs/design/COMPONENT_SPECS.md)** - UI components

---

## 🏗️ Architecture

### Core Components

**Engines:**
- `EuclideanEngine` - Björklund algorithm implementation
- `PolyrhythmEngine` - Multi-layer rhythm sequencer
- `AlgorithmicEngine` - Markov, L-System, Cellular, Probabilistic
- `StochasticEngine` - Brownian, Perlin, Drunk Walk, Lorenz
- `MIDIGenerator` - MIDI message creation with full expression

**DSP:**
- `ClockManager` - Tempo control and external MIDI sync
- `EventScheduler` - Sample-accurate event scheduling

**UI:**
- `CustomLookAndFeel` - Victorian steampunk theme rendering
- `PatternVisualizer` - Real-time pattern display
- `PluginEditor` - Main interface with parameter controls

### Technical Specifications

- **Sample Rates:** 44.1, 48, 88.2, 96 kHz
- **MIDI Channels:** 1-16
- **Buffer Size:** Adaptive (32-2048 samples)
- **Latency:** < 5ms typical
- **CPU Usage:** < 5% on modern hardware
- **Memory:** ~10MB RAM

---

## 🤝 Contributing

Contributions are welcome! Whether you're fixing bugs, adding features, or improving documentation.

### How to Contribute

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Make your changes
4. Run tests: `cd build && ctest`
5. Format code: `clang-format -i Source/**/*.cpp Source/**/*.h`
6. Commit: `git commit -m "Add amazing feature"`
7. Push: `git push origin feature/amazing-feature`
8. Open a Pull Request

### Development Setup

```bash
# Install pre-commit hooks
pre-commit install

# Run tests
cd build
ctest

# Format code
clang-format -i Source/**/*.cpp Source/**/*.h
```

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

---

## 🗺️ Roadmap

- [ ] Windows VST3 build
- [ ] Linux builds (VST3, LV2)
- [ ] Additional algorithmic generators (Fibonacci, Conway's Game of Life)
- [ ] MIDI input processing and transformation
- [ ] MPE (MIDI Polyphonic Expression) support
- [ ] Expanded preset library
- [ ] Standalone app with internal synth

---

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

Free to use, modify, and distribute for personal and commercial projects.

---

## 🙏 Credits

### Algorithm References
- **Euclidean Rhythms:** Godfried Toussaint (2005)
- **Björklund's Algorithm:** E. Björklund (2003)
- **L-Systems:** Aristid Lindenmayer (1968)
- **Cellular Automata:** Stephen Wolfram (1983)

### Built With
- [JUCE Framework](https://juce.com) - Cross-platform audio framework
- [CMake](https://cmake.org) - Build system

---

## 💬 Support

- 📖 **Documentation:** [docs/](docs/)
- 🐛 **Bug Reports:** [GitHub Issues](https://github.com/joshband/GenerativeMIDI/issues)
- 💡 **Feature Requests:** [GitHub Discussions](https://github.com/joshband/GenerativeMIDI/discussions)
- 📥 **Latest Release:** [Download here](https://github.com/joshband/GenerativeMIDI/releases/latest)

---

**Made with ❤️ for the open-source music community**
