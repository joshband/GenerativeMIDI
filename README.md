# Generative MIDI

An open-source, cross-platform generative MIDI processor supporting Euclidean rhythms, polyrhythmic sequencing, and algorithmic pattern generation.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20iOS%20%7C%20iPadOS-lightgrey.svg)

## Features

### Generative Engines (10 Total)
- **Euclidean Rhythm Generator** - Björklund's algorithm for creating perfectly distributed rhythmic patterns
- **Polyrhythm Engine** - Multi-layer polyrhythmic sequencer with independent timing divisions
- **Algorithmic Generators**:
  - Markov Chain melodic generation
  - L-System fractal pattern evolution
  - Cellular Automata (Wolfram rules)
  - Probabilistic note generation
- **Stochastic/Chaos Generators** (NEW in v0.7.0):
  - Brownian Motion - Random walk with momentum
  - Perlin Noise - Smooth natural randomness
  - Drunk Walk - Discrete random steps
  - Lorenz Attractor - Deterministic chaos

### MIDI Capabilities
- **MIDI Channel Routing** (NEW in v0.8.0) - Route to any MIDI channel (1-16)
- Full MIDI 1.0 expression support
- Note generation with velocity control
- Polyphonic aftertouch
- CC (Control Change) output
- Pitch bend
- Sample-accurate timing
- Real-time parameter automation
- Gate length control with legato mode
- Ratcheting (note retriggering)

### Musical Features
- 16 scale types (Major, Minor, Modes, Pentatonic, Blues, etc.)
- Root note selection (C-B)
- 6 swing groove templates
- Timing humanization (0-50ms)
- Velocity humanization (0-100%)
- Pitch and velocity range control
- Preset management system

### User Interface
- **Gilded Steampunk Victorian Aesthetic** - Brass, gold, copper theme
- **Color-Coded Generators** (NEW in v0.8.0) - Visual feedback by engine type
- **Context-Aware Controls** (NEW in v0.7.1) - Relevant controls auto-enable/disable
- **Live Playback Visualization** (NEW in v0.8.0) - Real-time pattern position display
- Rotary brass knobs with aether crystal centers
- Real-time pattern visualization with probability indication
- Resizable window (1200x500 to 2000x1000)
- Touch-optimized for iOS/iPadOS
- 31 parameters with full automation support

## Platform Support

### Desktop (macOS)
- ✅ **AU** (Audio Unit)
- ✅ **VST3**
- ✅ **Standalone** application

### Mobile (iOS/iPadOS)
- ✅ **AUv3** (Audio Unit v3)
- Compatible with:
  - GarageBand
  - AUM (Audio Mixer)
  - Cubasis
  - Beatmaker 3
  - Any AUv3 host

## Building from Source

### Prerequisites
- macOS 10.15 or later
- CMake 3.15 or later
- JUCE Framework 7.0+
- For iOS: Xcode with iOS SDK

### macOS Plugins

```bash
# Clone and setup
git clone https://github.com/yourusername/GenerativeMIDI.git
cd GenerativeMIDI
ln -s ~/JUCE JUCE  # Link to your JUCE installation

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j8

# Plugins installed to:
# ~/Library/Audio/Plug-Ins/Components/Generative MIDI.component  (AU)
# ~/Library/Audio/Plug-Ins/VST3/Generative MIDI.vst3            (VST3)
# build/GenerativeMIDI_artefacts/Release/Standalone/            (Standalone)
```

### iOS/iPadOS AUv3

See [docs/developer/BUILDING-iOS.md](docs/developer/BUILDING-iOS.md) for detailed iOS build instructions.

**Quick start (requires Xcode):**
```bash
./build_ios.sh
open build_ios/GenerativeMIDI.xcodeproj
# Configure code signing in Xcode and build to your device
```

**Cloud build (GitHub Actions):**
Push to GitHub and builds are automatically created. Download from Actions tab.

## Usage

### Parameter Guide

