# GenerativeMIDI Documentation

**Public face:** the hiring-facing GitHub Pages site —

- [Home](index.html) — ownership skim
- [Engineering](engineering.html) — architecture and proof table

Live URL (when Pages is enabled): **https://joshband.github.io/GenerativeMIDI/**

The markdown below is **internal / developer evidence** (build notes, design specs, session logs). Prefer the HTML pages for employer-facing narrative; use these files for drill-down detail.

---

## Documentation Index

### For Users

- **[Getting Started](user/GETTING_STARTED.md)** - Quick start guide for end users
- **[Features Guide](user/FEATURES.md)** - Complete feature list and usage instructions
- **[REAPER Quick Start](user/REAPER_QUICK_START.md)** - DAW-specific integration guide

### For Developers

- **[Build Instructions](developer/BUILD.md)** - How to build the plugin from source
- **[iOS Build Guide](developer/BUILDING-iOS.md)** - Building for iOS/iPadOS (AUv3)
- **[Changelog Guide](developer/CHANGELOG_GUIDE.md)** - How to write changelog entries
- **[Enhancement Roadmap](developer/ENHANCEMENTS.md)** - Planned features and improvements

### Design Documentation

- **[Design Index](design/README.md)** - Complete design documentation index
- **[UI Specification](design/SYNAPTIK_UI_SPEC.md)** - Complete UI design specification
- **[Color Palettes](design/COLOR_PALETTE.md)** - Color system and usage guidelines
- **[Component Specs](design/COMPONENT_SPECS.md)** - Individual component specifications
- **[Project Summary](design/PROJECT_SUMMARY.md)** - High-level project overview

### Deployment

- **[Deployment Summary](deployment/DEPLOYMENT_COMPLETE.md)** - Deployment process summary
- **[iOS Loading Guide](deployment/LOAD_IOS_GUIDE.md)** - How to load plugin on iOS devices
- **[iOS README](deployment/README-iOS.md)** - iOS-specific documentation

### Development Sessions

- **[Session Index](sessions/README.md)** - Day-by-day development logs
- [2025-10-17](sessions/2025-10-17.md) - Gate Length Control (v0.4.0)
- [2025-10-18](sessions/2025-10-18.md) - Ratcheting (v0.5.0), UI Fixes (v0.5.1), Asset Catalog

### Showcase design

- [Pages showcase design spec](superpowers/specs/2026-09-21-github-pages-showcase-design.md)

---

## Quick Links

| I want to... | Go to... |
|--------------|----------|
| Skim the hiring site | [Home](index.html) |
| Read the engineering case study | [Engineering](engineering.html) |
| Use the plugin | [Getting Started](user/GETTING_STARTED.md) |
| Build from source | [Build Instructions](developer/BUILD.md) |
| Build for iOS | [iOS Build Guide](developer/BUILDING-iOS.md) |
| Understand the features | [Features Guide](user/FEATURES.md) |
| See what's planned | [Enhancement Roadmap](developer/ENHANCEMENTS.md) |
| View UI design | [UI Specification](design/SYNAPTIK_UI_SPEC.md) |

---

## Repository Structure

```
GenerativeMIDI/
├── docs/              # Pages site + markdown evidence (you are here)
│   ├── index.html
│   ├── engineering.html
│   └── assets/
├── art/               # Visual assets and design files (submodule)
├── Source/            # C++ source code
├── Resources/         # Embedded resources
├── CHANGELOG.md       # Version history
├── STATUS.md          # Current project status
└── README.md          # Main project readme
```

---

**Last Updated**: 2026-09-21
