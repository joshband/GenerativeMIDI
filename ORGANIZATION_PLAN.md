# Project Organization Plan

**Created**: 2025-10-18
**Purpose**: Reorganize documentation and assets for better maintainability

---

## Current Problems

### Documentation Issues
1. **Root directory clutter**: 18+ markdown files in project root
2. **No clear hierarchy**: User docs mixed with dev docs mixed with guides
3. **Session files**: Development sessions should be in their own directory
4. **Duplicate content**: Some overlap between files

### Asset Issues
1. **conceptArt/ is flat**: 147 JPG files in one directory
2. **Mixed purposes**: Original assets, extracted PNGs, reference panels all together
3. **No categorization**: Hard to find specific asset types
4. **Documentation mixed in**: MD files alongside images

---

## Proposed New Structure

```
GenerativeMIDI/
├── README.md                          # Main project readme (stays in root)
├── LICENSE                            # License file (stays in root)
├── CMakeLists.txt                     # Build config (stays in root)
│
├── docs/                              # 📚 ALL DOCUMENTATION
│   ├── README.md                      # Documentation index/navigation
│   │
│   ├── user/                          # End-user documentation
│   │   ├── GETTING_STARTED.md         # Quick start for users
│   │   ├── FEATURES.md                # Feature list and usage
│   │   ├── REAPER_QUICK_START.md      # DAW-specific guide
│   │   └── FAQ.md                     # Common questions (to create)
│   │
│   ├── developer/                     # Developer documentation
│   │   ├── BUILD.md                   # Build instructions
│   │   ├── BUILDING-iOS.md            # iOS build guide
│   │   ├── CHANGELOG_GUIDE.md         # How to write changelogs
│   │   ├── ENHANCEMENTS.md            # Planned enhancements
│   │   └── ARCHITECTURE.md            # Code architecture (to create)
│   │
│   ├── deployment/                    # Deployment documentation
│   │   ├── DEPLOYMENT_COMPLETE.md     # Deployment summary
│   │   ├── LOAD_IOS_GUIDE.md          # iOS loading instructions
│   │   └── README-iOS.md              # iOS-specific readme
│   │
│   ├── design/                        # Design documentation
│   │   ├── SYNAPTIK_UI_SPEC.md        # UI specification
│   │   ├── PROJECT_SUMMARY.md         # Project overview
│   │   └── COLOR_PALETTE.md           # Color reference (to create)
│   │
│   └── sessions/                      # Development session logs
│       ├── README.md                  # Session index
│       ├── 2025-10-17.md              # Yesterday's session
│       ├── 2025-10-18.md              # Today's session
│       └── template.md                # Session template (to create)
│
├── art/                               # 🎨 ALL VISUAL ASSETS (renamed from conceptArt)
│   ├── README.md                      # Art assets index
│   │
│   ├── original/                      # Original AI-generated assets
│   │   ├── panels/                    # Full control panel mockups
│   │   │   ├── IMG_8119.JPG           # SYNAPTIK panel
│   │   │   ├── IMG_8120.JPG           # SYNNATIKS panel
│   │   │   └── ...
│   │   │
│   │   ├── knobs/                     # Knob designs
│   │   │   ├── IMG_8150.JPG           # Ornate filigree
│   │   │   ├── IMG_8183.JPG           # Astrolabe rings
│   │   │   ├── IMG_8200.JPG           # Clock face
│   │   │   └── ...
│   │   │
│   │   ├── frames/                    # Frame and bezel designs
│   │   │   ├── IMG_8125.JPG           # Art Deco panel
│   │   │   ├── IMG_8135.JPG           # Oval Victorian
│   │   │   └── ...
│   │   │
│   │   ├── sliders/                   # Slider designs
│   │   │   ├── IMG_8250.JPG           # Brass vertical
│   │   │   └── ...
│   │   │
│   │   ├── buttons/                   # Button designs
│   │   │   ├── IMG_8168.JPG           # Cross ornate
│   │   │   └── ...
│   │   │
│   │   └── decorative/                # Decorative elements
│   │       ├── IMG_8213.JPG           # Honeycomb
│   │       └── ...
│   │
│   ├── extracted/                     # Extracted PNG assets (from original)
│   │   ├── knobs/
│   │   │   ├── knob_ornate_filigree_raw.png
│   │   │   ├── knob_astrolabe_rings_raw.png
│   │   │   └── ...
│   │   ├── frames/
│   │   ├── buttons/
│   │   ├── sliders/
│   │   └── decorative/
│   │
│   ├── ui-ready/                      # Production-ready assets (multiple sizes)
│   │   ├── knobs/
│   │   │   ├── knob_ornate_filigree_64.png
│   │   │   ├── knob_ornate_filigree_128.png
│   │   │   ├── knob_ornate_filigree_256.png
│   │   │   └── ...
│   │   ├── frames/
│   │   ├── buttons/
│   │   ├── sliders/
│   │   └── decorative/
│   │
│   ├── icons/                         # App icons and branding
│   │   ├── icon-*.svg                 # SVG icons
│   │   └── ...
│   │
│   ├── mockups/                       # UI mockups and references
│   │   └── ...
│   │
│   ├── tools/                         # Asset processing tools
│   │   ├── extract_priority_assets.sh
│   │   ├── extract_priority_assets.py
│   │   ├── batch_process.sh           # (to create)
│   │   └── README.md                  # Tool documentation
│   │
│   └── catalog/                       # Asset documentation
│       ├── ASSET_CATALOG.md
│       ├── QUICK_START.md
│       ├── JUCE_INTEGRATION_EXAMPLE.cpp
│       └── asset-map.json             # Searchable asset index (to create)
│
├── Resources/                         # Build resources (embedded in binary)
│   └── assets/                        # Currently used UI assets
│       ├── knobs/
│       ├── frames/
│       ├── buttons/
│       └── sliders/
│
├── Source/                            # Source code (stays as-is)
│   ├── Core/
│   ├── UI/
│   └── ...
│
├── CHANGELOG.md                       # Changelog (stays in root for visibility)
└── STATUS.md                          # Project status (stays in root)
```