**Generator Type** - Select rhythm generation algorithm:
- Euclidean
- Polyrhythm
- Markov Chain
- L-System
- Cellular Automaton
- Probabilistic

**Tempo** (20-400 BPM) - Master tempo control

**Euclidean Controls:**
- **Steps** (1-64) - Total steps in pattern
- **Pulses** (0-64) - Number of active beats
- **Rotation** (0-64) - Rotate pattern offset

**Density** (0.0-1.0) - Note generation probability (for algorithmic modes)

**Velocity Range** - Min/Max velocity for generated notes

**Pitch Range** - Min/Max MIDI note numbers (0-127)

### Pattern Visualization

The pattern display shows:
- Active steps in cyan (#00d4ff)
- Inactive steps dimmed
- Current playhead highlighted
- Step numbers on 4-beat intervals

## Architecture

### Core Components

**Engines:**
- `EuclideanEngine` - Björklund algorithm implementation
- `PolyrhythmEngine` - Multi-layer rhythm sequencer
- `AlgorithmicEngine` - Markov, L-System, Cellular, Probabilistic generators
- `MIDIGenerator` - MIDI message creation with full expression

**DSP:**
- `ClockManager` - Tempo control and external MIDI sync
- `EventScheduler` - Sample-accurate event scheduling with priority queue

**UI:**
- `CustomLookAndFeel` - Modern dark theme with custom knob rendering
- `PatternVisualizer` - Real-time pattern display component
- `PluginEditor` - Main interface with parameter controls

### Plugin Architecture
```
PluginProcessor (AudioProcessor)
├── ClockManager (timing)
├── EventScheduler (MIDI output)
├── EuclideanEngine
├── PolyrhythmEngine
└── AlgorithmicEngine
    ├── MarkovChain
    ├── LSystem
    ├── CellularAutomaton
    └── ProbabilisticGenerator
```

## Technical Details

- **Sample Rate:** 44.1/48/88.2/96 kHz supported
- **MIDI Channels:** 1-16
- **Buffer Size:** Adaptive (32-2048 samples)
- **Latency:** < 5ms typical
- **CPU Usage:** < 5% on modern hardware
- **Memory:** ~10MB RAM

## License

MIT License - See [LICENSE](LICENSE) for details

## Contributing

Contributions welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

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

## Roadmap

- [ ] Additional algorithmic generators (Brownian motion, Fibonacci)
- [ ] MIDI input processing and transformation
- [ ] Preset management system
- [ ] Standalone app with internal synth
- [ ] MPE (MIDI Polyphonic Expression) support
- [ ] Windows VST3 build
- [ ] Linux builds

## Credits

**Algorithm References:**
- Euclidean rhythms: Godfried Toussaint (2005)
- Björklund's algorithm: E. Björklund (2003)
- L-Systems: Aristid Lindenmayer (1968)
- Cellular Automata: Stephen Wolfram (1983)

**Built With:**
- [JUCE Framework](https://juce.com) - Cross-platform audio framework
- [CMake](https://cmake.org) - Build system

## Documentation

Complete documentation is available in [docs/](docs/):

- **[Getting Started](docs/user/GETTING_STARTED.md)** - Quick start for users
- **[Features Guide](docs/user/FEATURES.md)** - Complete feature reference
- **[Build Instructions](docs/developer/BUILD.md)** - Developer build guide
- **[Enhancement Roadmap](docs/developer/ENHANCEMENTS.md)** - Planned features
- **[UI Specification](docs/design/SYNAPTIK_UI_SPEC.md)** - Design documentation
- **[Development Sessions](docs/sessions/README.md)** - Day-by-day development logs

## Support

- 📖 Full Documentation: [docs/](docs/)
- 🎨 Art Assets: [art/](art/)
- 🐛 Bug Reports: [GitHub Issues](https://github.com/yourusername/GenerativeMIDI/issues)
- 💬 Discussions: [GitHub Discussions](https://github.com/yourusername/GenerativeMIDI/discussions)

---

**Made with ❤️ for the open-source music community**
