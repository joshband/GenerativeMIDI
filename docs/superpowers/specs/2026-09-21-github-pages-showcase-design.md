# Generative MIDI — GitHub Pages Showcase Design

**Date:** 2026-09-21  
**Status:** Approved for implementation planning  
**Repo:** https://github.com/joshband/GenerativeMIDI  
**Public URL (when Pages enabled):** https://joshband.github.io/GenerativeMIDI/

## Purpose

Hiring-facing static site that sells the builder and the plugin together.

**Primary audiences**
- Recruiters / hiring managers — 2-minute skim on Home
- Audio / DSP engineers — proof drill-down on Engineering

**Not** App Store marketing. Optimize for potential employers.

## Decisions locked

| Topic | Choice |
|-------|--------|
| Builder brand | Josh Band (equal hero signal with product) |
| Product brand | Generative MIDI |
| IA placement | Overlay at `/docs` (A) |
| Visual direction | Workshop forge (A) |
| Listen / audio | Omit from v1; no Listen CTA |
| Contact | GitHub only (`joshband` / this repo) |
| Framework | Hand-authored HTML/CSS/JS only |

## Site shape

```
docs/
├── index.html              # Home — hiring skim
├── engineering.html        # Engineering case study
├── assets/
│   ├── site.css
│   ├── site.js
│   └── signal-path.svg     # illustrative diagram
├── README.md               # demoted to internal index; points to Pages
├── design/ …               # retained evidence (markdown)
├── developer/ …
├── user/ …
├── sessions/ …
├── deployment/ …
└── superpowers/specs/      # this design doc
```

No `docs/audio/` in v1.

GitHub Pages: **Settings → Pages → Deploy from branch → `/docs`**.

## Honesty rules (non-negotiable)

- Ground every status number in current repo docs/tests before publishing.
- Never imply App Store readiness, finished UI, or unshipped formats as shipped.
- Separate factual shipped proof from illustrative diagrams (caption the SVG).
- Prefer measured/verified claims over enthusiasm.
- Do not invent listening clips or UI screenshots (README references a missing screenshot — do not fabricate one on the site).
- Agentic / AI-fluent engineering is a through-line and one Home section — no agent role names, vendor logos, or “AI built this” hype.
- Human owner remains the gate.

### Claim sources (verify again at implement time)

| Claim | Source | Publish as |
|-------|--------|------------|
| Version / feature status | `STATUS.md` (v0.8.0, 2025-10-18) | Development status, not “released product polish” |
| macOS AU / VST3 / Standalone build | `BUILD_VERIFICATION.md` | Build-verified artifacts |
| AUv3 | `CMakeLists.txt` `FORMATS`; iOS docs | Configured target + docs — not store-ready |
| Generator count | Processor param list has 10 incl. Polyrhythm; Editor UI lists 9 without Polyrhythm | **9 in current UI**; Polyrhythm engine retained in core/params, not exposed in editor dropdown |
| Parameters | STATUS: 31 | 31 parameters (per STATUS) |
| Test coverage | STATUS: Manual testing | Manual testing + build verification — no coverage % |
| UI | Design docs + audit (bitmap path, steampunk LookAndFeel) | Theme / craft in progress — not finished product polish |

## Visual system — Workshop forge

- **Atmosphere:** Deep charcoal field, subtle grain, soft radial gradients (warm metal + cool steel), not flat black.
- **Accents:** Warm brass/metal for brand & display; cool steel-cyan for proof links and secondary emphasis.
- **Typography:** Distinctive display + clean body via Google Fonts (avoid Inter, Roboto, Arial, system stacks).
- **Motion (2–3):** Hero atmosphere drift; scroll section reveal; focus/hover on Engineering CTA / proof links.
- **Avoid:** Purple glow AI defaults; cream + terracotta; broadsheet density; floating promo chips; hero cards/badges.

## Home (`index.html`) — section order

1. **Dual-brand hero** — full-bleed forge atmosphere; Josh Band + Generative MIDI equal; one thesis; one supporting sentence; CTAs: **Engineering · GitHub**. No cards/badges.
2. **What this proves** — three ownership claims: DSP/core → format wrapper → evidence discipline.
3. **Agentic engineering practice** — concrete method: written decisions → interface plans → implementation → independent re-verification from disk; human as gate.
4. **System at a glance** — one signal-path SVG; labeled illustrative.
5. **Boundary** — shipped vs deferred (formats, UI, hosts, features, listening clips, automation).
6. **About** — short builder blurb + GitHub links only.

## Engineering (`engineering.html`)

1. Problem / thesis  
2. Architecture (layers + shared illustrative diagram)  
3. Hard parts: realtime/scheduling, parameter bridging (incl. Polyrhythm UI/param mismatch as live boundary), bypass/lifecycle, measurement contracts  
4. Proof table with GitHub blob links to STATUS, BUILD_VERIFICATION, CHANGELOG, CMakeLists, design/build docs, sessions  
5. Selected design-decision highlights (LookAndFeel vs bitmap path; SynaptikUIToolkit submodule; modulation archived; Polyrhythm deferred from UI)  
6. What’s next (from STATUS: MIDI expression UI, polyrhythm layer controls — no oversell)

Shared nav: Home · Engineering · GitHub.

## README updates

Root `README.md`:
- Link showcase URL and document Pages setup (`/docs` branch deploy).
- Soften or contextualize marketing that contradicts honesty framing where it would mislead employers (without rewriting the whole README).

`docs/README.md`:
- State that the public face is the HTML Pages site; markdown remains internal/developer evidence.

## Out of scope (v1)

- Listen section / curated audio
- Fake product UI imagery
- Nested `/docs/site` or full markdown→HTML absorb
- Frameworks, dashboards, ops chrome
- Non-GitHub contact channels

## Success bar

- Recruiter can explain what was built and why it matters in under two minutes.
- Engineer can reach architecture and proof without a scavenger hunt.
- AI fluency is obvious and credible.
- Claims still match on-disk status after publish.

## Implementation next step

Create an implementation plan (writing-plans), then build and locally verify desktop/mobile, links, and honesty labels.
