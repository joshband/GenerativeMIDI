# GenerativeMIDI UI/UX review — 2026-09-23

Post-overhaul critique of the shipped Standalone editor (brass + cyan-on-navy). Evidence is the screenshot set under `docs/qa/logs/ui_overhaul_*.png`, not the earlier pre-polish pass.

Canonical look stays gilded navy with cyan live cues. This note does not propose a Synaptik violet / radial rewrite.

## What already works

- Banded information architecture: Pattern, Generator, Expression, Advanced.
- Generator-aware dimming (Euclidean knobs fade when another generator is selected; stochastic knobs fade when irrelevant).
- Header status chip (`Standalone · stopped` / free-run / note).
- SYNAPTIK demoted to a small mark; product name is the title.
- Advanced subgroups: Ratchet | Stochastic | LFO.
- Branded combo popups and a restyled Preset Manager.
- Round cyan slider thumbs; vertical viewport for short host frames; MIDI activity log below the fold.
- AX titles on primary controls.

## Findings still open at review time

### P0 — horizontal clip below 960px

`GenerativeMIDIEditor::resized` used `contentW = jmax(getWidth(), 960)` while the viewport showed a vertical scrollbar only. A window narrower than 960px clipped the right edge. The compact capture cut **Gate** to “Gat” and hid Legato plus the AT / PB / CC row.

### P1 — hollow non-Euclidean pattern band

Drunk Walk (and other non-pattern generators) painted a tall gold frame with one cyan status line and ticks that read as a single hairline. The band did not show activity.

### P1 — disabled knobs keep full weight

Steps, Pulses, and Rotation stayed full-size brass dials beside live Tempo and Density, so the eye treated irrelevant controls as primary.

### P2

- Options sits alone in the header.
- Header reads “No Preset” until a library row is clicked (dialog and editor agree; the empty state is easy to misread).
- MIDI log lives below the fold. Acceptable if the strip stays labeled.

## Scorecard (post-overhaul, before this fix pass)

| Criterion | Score /5 | Evidence | Severity if weak |
|-----------|----------|----------|------------------|
| Visual hierarchy | 3 | Product title and Generator band lead; disabled Euclidean dials still compete; pattern band is empty for stochastic gens | P1 |
| Information architecture | 4 | Four bands plus Advanced groups; Preset Manager has Category / Library / Actions | P2 |
| Typographic and label clarity | 3 | Section headers readable; compact width truncated Gate | P0 |
| Color and material | 4 | Brass frames, navy fill, cyan for live state; consistent in editor and preset dialog | — |
| Component consistency | 4 | Knobs, branded menus, round slider thumbs share one language | P2 |
| Density and layout | 2 | 960px content wider than the window clips Expression | P0 |
| Feedback and state | 3 | Status chip works; pattern band fails to show non-Euclidean activity | P1 |
| Audio-domain UX | 4 | Generator, tempo, density, MIDI log, expression row are present when not clipped | P1 |
| Accessibility | 4 | AX titles exist; clipped controls are unreachable by sight before scroll-x (there is no scroll-x) | P0 |
| Spec fidelity | 4 | Shipped UI matches brass + cyan, not the unused Synaptik violet spec | P2 |
| Implementation smell | 3 | Viewport is correct for height; width used a hard 960px floor | P0 |

## Fixes applied with this review

1. Content width tracks the editor (floor 720). Side insets and knob size shrink instead of clipping.
2. Pattern and generator bands are shorter so Expression stays intact above a short fold; Advanced and the MIDI log still scroll.
3. Non-Euclidean pattern band draws a readable activity meter (navy slots, cyan hits, status line).
4. Disabled Euclidean steps/pulses/rotation and disabled stochastic knobs are removed from the layout so live controls take the space.

## Still deferred

Physical-device AUv3 touch QA, App Store packaging, modulation matrix, MPE, Polyrhythm step-grid editing.
