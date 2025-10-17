# Changelog Maintenance Guide

This guide will help you keep `CHANGELOG.md` up-to-date as you develop Generative MIDI.

## Quick Reference

### Change Categories

Use these categories to organize changes:

- **Added** - New features, functionality, or files
- **Changed** - Modifications to existing features
- **Deprecated** - Features that will be removed in future versions
- **Removed** - Features that have been deleted
- **Fixed** - Bug fixes
- **Security** - Security vulnerability fixes

### Unreleased Section

Always add new changes to the `[Unreleased]` section at the top. When you release a new version, rename this section to the version number and date.

---

## Templates

### Adding a New Feature

```markdown
### Added
- Feature name with brief description
  - Sub-feature or detail
  - Another sub-feature
- Another feature
```

**Example:**
```markdown
### Added
- Scale constraint system
  - Major, minor, pentatonic, blues scales
  - Custom scale definition support
  - Per-generator scale override
- MIDI file export functionality
```

### Fixing a Bug

```markdown
### Fixed
- Brief description of what was broken and how it's fixed
- Another bug fix with location reference (e.g., `PluginProcessor.cpp:123`)
```

**Example:**
```markdown
### Fixed
- Clock manager now properly resets on transport stop
- Euclidean pattern generation with pulses > steps no longer crashes
- Note-off events now correctly scheduled when tempo changes mid-playback
```

### Changing Existing Behavior

```markdown
### Changed
- What changed and why (if relevant)
- Impact on users or behavior differences
```

**Example:**
```markdown
### Changed
- Default tempo changed from 120 to 128 BPM for electronic music workflows
- Pattern visualizer now shows velocity as bar height instead of color
- Increased maximum steps from 64 to 128 for longer patterns
```

---

## Workflow Options

### Option 1: Update as You Code

Add a comment in files you modify:

```cpp
// CHANGELOG: Fixed velocity range not applied in Euclidean mode
```

Then when ready to update changelog, search for `CHANGELOG:` comments:
```bash
grep -r "CHANGELOG:" Source/
```

### Option 2: Use Git Commits

Write descriptive commit messages:
```bash
git commit -m "Fix: Velocity range now applied to all generator modes"
git commit -m "Add: Scale constraint system with major/minor modes"
```

Then review recent commits:
```bash
git log --oneline -20
```

### Option 3: Keep Notes File

Create `CHANGES.md` in the project root and jot down changes as you work:
```markdown
# Changes Since Last Release

- Fixed velocity bug in PluginProcessor
- Added scale constraints
- Improved pattern visualizer
```

Then periodically move these to `CHANGELOG.md` with proper formatting.

---

## Asking Claude to Update the Changelog

### For Individual Changes

Just describe what changed:
```
"I fixed a crash when Euclidean pulses exceed steps, update the changelog"
```

### For Multiple Changes

Provide a list:
```
"Update changelog with these changes:
- Fixed crash with pulses > steps
- Added scale constraint dropdown
- Changed default tempo to 128"
```

### For Version Releases

Ask to finalize the unreleased section:
```
"I'm releasing v1.1.0, update the changelog and move Unreleased to v1.1.0"
```

### From Git History

Ask to scan git commits:
```
"Look at the last 10 git commits and update the changelog"
```

---

## Version Numbering (Semantic Versioning)

