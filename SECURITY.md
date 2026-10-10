# Security Policy

## Supported versions

This project is under active development (see `STATUS.md`). Security fixes are applied on `master` when reported.

## Build & distribution posture

- CI and local builds produce **unsigned** plugins by default for open-source development.
- There is **no App Store / notarization secret** in this repository. Do not commit Apple certificates, provisioning profiles, or API keys.
- Treat downloaded unsigned binaries as developer builds: gatekeepers may quarantine them; that is expected.

## Reporting a vulnerability

Please do not post vulnerability details in a public issue.

1. Preferred: use GitHub's private vulnerability reporting (repository **Security** tab → **Report a vulnerability**). If that button is not shown, private reporting has not been enabled yet; use step 2.
2. Otherwise open a public issue titled "Security contact request" with **no technical details**, and the maintainer will arrange a private channel.

You can expect an acknowledgement within a few days. This is a spare-time open-source project, so there is no formal response-time guarantee.

Please include:
- Affected version / commit
- Reproduction steps
- Impact assessment (crash, unexpected MIDI, file write outside preset dir, etc.)

## Scope notes

- Preset import accepts user XML; malformed files should fail closed without crashing the host.
- The `art/` git submodule is third-party UI assets; initialize it intentionally and verify the remote.
