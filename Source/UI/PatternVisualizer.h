/*
  ==============================================================================
    PatternVisualizer.h

    One cavity, ten generator views. Wells, stems, curves, stairs, and an orbit
    Quiet metal history, with the generator family colour on the playhead.

    Refreshed by the editor's 30 Hz timer — this component does not own a Timer.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "CustomLookAndFeel.h"
#include <array>
#include <cmath>

class PatternVisualizer : public juce::Component
{
public:
    enum class Mode
    {
        Steps,    // Euclidean hits
        Cells,    // Cellular row; index 0 is the voice
        Symbols,  // L-system characters
        Stems,    // Markov / probabilistic pitches
        Curve,    // Brownian, Perlin
        Stairs,   // Drunk walk
        Orbit     // Lorenz x/z
    };

    enum class Role
    {
        Melody, // circle, family colour
        Root,   // square, brass
        Chord,  // triangle, steel
        Arp     // diamond, clay
    };

    PatternVisualizer() = default;

    static void paintOneWell(juce::Graphics& g, juce::Rectangle<float> cell,
                             bool active, bool current, float alpha = 1.0f,
                             juce::Colour currentColour = juce::Colour(CustomLookAndFeel::GOLD_TEMPLE))
    {
        const auto history = juce::Colour(CustomLookAndFeel::COPPER_STEAM);
        const float a = juce::jlimit(0.0f, 1.0f, alpha);
        g.setColour(juce::Colour(CustomLookAndFeel::ABYSS_NAVY).brighter(0.12f).withMultipliedAlpha(a));
        g.fillRoundedRectangle(cell, 2.0f);

        if (active)
        {
            const float h = current ? cell.getHeight() : cell.getHeight() * 0.72f;
            auto fill = cell.withTop(cell.getBottom() - h);
            g.setColour((current ? currentColour : history.withAlpha(0.45f)).withMultipliedAlpha(a));
            g.fillRoundedRectangle(fill, 2.0f);
        }
        else if (current)
        {
            g.setColour(currentColour.withAlpha(0.85f * a));
            g.fillRect(cell.getX() + 1.0f, cell.getY(), juce::jmax(1.0f, cell.getWidth() - 2.0f), 3.0f);
        }

        if (current)
        {
            g.setColour(juce::Colours::white.withAlpha(0.9f * a));
            g.fillRect(cell.getX() + 1.0f, cell.getY(), juce::jmax(1.0f, cell.getWidth() - 2.0f), 2.0f);
        }
    }

    void setMode(Mode next)
    {
        if (mode == next)
            return;
        mode = next;
        clearTrail();
    }

    void setGenerator(int id)
    {
        if (generatorId == id)
            return;
        generatorId = id;
        familyColour = CustomLookAndFeel::familyAccent(id);
        clearTrail();
    }

    void setCaption(const juce::String& text)
    {
        if (caption == text)
            return;
        caption = text;
        publishDescription();
        repaint();
    }

    void setSteps(const bool* hits, int count, int playhead)
    {
        stepCount = juce::jlimit(0, kMaxSteps, count);
        playheadIndex = playhead;
        for (int i = 0; i < stepCount; ++i)
            steps[static_cast<size_t>(i)] = hits[i];
        repaint();
    }

    void setSymbols(const juce::String& symbols, int playhead)
    {
        symbolText = symbols.substring(0, kMaxSteps);
        stepCount = symbolText.length();
        playheadIndex = playhead;
        repaint();
    }

    void setCells(const bool* cellsIn, int count, int voiceIndex)
    {
        stepCount = juce::jlimit(0, kMaxSteps, count);
        playheadIndex = voiceIndex;
        for (int i = 0; i < stepCount; ++i)
            steps[static_cast<size_t>(i)] = cellsIn[i];
        repaint();
    }

    /** One elementary-CA generation. Oldest rows scroll off after kHistory. */
    void pushCells(const bool* cellsIn, int count, int voiceIndex)
    {
        count = juce::jlimit(0, kMaxSteps, count);
        auto& row = cellHistory[static_cast<size_t>(historyWrite)];
        row.fill(false);
        for (int i = 0; i < count; ++i)
            row[static_cast<size_t>(i)] = cellsIn[i];
        cellCounts[static_cast<size_t>(historyWrite)] = count;
        historyWrite = (historyWrite + 1) % kHistory;
        if (historyCount < kHistory)
            ++historyCount;
        playheadIndex = voiceIndex;
        repaint();
    }

    /** L-system generations, oldest first. Playhead walks the newest row. */
    void setGrowth(const juce::StringArray& rows, int playhead)
    {
        const int n = juce::jmin(rows.size(), kHistory);
        bool changed = n != symbolRows.size();
        if (!changed)
        {
            for (int i = 0; i < n; ++i)
            {
                if (symbolRows[i] != rows[i])
                {
                    changed = true;
                    break;
                }
            }
        }

        if (changed)
        {
            symbolRows.clearQuick();
            for (int i = 0; i < n; ++i)
                symbolRows.add(rows[i].substring(0, kMaxSteps));
        }

        playheadIndex = playhead;
        repaint();
    }

    /** Push a 0..1 sample. Identical repeats are kept so holds stay visible. */
    void pushSample(float x, float y)
    {
        x = juce::jlimit(0.0f, 1.0f, x);
        y = juce::jlimit(0.0f, 1.0f, y);
        traceX[static_cast<size_t>(traceWrite)] = x;
        traceY[static_cast<size_t>(traceWrite)] = y;
        traceWrite = (traceWrite + 1) % kTrace;
        if (traceCount < kTrace)
            ++traceCount;
        lastPushed = y;
        repaint();
    }

    void pushSampleIfChanged(float y)
    {
        if (std::abs(y - lastPushed) < 0.0001f && traceCount > 0)
            return;
        pushSample(0.0f, y);
    }

    void setDensity(float amount)
    {
        density = juce::jlimit(0.0f, 1.0f, amount);
    }

    void setShowDensity(bool show) { showDensity = show; }

    /** Mono keeps the newest melody pitch. Poly keeps every melody pitch still inside the gate. */
    void setVoice(bool mono, float)
    {
        if (monoVoice == mono)
            return;
        monoVoice = mono;
        publishDescription();
        repaint();
    }

    /** A melody note that just sounded. Same pitch a few milliseconds later is the same event. */
    void noteMelody(int pitch)
    {
        pitch = juce::jlimit(0, 127, pitch);
        const auto now = juce::Time::getMillisecondCounter();
        if (melodyCount > 0)
        {
            const int prev = (melodyWrite + kMelody - 1) % kMelody;
            if (melodyPitch[static_cast<size_t>(prev)] == pitch
                && now - melodyAt[static_cast<size_t>(prev)] < 15)
                return;
        }

        melodyPitch[static_cast<size_t>(melodyWrite)] = pitch;
        melodyAt[static_cast<size_t>(melodyWrite)] = now;
        melodyWrite = (melodyWrite + 1) % kMelody;
        if (melodyCount < kMelody)
            ++melodyCount;
        stamp(Role::Melody, pitch);
        repaint();
    }

    int newestMelody() const
    {
        if (melodyCount <= 0)
            return -1;
        return melodyPitch[static_cast<size_t>((melodyWrite + kMelody - 1) % kMelody)];
    }

    void setHarmony(int root, int third, int fifth)
    {
        root = juce::jlimit(0, 127, root);
        third = juce::jlimit(0, 127, third);
        fifth = juce::jlimit(0, 127, fifth);
        const bool changed = chordRoot != root || chordThird != third || chordFifth != fifth;
        chordRoot = root;
        chordThird = third;
        chordFifth = fifth;
        const bool stamped = stampOpenParts();
        if (changed || stamped)
            repaint();
    }

    void setPitchSpan(int low, int high)
    {
        low = juce::jlimit(0, 127, low);
        high = juce::jlimit(low + 1, 127, high);
        pitchLow = low;
        pitchHigh = high;
    }

    /** step < 0 is a stopped clock: part marks stay dark. gateSeconds is one gate at the current tempo. */
    void setParts(int count, int step, int sixteenthsPerBar, float gateSecondsIn)
    {
        partCount = juce::jlimit(1, 4, count);
        barSixteenths = juce::jmax(1, sixteenthsPerBar);
        gateSeconds = juce::jlimit(0.02f, 4.0f, gateSecondsIn);
        partStep = step;
        clock.advance(partCount, step, barSixteenths);
        publishDescription();
        repaint();
    }

    struct PartClock
    {
        uint32_t rootAt = 0;
        uint32_t chordAt = 0;
        uint32_t arpAt = 0;
        int arpIndex = 0;
        int lastStep = -2;

        void advance(int count, int step, int sixteenthsPerBar) noexcept
        {
            if (step < 0)
            {
                lastStep = -1;
                return;
            }
            if (step == lastStep)
                return;
            lastStep = step;
            const auto now = juce::Time::getMillisecondCounter();
            if (count >= 2 && (step % 4) == 0)
                rootAt = now;
            if (count >= 3 && (step % juce::jmax(1, sixteenthsPerBar)) == 0)
                chordAt = now;
            if (count >= 4 && (step % 2) == 0)
            {
                arpAt = now;
                arpIndex = (step / 2) % 3;
            }
        }

        static float fade(uint32_t at, float gateSecondsIn, bool live) noexcept
        {
            if (!live || at == 0)
                return 0.0f;
            const float gateMs = juce::jmax(30.0f, gateSecondsIn * 1000.0f);
            const float age = static_cast<float>(juce::Time::getMillisecondCounter() - at);
            if (age >= gateMs)
                return 0.0f;
            return 1.0f - age / gateMs;
        }
    };

    struct StackMark
    {
        Role role = Role::Melody;
        int pitch = 60;
        float amount = 0.0f;
        juce::Colour colour { juce::Colour(CustomLookAndFeel::GOLD_TEMPLE) };
        const char* name = "";
    };

    /** Pitches still inside the gate, plus the parts the 1–4 selection includes. */
    int fillStack(StackMark* out, int max) const
    {
        if (out == nullptr || max <= 0)
            return 0;

        const bool live = partStep >= 0;
        const auto now = juce::Time::getMillisecondCounter();
        const float gateMs = juce::jmax(30.0f, gateSeconds * 1000.0f);
        int count = 0;

        auto push = [&](Role role, int pitch, float amount, juce::Colour colour, const char* name)
        {
            if (count >= max || pitch < 0)
                return;
            out[count].role = role;
            out[count].pitch = juce::jlimit(0, 127, pitch);
            out[count].amount = live ? amount : 0.0f;
            out[count].colour = colour;
            out[count].name = name;
            ++count;
        };

        bool anyMelody = false;
        for (int age = 0; age < melodyCount && count < max; ++age)
        {
            const int idx = (melodyWrite - 1 - age + kMelody) % kMelody;
            const float ms = static_cast<float>(now - melodyAt[static_cast<size_t>(idx)]);
            if (ms >= gateMs)
                break;
            push(Role::Melody, melodyPitch[static_cast<size_t>(idx)], 1.0f - ms / gateMs,
                 familyColour, anyMelody ? "" : "M");
            anyMelody = true;
            if (monoVoice)
                break;
        }

        if (!anyMelody)
            push(Role::Melody, melodyCount > 0 ? newestMelody() : 60, 0.0f, familyColour, "M");

        const float root = partCount >= 2 ? PartClock::fade(clock.rootAt, gateSeconds, live) : 0.0f;
        const float chord = partCount >= 3 ? PartClock::fade(clock.chordAt, gateSeconds, live) : 0.0f;
        const float arp = partCount >= 4 ? PartClock::fade(clock.arpAt, gateSeconds, live) : 0.0f;
        if (partCount >= 2)
            push(Role::Root, chordRoot, root, juce::Colour(CustomLookAndFeel::PART_ROOT), "R");
        if (partCount >= 3)
        {
            push(Role::Chord, chordRoot, chord, juce::Colour(CustomLookAndFeel::PART_CHORD), "C");
            push(Role::Chord, chordThird, chord, juce::Colour(CustomLookAndFeel::PART_CHORD), "");
            push(Role::Chord, chordFifth, chord, juce::Colour(CustomLookAndFeel::PART_CHORD), "");
        }
        if (partCount >= 4)
        {
            const int tone = clock.arpIndex == 1 ? chordThird : (clock.arpIndex == 2 ? chordFifth : chordRoot);
            push(Role::Arp, tone, arp, juce::Colour(CustomLookAndFeel::PART_ARP), "A");
        }
        return count;
    }

    static void paintPitchStack(juce::Graphics& g, juce::Rectangle<float> area,
                                const StackMark* marks, int count,
                                int pitchMin, int pitchMax, bool labels)
    {
        if (marks == nullptr || count <= 0 || area.getWidth() < 8.0f || area.getHeight() < 16.0f)
            return;

        pitchMin = juce::jlimit(0, 126, pitchMin);
        pitchMax = juce::jlimit(pitchMin + 1, 127, pitchMax);
        const float span = static_cast<float>(pitchMax - pitchMin);
        const float mark = juce::jmin(12.0f, labels ? area.getWidth() * 0.42f : area.getWidth() * 0.7f);

        for (int i = 0; i < count; ++i)
        {
            int slot = 0;
            for (int j = 0; j < i; ++j)
                if (marks[j].pitch == marks[i].pitch)
                    ++slot;

            const float yNorm = juce::jlimit(0.0f, 1.0f, static_cast<float>(marks[i].pitch - pitchMin) / span);
            const float y = area.getBottom() - mark * 0.5f - yNorm * (area.getHeight() - mark);
            const float x = (labels ? area.getRight() - mark * 0.5f - 1.0f : area.getCentreX())
                            - static_cast<float>(slot) * 3.5f;
            auto well = juce::Rectangle<float>(mark, mark).withCentre({ x, y });

            if (labels && marks[i].name != nullptr && marks[i].name[0] != '\0')
            {
                g.setColour(marks[i].colour.withAlpha(0.85f));
                g.setFont(juce::FontOptions(9.0f));
                g.drawText(marks[i].name,
                           juce::Rectangle<float>(area.getX(), y - 6.0f, juce::jmax(8.0f, well.getX() - area.getX() - 1.0f), 12.0f),
                           juce::Justification::centredRight, false);
            }

            paintRoleMark(g, well, marks[i].role, marks[i].colour, marks[i].amount);
        }
    }

    static void paintRoleMark(juce::Graphics& g, juce::Rectangle<float> well,
                              Role role, juce::Colour colour, float amount)
    {
        amount = juce::jlimit(0.0f, 1.0f, amount);
        const float alpha = amount <= 0.02f ? 0.22f : (0.4f + 0.55f * amount);
        const auto fill = colour.withAlpha(alpha);
        const auto edge = juce::Colour(CustomLookAndFeel::ABYSS_NAVY).withAlpha(juce::jmin(1.0f, alpha + 0.3f));
        const float stroke = juce::jmax(1.0f, well.getWidth() * 0.1f);

        auto fillAndStroke = [&](const juce::Path& path)
        {
            g.setColour(fill);
            g.fillPath(path);
            g.setColour(edge);
            g.strokePath(path, juce::PathStrokeType(stroke));
        };

        switch (role)
        {
            case Role::Melody:
                g.setColour(fill);
                g.fillEllipse(well);
                g.setColour(edge);
                g.drawEllipse(well.reduced(stroke * 0.5f), stroke);
                break;

            case Role::Root:
                g.setColour(fill);
                g.fillRect(well);
                g.setColour(edge);
                g.drawRect(well, stroke);
                break;

            case Role::Chord:
            {
                juce::Path triangle;
                triangle.addTriangle(well.getCentreX(), well.getY(),
                                     well.getX(), well.getBottom(),
                                     well.getRight(), well.getBottom());
                fillAndStroke(triangle);
                break;
            }

            case Role::Arp:
            {
                juce::Path diamond;
                const float cx = well.getCentreX();
                const float cy = well.getCentreY();
                diamond.startNewSubPath(cx, well.getY());
                diamond.lineTo(well.getRight(), cy);
                diamond.lineTo(cx, well.getBottom());
                diamond.lineTo(well.getX(), cy);
                diamond.closeSubPath();
                fillAndStroke(diamond);
                break;
            }
        }
    }

    // Legacy hooks kept so older call sites still compile during the switch.
    void setPattern(const std::vector<bool>& newPattern)
    {
        stepCount = juce::jlimit(0, kMaxSteps, static_cast<int>(newPattern.size()));
        for (int i = 0; i < stepCount; ++i)
            steps[static_cast<size_t>(i)] = newPattern[static_cast<size_t>(i)];
        mode = Mode::Steps;
        repaint();
    }

    void setCurrentStep(int step) { playheadIndex = step; repaint(); }
    void setAccentColor(juce::Colour) {}
    void setStatusText(const juce::String& text) { setCaption(text); }
    void setActivityLevel(float) {}
    void pushActivityTick(bool) {}

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto plot = bounds.reduced(16.0f, 12.0f);
        auto captionArea = plot.removeFromBottom(16.0f);
        plot.removeFromBottom(6.0f);
        auto stackArea = plot.removeFromRight(36.0f);
        plot.removeFromRight(6.0f);

        const auto inner = bounds.reduced(10.0f);
        g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.65f));
        const float tick = 8.0f;
        auto corner = [&](float x, float y, float sx, float sy)
        {
            g.drawLine(x, y, x + sx * tick, y, 1.0f);
            g.drawLine(x, y, x, y + sy * tick, 1.0f);
        };
        corner(inner.getX(), inner.getY(), 1.0f, 1.0f);
        corner(inner.getRight(), inner.getY(), -1.0f, 1.0f);
        corner(inner.getX(), inner.getBottom(), 1.0f, -1.0f);
        corner(inner.getRight(), inner.getBottom(), -1.0f, -1.0f);

        switch (mode)
        {
            case Mode::Steps: paintEuclideanRing(g, plot); break;
            case Mode::Cells: paintCellHistory(g, plot); break;
            case Mode::Symbols: paintGrowth(g, plot); break;
            case Mode::Stems: paintPitchChain(g, plot); break;
            case Mode::Curve: paintScope(g, plot, false); break;
            case Mode::Stairs: paintScope(g, plot, true); break;
            case Mode::Orbit: paintOrbit(g, plot); break;
        }

        StackMark stackMarks[16];
        const int stackCount = fillStack(stackMarks, 16);
        paintPitchStack(g, stackArea, stackMarks, stackCount, pitchLow, pitchHigh, false);

        g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED));
        g.setFont(juce::Font(juce::FontOptions(11.0f)).withExtraKerningFactor(0.14f));
        g.drawText(voiceCaption(), captionArea, juce::Justification::centredLeft, false);
    }

