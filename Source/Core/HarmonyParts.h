/*
  ==============================================================================
    HarmonyParts.h

    Scale triad and MIDI channel offsets for root, chord, and arp parts.
    Fixed arithmetic only — no allocation.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <cstdlib>

namespace HarmonyParts
{
    inline int roleChannel(int baseChannel, int offset) noexcept
    {
        const int base = juce::jlimit(1, 16, baseChannel);
        return ((base - 1 + offset) % 16) + 1;
    }

    struct Triad
    {
        int root = 48;
        int third = 52;
        int fifth = 55;

        int tone(int index) const noexcept
        {
            const int wrapped = index % 3;
            if (wrapped == 0)
                return root;
            return wrapped == 1 ? third : fifth;
        }
    };

    inline int noteAtOrBelow(int pitchClass, int ceiling) noexcept
    {
        pitchClass = (pitchClass % 12 + 12) % 12;
        ceiling = juce::jlimit(0, 127, ceiling);
        int note = (ceiling / 12) * 12 + pitchClass;
        if (note > ceiling)
            note -= 12;
        return juce::jlimit(0, 127, note);
    }

    inline int noteAtOrAbove(int pitchClass, int floor) noexcept
    {
        pitchClass = (pitchClass % 12 + 12) % 12;
        floor = juce::jlimit(0, 127, floor);
        int note = (floor / 12) * 12 + pitchClass;
        if (note < floor)
            note += 12;
        return juce::jlimit(0, 127, note);
    }

    inline Triad triadForMelody(int melodyNote,
                                int rootPitchClass,
                                const int* intervals,
                                int intervalCount,
                                bool chromatic) noexcept
    {
        melodyNote = juce::jlimit(0, 127, melodyNote);
        rootPitchClass = (rootPitchClass % 12 + 12) % 12;

        int rootClass = melodyNote % 12;
        int thirdClass = (rootClass + 4) % 12;
        int fifthClass = (rootClass + 7) % 12;

        if (!chromatic && intervals != nullptr && intervalCount > 0)
        {
            const int fromRoot = (rootClass - rootPitchClass + 12) % 12;
            int degree = 0;
            int bestDistance = 99;
            for (int i = 0; i < intervalCount; ++i)
            {
                const int distance = std::abs(fromRoot - intervals[i]);
                if (distance < bestDistance)
                {
                    bestDistance = distance;
                    degree = i;
                }
            }

            auto pitchClassAt = [&](int steps)
            {
                int index = degree + steps;
                index %= intervalCount;
                if (index < 0)
                    index += intervalCount;
                return (rootPitchClass + intervals[index]) % 12;
            };

            rootClass = pitchClassAt(0);
            thirdClass = pitchClassAt(2);
            fifthClass = pitchClassAt(4);
        }

        Triad triad;
        triad.root = noteAtOrBelow(rootClass, juce::jmax(0, melodyNote - 12));
        triad.third = noteAtOrAbove(thirdClass, triad.root + 1);
        triad.fifth = noteAtOrAbove(fifthClass, triad.third + 1);
        return triad;
    }
}