---

## Migration Plan

### Phase 1: Create New Directory Structure (5 minutes)
```bash
# Create documentation structure
mkdir -p docs/{user,developer,deployment,design,sessions}

# Create art structure
mkdir -p art/{original/{panels,knobs,frames,sliders,buttons,decorative},extracted,ui-ready,icons,mockups,tools,catalog}
```

### Phase 2: Move Documentation Files (10 minutes)

**To docs/user/**:
- GETTING_STARTED.md
- FEATURES.md
- REAPER_QUICK_START.md

**To docs/developer/**:
- BUILD.md
- BUILDING-iOS.md
- CHANGELOG_GUIDE.md
- ENHANCEMENTS.md

**To docs/deployment/**:
- DEPLOYMENT_COMPLETE.md
- LOAD_IOS_GUIDE.md
- README-iOS.md

**To docs/design/**:
- SYNAPTIK_UI_SPEC.md
- PROJECT_SUMMARY.md

**To docs/sessions/**:
- DEV_SESSION_2025-10-17.md → 2025-10-17.md
- DEV_SESSION_2025-10-18.md → 2025-10-18.md

### Phase 3: Organize Art Assets (15 minutes)

**Sort original JPGs into categories**:
```bash
# Use the ASSET_CATALOG.md to categorize original JPGs
# Move panel mockups to art/original/panels/
# Move knob designs to art/original/knobs/
# etc.
```

**Move extracted assets**:
```bash
# Move conceptArt/extracted/* to art/extracted/
```

**Move UI-ready assets**:
```bash
# Move Resources/assets/* to art/ui-ready/
# Keep symlink/copy in Resources/ for build
```

**Move tools and documentation**:
```bash
# Move extract_*.sh/py to art/tools/
# Move ASSET_CATALOG.md to art/catalog/
# Move QUICK_START.md to art/catalog/
# Move JUCE_INTEGRATION_EXAMPLE.cpp to art/catalog/
```

### Phase 4: Create Index Files (10 minutes)

**docs/README.md**:
```markdown
# GenerativeMIDI Documentation

## For Users
- [Getting Started](user/GETTING_STARTED.md)
- [Features Guide](user/FEATURES.md)
- [REAPER Quick Start](user/REAPER_QUICK_START.md)

## For Developers
- [Build Instructions](developer/BUILD.md)
- [iOS Build Guide](developer/BUILDING-iOS.md)
- [Enhancement Roadmap](developer/ENHANCEMENTS.md)

## Design
- [UI Specification](design/SYNAPTIK_UI_SPEC.md)

## Deployment
- [iOS Deployment](deployment/LOAD_IOS_GUIDE.md)

## Development Sessions
- [Session Index](sessions/README.md)
```

**art/README.md**:
```markdown
# GenerativeMIDI Art Assets

## Asset Organization
- `original/` - AI-generated source images (147 JPGs organized by type)
- `extracted/` - Processed PNGs with transparency (raw extractions)
- `ui-ready/` - Production assets at multiple resolutions (64/128/256px)
- `icons/` - App icons and branding
- `mockups/` - UI mockups and design references
- `tools/` - Asset extraction and processing scripts
- `catalog/` - Asset documentation and integration guides

## Quick Links
- [Asset Catalog](catalog/ASSET_CATALOG.md) - Complete asset reference
- [Quick Start](catalog/QUICK_START.md) - 15-minute integration guide
- [Extraction Tools](tools/README.md) - How to process new assets
```

**docs/sessions/README.md**:
```markdown
# Development Session Logs

Session logs document day-by-day development progress, decisions, and implementation details.

## Sessions
- [2025-10-17](2025-10-17.md) - Gate Length Control (v0.4.0)
- [2025-10-18](2025-10-18.md) - Ratcheting (v0.5.0), UI Fixes (v0.5.1), Asset Catalog

## Template
See [template.md](template.md) for session documentation structure.
```

### Phase 5: Update References (10 minutes)

**Update CMakeLists.txt** if needed for new asset paths

**Update README.md** with new documentation structure:
```markdown
## Documentation
See [docs/](docs/) for complete documentation, organized by:
- **User Guides**: Getting started, features, DAW integration
- **Developer Docs**: Build instructions, architecture, enhancements
- **Design Specs**: UI design, project summary
- **Dev Sessions**: Day-by-day development logs
```

**Update STATUS.md** references to moved files

**Update CHANGELOG.md** references to moved files

---

## Benefits of New Structure

### Documentation
✅ **Clear separation**: User vs developer vs design docs
✅ **Easy navigation**: README.md in each directory
✅ **Reduced clutter**: Root directory only has essentials
✅ **Scalable**: Easy to add new categories
✅ **Professional**: Standard documentation structure

### Art Assets
✅ **Categorized originals**: Easy to find specific asset types
✅ **Clear workflow**: original → extracted → ui-ready
✅ **Tool organization**: All scripts in one place
✅ **Documentation**: Asset docs with the assets
✅ **Version control friendly**: Smaller directories, clearer commits

### Developer Experience
✅ **Faster onboarding**: Clear documentation structure
✅ **Less confusion**: Files are where you expect them
✅ **Better git history**: Organized commits by category
✅ **Easier maintenance**: Related files together

---

## Backwards Compatibility

### Maintain Symlinks (Optional)
If tools expect old paths, create symlinks:
```bash
# Keep old paths working temporarily
ln -s docs/developer/BUILD.md BUILD.md
ln -s art/catalog/ASSET_CATALOG.md conceptArt/ASSET_CATALOG.md
```

### Update Build Scripts
Update any build scripts or CI/CD that reference old paths

---

## Implementation Checklist

- [ ] Create new directory structure
- [ ] Move documentation files
- [ ] Create documentation index files (README.md)
- [ ] Organize original art assets by category
- [ ] Move extracted and ui-ready assets
- [ ] Move asset tools and documentation
- [ ] Create art index file
- [ ] Update root README.md
- [ ] Update STATUS.md references
- [ ] Update CHANGELOG.md references
- [ ] Test build with new asset paths
- [ ] Update .gitignore if needed
- [ ] Clean up old empty directories
- [ ] Create session template
- [ ] Document new structure in README

---

## Timeline

**Total Time**: ~50 minutes

- Phase 1 (Structure): 5 min
- Phase 2 (Docs): 10 min
- Phase 3 (Art): 15 min
- Phase 4 (Indexes): 10 min
- Phase 5 (References): 10 min

---

## Notes

- Keep CHANGELOG.md and STATUS.md in root (convention)
- Keep README.md in root (essential)
- Original JPGs should never be modified (preserve source)
- Resources/assets/ may need to stay for JUCE binary embedding
- Consider adding .editorconfig for consistent formatting

---

**Status**: READY TO IMPLEMENT
**Approval**: Awaiting user confirmation
