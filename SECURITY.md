# Security Policy

## Supported versions

This project is under active development (see `STATUS.md`). Security fixes are applied on `master` when reported.

## Build & distribution posture

- CI and local builds produce **unsigned** plugins by default for open-source development.
- There is **no App Store / notarization secret** in this repository. Do not commit Apple certificates, provisioning profiles, or API keys.
- Treat downloaded unsigned binaries as developer builds: gatekeepers may quarantine them; that is expected.

## Reporting a vulnerability

Open a [GitHub Security Advisory](https://github.com/joshband/GenerativeMIDI/security/advisories/new) if available, or file a private report via GitHub Issues marked as security-sensitive.

Please include:
- Affected version / commit
- Reproduction steps
- Impact assessment (crash, unexpected MIDI, file write outside preset dir, etc.)

## Scope notes

- Preset import accepts user XML; malformed files should fail closed without crashing the host.
- The `art/` git submodule is third-party UI assets; initialize it intentionally and verify the remote.