Follow [Semantic Versioning](https://semver.org/): **MAJOR.MINOR.PATCH**

- **MAJOR** (1.0.0 → 2.0.0) - Breaking changes, incompatible API changes
  - Examples: Removing parameters, changing file format, major architecture changes

- **MINOR** (1.0.0 → 1.1.0) - New features, backwards-compatible additions
  - Examples: New generator type, new parameters, MIDI file export

- **PATCH** (1.0.0 → 1.0.1) - Bug fixes, backwards-compatible fixes
  - Examples: Fixing crashes, correcting parameter behavior, performance improvements

### Pre-release versions
- **Alpha**: `1.1.0-alpha.1` - Early testing, features incomplete
- **Beta**: `1.1.0-beta.1` - Feature complete, testing for bugs
- **Release Candidate**: `1.1.0-rc.1` - Final testing before release

---

## Example: Adding a Complete Change Entry

### Before (Current Changelog)
```markdown
## [Unreleased]

### Fixed
- Velocity range parameters now properly applied to all generated notes
```

### After Adding Your Change
```markdown
## [Unreleased]

### Added
- Scale constraint system
  - Major, minor, pentatonic, harmonic minor, melodic minor scales
  - Custom scale definition support
  - Scale selector dropdown in UI
  - Per-generator scale override capability

### Fixed
- Velocity range parameters now properly applied to all generated notes
- Euclidean engine no longer crashes when pulses exceed steps
```

---

## Example: Releasing a New Version

### Before Release (Unreleased Section)
```markdown
## [Unreleased]

### Added
- Scale constraint system
- MIDI file export

### Fixed
- Velocity range parameters
- Euclidean crash bug
```

### After Release (v1.1.0)
```markdown
## [Unreleased]

## [1.1.0] - 2025-10-20

### Added
- Scale constraint system
  - Major, minor, pentatonic, harmonic minor, melodic minor scales
  - Custom scale definition support
  - Scale selector dropdown in UI
- MIDI file export
  - Export generated patterns to MIDI files
  - Configurable length and loop count
  - Standard MIDI File Format 1 support

### Fixed
- Velocity range parameters now properly applied to all generated notes across all generator types
- Euclidean engine no longer crashes when pulses exceed steps
- Clock manager properly resets on transport stop

### Changed
- Default tempo increased from 120 to 128 BPM

## [1.0.0] - 2025-10-16
...
```

---

## Tips for Good Changelog Entries

### ✅ Good
- **Specific**: "Fixed note-off messages using incorrect pitch in algorithmic mode"
- **User-focused**: "Added scale constraints to keep melodies in key"
- **Complete**: "Velocity range now properly applied to Euclidean, Polyrhythm, and Algorithmic generators"

### ❌ Avoid
- **Vague**: "Fixed bug"
- **Too technical**: "Refactored PluginProcessor::processBlock() to use std::vector"
- **Implementation details**: "Changed loop index from i to idx"

### Focus on User Impact

Ask yourself: "Would a user care about this change?"
- **Yes**: Bug fixes, new features, behavior changes, performance improvements
- **No**: Code refactoring, internal restructuring (unless it improves performance)

---

## Integration with Git

### Tag Releases
```bash
# Create annotated tag for release
git tag -a v1.1.0 -m "Release version 1.1.0"
git push origin v1.1.0
```

### Compare Versions
```bash
# See all changes between versions
git log v1.0.0..v1.1.0 --oneline

# See changed files
git diff v1.0.0..v1.1.0 --name-only
```

### Link Changelog to GitHub Releases
When you create a GitHub release, copy the relevant section from CHANGELOG.md into the release notes.

---

## Automation Ideas (Future)

### Git Hook
Create `.git/hooks/prepare-commit-msg` to remind you:
```bash
#!/bin/bash
echo "# Remember to update CHANGELOG.md!" >> $1
```

### Script to Extract Git Commits
```bash
#!/bin/bash
# get-changes.sh - Extract commits since last tag
git log $(git describe --tags --abbrev=0)..HEAD --oneline
```

---

## Quick Checklist Before Release

- [ ] All changes moved from `[Unreleased]` to new version section
- [ ] Version number follows semantic versioning
- [ ] Date added in format `YYYY-MM-DD`
- [ ] Changes organized by category (Added/Changed/Fixed/etc.)
- [ ] Descriptions are clear and user-focused
- [ ] Technical details provided where helpful
- [ ] No placeholder text or TODO items
- [ ] Spelling and grammar checked
- [ ] New `[Unreleased]` section created at top

---

## Questions?

When you're ready to update the changelog, just tell Claude:

1. **What you did** - "Fixed crash when tempo is below 60 BPM"
2. **What to do** - "Update the changelog"

Or simply paste your git log and ask:
```
git log --oneline -10
```
"Update the changelog from these commits"

---

**Remember**: The changelog is for users, not developers. Focus on what changed from their perspective!