private:
    static constexpr int kMaxSteps = 64;
    static constexpr int kTrace = 96;
    static constexpr int kHistory = 12;
    static constexpr int kScroll = 64;

    juce::String voiceCaption() const
    {
        juce::String shown = caption;
        if (shown.isNotEmpty())
            shown << "   ";
        shown << (monoVoice ? "Mono" : "Poly");
        if (partCount >= 2)
            shown << "  Root";
        if (partCount >= 3)
            shown << " Chord";
        if (partCount >= 4)
            shown << " Arp";
        return shown;
    }

    void publishDescription()
    {
        const auto text = voiceCaption();
        if (text == described)
            return;
        described = text;
        setTitle("Pattern");
        setDescription(text);
    }

    void clearTrail()
    {
        traceCount = 0;
        traceWrite = 0;
        lastPushed = -1000.0f;
        historyCount = 0;
        historyWrite = 0;
        symbolRows.clearQuick();
        melodyCount = 0;
        melodyWrite = 0;
        scrollCount = 0;
        scrollWrite = 0;
        stampedRoot = clock.rootAt;
        stampedChord = clock.chordAt;
        stampedArp = clock.arpAt;
        repaint();
    }

    struct ScrollHit
    {
        Role role = Role::Melody;
        int pitch = 60;
        uint32_t at = 0;
        int anchor = 0;
    };

    int scrollAnchor() const
    {
        switch (mode)
        {
            case Mode::Stems:
            case Mode::Curve:
            case Mode::Stairs:
            case Mode::Orbit:
                return traceCount > 0 ? (traceWrite + kTrace - 1) % kTrace : 0;
            case Mode::Cells:
                return historyCount > 0 ? (historyWrite + kHistory - 1) % kHistory : 0;
            case Mode::Steps:
            case Mode::Symbols:
                return playheadIndex;
        }
        return 0;
    }

    void stamp(Role role, int pitch)
    {
        pitch = juce::jlimit(0, 127, pitch);
        const auto now = juce::Time::getMillisecondCounter();
        const int anchor = scrollAnchor();
        if (scrollCount > 0)
        {
            const int prev = (scrollWrite + kScroll - 1) % kScroll;
            const auto& last = scrollHits[static_cast<size_t>(prev)];
            if (last.role == role && last.pitch == pitch && last.anchor == anchor && now - last.at < 15)
                return;
        }

        scrollHits[static_cast<size_t>(scrollWrite)] = { role, pitch, now, anchor };
        scrollWrite = (scrollWrite + 1) % kScroll;
        if (scrollCount < kScroll)
            ++scrollCount;
    }

    bool stampOpenParts()
    {
        bool stamped = false;
        if (partCount >= 2 && clock.rootAt != 0 && clock.rootAt != stampedRoot)
        {
            stampedRoot = clock.rootAt;
            stamp(Role::Root, chordRoot);
            stamped = true;
        }
        if (partCount >= 3 && clock.chordAt != 0 && clock.chordAt != stampedChord)
        {
            stampedChord = clock.chordAt;
            stamp(Role::Chord, chordRoot);
            stamp(Role::Chord, chordThird);
            stamp(Role::Chord, chordFifth);
            stamped = true;
        }
        if (partCount >= 4 && clock.arpAt != 0 && clock.arpAt != stampedArp)
        {
            stampedArp = clock.arpAt;
            const int tone = clock.arpIndex == 1 ? chordThird : (clock.arpIndex == 2 ? chordFifth : chordRoot);
            stamp(Role::Arp, tone);
            stamped = true;
        }
        return stamped;
    }

    void paintScrollMarks(juce::Graphics& g, juce::Rectangle<float> field)
    {
        if (scrollCount <= 0 || field.isEmpty())
            return;

        const float mark = 10.0f;
        auto colourFor = [this](Role role)
        {
            switch (role)
            {
                case Role::Root: return juce::Colour(CustomLookAndFeel::PART_ROOT);
                case Role::Chord: return juce::Colour(CustomLookAndFeel::PART_CHORD);
                case Role::Arp: return juce::Colour(CustomLookAndFeel::PART_ARP);
                case Role::Melody:
                default: return familyColour;
            }
        };
        auto pitchY = [this, field](int pitch)
        {
            const float span = static_cast<float>(juce::jmax(1, pitchHigh - pitchLow));
            const float n = juce::jlimit(0.0f, 1.0f, static_cast<float>(pitch - pitchLow) / span);
            return field.getBottom() - n * field.getHeight();
        };

        for (int n = 0; n < scrollCount; ++n)
        {
            const int idx = (scrollWrite - scrollCount + n + kScroll) % kScroll;
            const auto& hit = scrollHits[static_cast<size_t>(idx)];
            int slot = 0;
            for (int p = 0; p < n; ++p)
            {
                const int prev = (scrollWrite - scrollCount + p + kScroll) % kScroll;
                const auto& other = scrollHits[static_cast<size_t>(prev)];
                if (other.anchor == hit.anchor && other.pitch == hit.pitch)
                    ++slot;
            }

            float amount = 1.0f;
            juce::Point<float> at;
            bool visible = false;

            if (mode == Mode::Steps && stepCount > 0)
            {
                const int step = ((hit.anchor % stepCount) + stepCount) % stepCount;
                const int head = playheadIndex % stepCount;
                const int behind = (head - step + stepCount) % stepCount;
                if (behind > 8)
                    continue;
                amount = 1.0f - static_cast<float>(behind) / 9.0f;
                const float ang = -juce::MathConstants<float>::halfPi
                                  + juce::MathConstants<float>::twoPi * static_cast<float>(step) / static_cast<float>(stepCount);
                const float radius = juce::jmin(field.getWidth(), field.getHeight()) * 0.38f;
                float extra = 0.0f;
                if (hit.role == Role::Root)
                    extra = -0.16f;
                else if (hit.role == Role::Chord)
                    extra = 0.18f;
                else if (hit.role == Role::Arp)
                    extra = 0.30f;
                const float r = radius * (1.0f + extra);
                at = { field.getCentreX() + std::cos(ang) * r - static_cast<float>(slot) * 3.0f,
                       field.getCentreY() + std::sin(ang) * r };
                visible = true;
            }
            else if (mode == Mode::Cells && historyCount > 0)
            {
                const int ageFromNewest = (historyWrite - 1 - hit.anchor + kHistory) % kHistory;
                if (ageFromNewest >= historyCount)
                    continue;
                const int age = historyCount - 1 - ageFromNewest;
                int cols = 1;
                for (int i = 0; i < historyCount; ++i)
                {
                    const int row = (historyWrite - historyCount + i + kHistory) % kHistory;
                    cols = juce::jmax(cols, cellCounts[static_cast<size_t>(row)]);
                }
                const auto grid = gridFor(field, cols, kHistory);
                const float yBase = field.getBottom() - grid.pitchY * static_cast<float>(historyCount);
                amount = ageFromNewest == 0 ? 1.0f : 0.4f + 0.6f * static_cast<float>(age + 1) / static_cast<float>(historyCount);
                const float span = static_cast<float>(juce::jmax(1, pitchHigh - pitchLow));
                const float nx = juce::jlimit(0.0f, 1.0f, static_cast<float>(hit.pitch - pitchLow) / span);
                at = { field.getX() + nx * field.getWidth() - static_cast<float>(slot) * 3.0f,
                       yBase + grid.pitchY * static_cast<float>(age) + grid.cellH * 0.5f };
                visible = true;
            }
            else if (mode == Mode::Symbols && !symbolRows.isEmpty())
            {
                const int rows = symbolRows.size();
                const int symbols = juce::jmax(1, symbolRows[rows - 1].length());
                const int head = playheadIndex % symbols;
                const int col = ((hit.anchor % symbols) + symbols) % symbols;
                const int behind = (head - col + symbols) % symbols;
                if (behind > 8)
                    continue;
                amount = 1.0f - static_cast<float>(behind) / 9.0f;
                int cols = 1;
                for (int r = 0; r < rows; ++r)
                    cols = juce::jmax(cols, symbolRows[r].length());
                const auto grid = gridFor(field, cols, rows);
                at = { grid.x0 + grid.pitchX * static_cast<float>(col) + grid.cellW * 0.5f - static_cast<float>(slot) * 3.0f,
                       field.getY() + grid.pitchY * static_cast<float>(rows - 1) + grid.cellH * 0.5f };
                visible = true;
            }
            else if (traceCount > 0
                     && (mode == Mode::Stems || mode == Mode::Curve || mode == Mode::Stairs || mode == Mode::Orbit))
            {
                const int ageFromNewest = (traceWrite - 1 - hit.anchor + kTrace) % kTrace;
                if (ageFromNewest >= traceCount)
                    continue;
                const int i = traceCount - 1 - ageFromNewest;
                amount = 0.35f + 0.65f * static_cast<float>(i + 1) / static_cast<float>(traceCount);
                if (mode == Mode::Orbit)
                {
                    const int slotIdx = traceIndex(i);
                    const float nx = juce::jlimit(0.0f, 1.0f, traceX[static_cast<size_t>(slotIdx)]);
                    const float ny = juce::jlimit(0.0f, 1.0f, traceY[static_cast<size_t>(slotIdx)]);
                    float lift = 0.0f;
                    if (hit.role == Role::Root)
                        lift = -9.0f;
                    else if (hit.role == Role::Chord)
                        lift = -16.0f - static_cast<float>(slot) * 5.0f;
                    else if (hit.role == Role::Arp)
                        lift = 9.0f;
                    at = { field.getX() + nx * field.getWidth(),
                           field.getBottom() - ny * field.getHeight() + lift };
                }
                else
                {
                    float x = field.getX();
                    if (mode == Mode::Stairs)
                    {
                        const float slotW = field.getWidth() / static_cast<float>(traceCount);
                        x += slotW * (static_cast<float>(i) + 0.5f);
                    }
                    else if (mode == Mode::Stems)
                        x += field.getWidth() * (static_cast<float>(i) + 0.5f) / static_cast<float>(traceCount);
                    else
                    {
                        const float denom = static_cast<float>(juce::jmax(1, traceCount - 1));
                        x += field.getWidth() * static_cast<float>(i) / denom;
                    }
                    at = { x - static_cast<float>(slot) * 3.5f, pitchY(hit.pitch) };
                }
                visible = true;
            }

            if (!visible)
                continue;
            paintRoleMark(g, juce::Rectangle<float>(mark, mark).withCentre(at),
                          hit.role, colourFor(hit.role), amount);
        }
    }

    void paintScopeBackdrop(juce::Graphics& g, juce::Rectangle<float> plot) const
    {
        g.setColour(juce::Colours::white.withAlpha(0.06f));
        for (int i = 1; i <= 3; ++i)
        {
            const float y = plot.getY() + plot.getHeight() * static_cast<float>(i) / 4.0f;
            g.drawHorizontalLine(juce::roundToInt(y), plot.getX(), plot.getRight());
        }
    }

    void paintEuclideanRing(juce::Graphics& g, juce::Rectangle<float> plot)
    {
        if (stepCount <= 0)
            return;

        const float cx = plot.getCentreX();
        const float cy = plot.getCentreY();
        const float radius = juce::jmin(plot.getWidth(), plot.getHeight()) * 0.38f;
        const int head = playheadIndex % stepCount;
        const float headAng = -juce::MathConstants<float>::halfPi
                              + juce::MathConstants<float>::twoPi * static_cast<float>(head) / static_cast<float>(stepCount);

        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 1.0f);

        g.setColour(familyColour.withAlpha(0.7f));
        g.drawLine(cx, cy,
                   cx + std::cos(headAng) * radius * 0.62f,
                   cy + std::sin(headAng) * radius * 0.62f,
                   1.25f);

        const float arc = juce::MathConstants<float>::twoPi * radius / static_cast<float>(stepCount);
        const float cell = juce::jlimit(7.0f, 22.0f, arc * 0.62f);

        for (int i = 0; i < stepCount; ++i)
        {
            const float ang = -juce::MathConstants<float>::halfPi
                              + juce::MathConstants<float>::twoPi * static_cast<float>(i) / static_cast<float>(stepCount);
            auto well = juce::Rectangle<float>(cell, cell)
                            .withCentre({ cx + std::cos(ang) * radius,
                                          cy + std::sin(ang) * radius });
            paintOneWell(g, well, steps[static_cast<size_t>(i)], i == head, 1.0f, familyColour);
        }
        paintScrollMarks(g, plot);
    }

    void paintDensityColumn(juce::Graphics& g, juce::Rectangle<float> column) const
    {
        constexpr int kLanes = 8;
        const float gap = 3.0f;
        const float h = juce::jmax(4.0f, (column.getHeight() - gap * (kLanes - 1)) / kLanes);
        const float w = juce::jmin(column.getWidth(), h * 1.15f);
        const float x = column.getCentreX() - w * 0.5f;
        const int lit = juce::roundToInt(density * static_cast<float>(kLanes));

        for (int i = 0; i < kLanes; ++i)
        {
            const int fromBottom = kLanes - 1 - i;
            auto well = juce::Rectangle<float>(x, column.getY() + (h + gap) * static_cast<float>(i), w, h);
            paintOneWell(g, well, fromBottom < lit, false, fromBottom < lit ? 1.0f : 0.55f);
        }
    }

    struct GridMetrics
    {
        float x0 = 0.0f;
        float cellW = 2.0f;
        float cellH = 2.0f;
        float pitchX = 4.0f;
        float pitchY = 4.0f;
    };

    GridMetrics gridFor(juce::Rectangle<float> plot, int cols, int rows) const
    {
        GridMetrics m;
        cols = juce::jmax(1, cols);
        rows = juce::jmax(1, rows);
        const float rowH = plot.getHeight() / static_cast<float>(rows);
        const float slot = plot.getWidth() / static_cast<float>(cols);
        m.cellH = juce::jmax(3.0f, rowH - 3.0f);
        m.cellW = juce::jmin(juce::jmax(3.0f, slot - 2.0f), m.cellH * 1.35f);
        m.cellH = juce::jmin(m.cellH, m.cellW);
        m.pitchX = m.cellW + 2.0f;
        m.pitchY = rowH;
        const float gridW = m.pitchX * static_cast<float>(cols);
        m.x0 = plot.getX() + juce::jmax(0.0f, (plot.getWidth() - gridW) * 0.5f);
        return m;
    }

    void paintCellHistory(juce::Graphics& g, juce::Rectangle<float> plot)
    {
        if (historyCount <= 0)
            return;

        int cols = 1;
        for (int i = 0; i < historyCount; ++i)
        {
            const int idx = (historyWrite - historyCount + i + kHistory) % kHistory;
            cols = juce::jmax(cols, cellCounts[static_cast<size_t>(idx)]);
        }

        const auto grid = gridFor(plot, cols, kHistory);
        const float yBase = plot.getBottom() - grid.pitchY * static_cast<float>(historyCount);

        for (int age = 0; age < historyCount; ++age)
        {
            const int idx = (historyWrite - historyCount + age + kHistory) % kHistory;
            const int ageFromNewest = historyCount - 1 - age;
            const float alpha = ageFromNewest == 0
                                    ? 1.0f
                                    : 0.28f + 0.45f * static_cast<float>(age) / static_cast<float>(historyCount);
            const float y = yBase + grid.pitchY * static_cast<float>(age);
            const int n = cellCounts[static_cast<size_t>(idx)];

            for (int i = 0; i < n; ++i)
            {
                auto cell = juce::Rectangle<float>(grid.x0 + grid.pitchX * static_cast<float>(i),
                                                   y,
                                                   grid.cellW,
                                                   grid.cellH);
                const bool on = cellHistory[static_cast<size_t>(idx)][static_cast<size_t>(i)];
                paintOneWell(g, cell, on, false, alpha, familyColour);
            }
        }
        paintScrollMarks(g, plot);
    }

    void paintGrowth(juce::Graphics& g, juce::Rectangle<float> plot)
    {
        const int rows = symbolRows.size();
        if (rows <= 0)
            return;

        int cols = 1;
        for (int r = 0; r < rows; ++r)
            cols = juce::jmax(cols, symbolRows[r].length());

        const auto grid = gridFor(plot, cols, rows);

        for (int r = 0; r < rows; ++r)
        {
            const auto& text = symbolRows[r];
            const bool newest = (r == rows - 1);
            const float alpha = newest ? 1.0f : 0.35f + 0.4f * static_cast<float>(r) / static_cast<float>(rows);
            const float y = plot.getY() + grid.pitchY * static_cast<float>(r);
            const int n = text.length();
            const int head = newest ? (playheadIndex % juce::jmax(1, n)) : -1;

            for (int i = 0; i < n; ++i)
            {
                auto cell = juce::Rectangle<float>(grid.x0 + grid.pitchX * static_cast<float>(i),
                                                   y,
                                                   grid.cellW,
                                                   grid.cellH);
                const juce::juce_wchar ch = text[i];
                const bool note = (ch >= 'A' && ch <= 'G');
                const bool playhead = newest && i == head;
                paintOneWell(g, cell, note, playhead, alpha, familyColour);
                if (grid.cellW > 12.0f)
                {
                    g.setColour(juce::Colour(CustomLookAndFeel::GOLD_TEMPLE).withAlpha(0.92f * alpha));
                    g.setFont(juce::FontOptions(juce::jmin(13.0f, grid.cellW * 0.62f)));
                    g.drawText(juce::String::charToString(ch), cell, juce::Justification::centred, false);
                }
            }
        }
        paintScrollMarks(g, plot);
    }

    int traceIndex(int ageFromOldest) const
    {
        const int start = (traceWrite - traceCount + kTrace) % kTrace;
        return (start + ageFromOldest) % kTrace;
    }

    void paintPitchChain(juce::Graphics& g, juce::Rectangle<float> plot)
    {
        auto field = plot;
        if (showDensity)
        {
            auto column = field.removeFromLeft(16.0f);
            field.removeFromLeft(12.0f);
            paintDensityColumn(g, column);
        }

        paintScopeBackdrop(g, field);
        if (traceCount <= 0)
            return;

        const float cell = juce::jlimit(8.0f, 16.0f, field.getWidth() / static_cast<float>(juce::jmax(traceCount, 12)) * 0.45f);
        auto pointFor = [&](int i)
        {
            const float x = field.getX() + field.getWidth() * (static_cast<float>(i) + 0.5f) / static_cast<float>(traceCount);
            const float yNorm = traceY[static_cast<size_t>(traceIndex(i))];
            const float y = field.getBottom() - cell - yNorm * (field.getHeight() - cell);
            return juce::Point<float>(x, y + cell * 0.5f);
        };

        const auto accent = familyColour;
        for (int i = 1; i < traceCount; ++i)
        {
            const float alpha = 0.12f + 0.38f * static_cast<float>(i) / static_cast<float>(traceCount);
            const auto a = pointFor(i - 1);
            const auto b = pointFor(i);
            g.setColour(accent.withAlpha(alpha));
            g.drawLine(a.x, a.y, b.x, b.y, 1.2f);
        }

        for (int i = 0; i < traceCount; ++i)
        {
            const auto p = pointFor(i);
            const bool newest = i == traceCount - 1;
            const float alpha = newest ? 1.0f : 0.28f + 0.55f * static_cast<float>(i) / static_cast<float>(traceCount);
            paintOneWell(g, juce::Rectangle<float>(cell, cell).withCentre(p), true, newest, alpha, familyColour);
        }
        paintScrollMarks(g, field);
    }

    void paintPlayheadDot(juce::Graphics& g, juce::Point<float> at) const
    {
        paintRoleMark(g, juce::Rectangle<float>(10.0f, 10.0f).withCentre(at),
                      Role::Melody, familyColour, 1.0f);
    }

    void paintScope(juce::Graphics& g, juce::Rectangle<float> plot, bool stairs)
    {
        paintScopeBackdrop(g, plot);
        if (traceCount <= 0)
            return;

        auto pointFor = [&](int i)
        {
            const float denom = static_cast<float>(juce::jmax(1, traceCount - 1));
            const float x = plot.getX() + plot.getWidth() * static_cast<float>(i) / denom;
            const float y = plot.getBottom() - traceY[static_cast<size_t>(traceIndex(i))] * plot.getHeight();
            return juce::Point<float>(x, juce::jlimit(plot.getY(), plot.getBottom(), y));
        };

        const auto accent = familyColour;

        if (stairs)
        {
            const float treadH = juce::jlimit(6.0f, 10.0f, plot.getHeight() * 0.045f);
            const float slot = plot.getWidth() / static_cast<float>(traceCount);
            for (int i = 0; i < traceCount; ++i)
            {
                const float x = plot.getX() + slot * static_cast<float>(i);
                const float y = plot.getBottom() - traceY[static_cast<size_t>(traceIndex(i))] * plot.getHeight();
                const bool newest = i == traceCount - 1;
                const float alpha = newest ? 1.0f : 0.25f + 0.5f * static_cast<float>(i + 1) / static_cast<float>(traceCount);
                auto tread = juce::Rectangle<float>(x + 1.0f, y - treadH * 0.5f, juce::jmax(4.0f, slot - 3.0f), treadH);
                paintOneWell(g, tread, true, newest, alpha, familyColour);

                if (i > 0)
                {
                    const float prevY = plot.getBottom() - traceY[static_cast<size_t>(traceIndex(i - 1))] * plot.getHeight();
                    g.setColour(accent.withAlpha(alpha * 0.7f));
                    g.drawLine(x, prevY, x, y, 1.2f);
                }
            }
        }
        else
        {
            for (int i = 1; i < traceCount; ++i)
            {
                const float alpha = 0.12f + 0.78f * static_cast<float>(i) / static_cast<float>(juce::jmax(1, traceCount - 1));
                const auto a = pointFor(i - 1);
                const auto b = pointFor(i);
                g.setColour(accent.withAlpha(alpha));
                g.drawLine(a.x, a.y, b.x, b.y, 1.8f);
            }

            const auto end = pointFor(traceCount - 1);
            g.setColour(juce::Colours::white.withAlpha(0.16f));
            g.drawVerticalLine(juce::roundToInt(end.x), plot.getY(), plot.getBottom());
            paintPlayheadDot(g, end);
        }
        paintScrollMarks(g, plot);
    }

    void paintOrbit(juce::Graphics& g, juce::Rectangle<float> plot)
    {
        // Engine values are already 0..1. A fitted zoom turns a short arc into a triangle.
        auto field = plot.reduced(6.0f, 2.0f);
        g.setColour(juce::Colours::white.withAlpha(0.06f));
        g.drawLine(field.getCentreX(), field.getY(), field.getCentreX(), field.getBottom(), 1.0f);
        g.drawLine(field.getX(), field.getCentreY(), field.getRight(), field.getCentreY(), 1.0f);

        if (traceCount < 2)
            return;

        auto mapPoint = [&](int i)
        {
            const int idx = traceIndex(i);
            const float nx = juce::jlimit(0.0f, 1.0f, traceX[static_cast<size_t>(idx)]);
            const float ny = juce::jlimit(0.0f, 1.0f, traceY[static_cast<size_t>(idx)]);
            return juce::Point<float>(field.getX() + nx * field.getWidth(),
                                      field.getBottom() - ny * field.getHeight());
        };

        const auto accent = familyColour;
        for (int i = 1; i < traceCount; ++i)
        {
            const float alpha = 0.08f + 0.82f * static_cast<float>(i) / static_cast<float>(traceCount - 1);
            const auto a = mapPoint(i - 1);
            const auto b = mapPoint(i);
            g.setColour(accent.withAlpha(alpha));
            g.drawLine(a.x, a.y, b.x, b.y, 1.5f);
        }

        paintPlayheadDot(g, mapPoint(traceCount - 1));
        paintScrollMarks(g, field);
    }

    Mode mode = Mode::Steps;
    int generatorId = -1;
    juce::Colour familyColour { juce::Colour(CustomLookAndFeel::GOLD_TEMPLE) };
    juce::String caption;
    juce::String described;
    juce::String symbolText;
    juce::StringArray symbolRows;
    std::array<bool, kMaxSteps> steps {};
    std::array<std::array<bool, kMaxSteps>, kHistory> cellHistory {};
    std::array<int, kHistory> cellCounts {};
    int historyCount = 0;
    int historyWrite = 0;
    int stepCount = 0;
    int playheadIndex = 0;
    std::array<float, kTrace> traceX {};
    std::array<float, kTrace> traceY {};
    int traceCount = 0;
    int traceWrite = 0;
    float lastPushed = -1000.0f;
    float density = 0.0f;
    bool showDensity = false;
    bool monoVoice = false;
    int partCount = 1;
    int partStep = -1;
    int barSixteenths = 16;
    float gateSeconds = 0.2f;
    PartClock clock;
    static constexpr int kMelody = 8;
    std::array<int, kMelody> melodyPitch {};
    std::array<uint32_t, kMelody> melodyAt {};
    int melodyCount = 0;
    int melodyWrite = 0;
    int chordRoot = 48;
    int chordThird = 52;
    int chordFifth = 55;
    int pitchLow = 48;
    int pitchHigh = 72;
    std::array<ScrollHit, kScroll> scrollHits {};
    int scrollCount = 0;
    int scrollWrite = 0;
    uint32_t stampedRoot = 0;
    uint32_t stampedChord = 0;
    uint32_t stampedArp = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternVisualizer)
};
