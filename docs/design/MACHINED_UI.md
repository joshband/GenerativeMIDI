# Machined UI

Current editor look. Older brass and Synaptik violet specs in this folder are historical.

## Order

All four sections start open. Each has a full-width header bar; click the bar to collapse it to that bar. Accessibility titles are Pattern, Performance, Musical, and Shape.

1. Header, one row: product name and transport chip, then a Piano switch in the standalone app, then Generator, MIDI channel, and Preset. The SYNAPTIK wordmark is not in the header. Edited sits on this row, in ice, only while the generator differs from the loaded preset. Piano plays the generated notes in the app. Plugin formats stay MIDI-only, so the switch is hidden there.
2. Pattern. About two-fifths of the window, and never more than half. Polyrhythm replaces the pattern view with the layer editor.
3. Below the pattern, on a wide window, Performance, Musical, and Shape share one row. Knobs sit in a tight grid instead of stretching across the panel.
   - Performance: Tempo, Density, and only the knobs for the current generator.
   - Musical: velocity and pitch (each marked Min and Max), scale as one group, then swing, timing, velocity variation, gate, and legato.
   - Shape: aftertouch, pitch bend, CC, and ratchet.
4. A modulation bar sits under that row: LFO, LFO Hz, S&H Hz, then four routes. The first two routes are LFO to velocity and LFO to density. The other two choose a source and a destination.
5. Below about 1100px the three panels stack again, and the modulation bar stacks under Shape. Musical wraps under 780px of its own width.
6. MIDI log: pinned to the bottom of the window, collapsed to a single bar until opened.

At 1280×760 the pattern, the control row, and the modulation bar fill the space above the MIDI log. Opening the log shortens the pattern so the window still fits. Minimum width is 720. A collapsed section stays a single bar.

## Colour

Graphite panels and aluminum edges stay the same on every generator.

- Ice (`#5EE0FF`) is interaction only: knob thumbs, focus, the open chevron, and the Edited label.
- The pattern playhead and the generator name use one family accent:
  - Rhythm (Euclidean, Polyrhythm): warm aluminum
  - Algorithmic (Markov, L-System, Cellular, Probabilistic): cooled green
  - Stochastic (Brownian, Perlin, Drunk Walk, Lorenz): muted violet

Knobs do not change colour when the generator changes.

## Material

Drawn in code. Victorian bitmap assets are not loaded.

- The pattern is the display: an inset well, four aluminum corner ticks, and the generator caption at the bottom left. The drawing sits in the well, not in a second frame. A pitch stack on the right shows the notes that are sounding, and those same marks travel through the scrolling picture until they leave the history. Melody is a circle in the generator colour: the newest pitch in Mono, and every melody pitch still inside the gate in Poly. Parts 2 adds the root square, 3 adds the three chord triangles, 4 adds the arp diamond. Height is the pitch. Marks that share a pitch sit a few pixels apart. They fade across the gate. The caption names Mono or Poly and only the parts that are on. Polyrhythm keeps that same stack in a column beside the rows, with M, R, C, and A.
- On a wide window, Performance, Musical, and Shape share one faceplate. A hairline separates the groups. Each group still has its own disclosure bar.
- A knob is an aluminum bezel, a recessed dish, a short scale along the travel, an ice value arc, and an off-white needle.
- Type has three roles: tracked aluminum section titles, secondary names under the controls, and values in a small inset readout.
- A switch is an outline. Ice when it is on, aluminum when it is off. Expression, Ratchet, LFO, and Legato keep a name beside or under the control.
