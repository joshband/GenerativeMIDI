/*
  ==============================================================================
    ScaleQuantizer.h

    Musical scale quantization system for constraining MIDI notes to scales

  ==============================================================================
*/

#pragma once

#include <vector>
#include <array>
#include <algorithm>
#include <cmath>

/**
 * @brief Quantizes MIDI note numbers to musical scales
 *
 * Ensures generated notes conform to specific musical scales/modes,
 * making output more musically coherent.
 */
class ScaleQuantizer
{
public:
    /**
     * @brief Supported musical scales and modes
     */
    enum class Scale
    {
        Chromatic,          // All 12 notes
        Major,              // Ionian mode
        Minor,              // Natural minor (Aeolian)
        HarmonicMinor,      // Minor with raised 7th
        MelodicMinor,       // Minor with raised 6th and 7th
        Dorian,             // 2nd mode of major
        Phrygian,           // 3rd mode of major
        Lydian,             // 4th mode of major
        Mixolydian,         // 5th mode of major
        Locrian,            // 7th mode of major
        MajorPentatonic,    // 5-note major scale
        MinorPentatonic,    // 5-note minor scale
        Blues,              // Blues scale with blue notes
        WholeTone,          // 6 whole tones
        Diminished,         // Half-whole diminished
        HarmonicMajor,      // Major with lowered 6th
        Custom              // User-defined scale
    };

    ScaleQuantizer()
        : currentScale(Scale::Chromatic)
        , rootNote(60) // Middle C
    {
        initializeScales();
    }

    /**
     * @brief Set the root note for the scale (0-127)
     */
    void setRootNote(int root)
    {
        rootNote = root % 12; // Store as pitch class
    }

    /**
     * @brief Get current root note
     */
    int getRootNote() const { return rootNote; }

    /**
     * @brief Set the current scale/mode
     */
    void setScale(Scale scale)
    {
        currentScale = scale;
    }

    /**
     * @brief Get the current scale
     */
    Scale getScale() const { return currentScale; }

    /**
     * @brief Define a custom scale using semitone intervals from root
     * @param intervals Vector of semitone intervals (e.g., {0, 2, 4, 5, 7, 9, 11} for major)
     */
    void setCustomScale(const std::vector<int>& intervals)
    {
        customScaleIntervals = intervals;
        currentScale = Scale::Custom;
    }

    /**
     * @brief Quantize a MIDI note to the nearest note in the current scale
     * @param midiNote Input MIDI note (0-127)
     * @return Quantized MIDI note (0-127)
     */
    int quantize(int midiNote) const
    {
        if (currentScale == Scale::Chromatic)
            return midiNote; // No quantization needed

        // Get the intervals for current scale
        const auto& intervals = getScaleIntervals();
        if (intervals.empty())
            return midiNote; // Fallback if scale not defined

        // Break note into octave and pitch class
        int octave = midiNote / 12;
        int pitchClass = midiNote % 12;

        // Find nearest note in scale
        int nearestInterval = findNearestInterval(pitchClass, intervals);

        // Reconstruct MIDI note
        return octave * 12 + ((rootNote + nearestInterval) % 12);
    }

    /**
     * @brief Quantize to next higher note in scale (useful for arpeggios)
     */
    int quantizeUp(int midiNote) const
    {
        if (currentScale == Scale::Chromatic)
            return midiNote;

        const auto& intervals = getScaleIntervals();
        if (intervals.empty())
            return midiNote;

        int octave = midiNote / 12;
        int pitchClass = midiNote % 12;

        // Find next higher interval
        int targetPitch = (pitchClass - rootNote + 12) % 12;

        for (int interval : intervals)
        {
            if (interval > targetPitch)
                return octave * 12 + ((rootNote + interval) % 12);
        }

        // Wrap to next octave if we're past all intervals
        return (octave + 1) * 12 + ((rootNote + intervals[0]) % 12);
    }

    /**
     * @brief Quantize to next lower note in scale
     */
    int quantizeDown(int midiNote) const
    {
        if (currentScale == Scale::Chromatic)
            return midiNote;

        const auto& intervals = getScaleIntervals();
        if (intervals.empty())
            return midiNote;

        int octave = midiNote / 12;
        int pitchClass = midiNote % 12;

        // Find next lower interval
        int targetPitch = (pitchClass - rootNote + 12) % 12;

        for (int i = static_cast<int>(intervals.size()) - 1; i >= 0; --i)
        {
            if (intervals[i] < targetPitch)
                return octave * 12 + ((rootNote + intervals[i]) % 12);
        }

        // Wrap to previous octave if we're below all intervals
        return (octave - 1) * 12 + ((rootNote + intervals.back()) % 12);
    }

    /**
     * @brief Get the name of a scale
     */
    static const char* getScaleName(Scale scale)
    {
        switch (scale)
        {
            case Scale::Chromatic:          return "Chromatic";
            case Scale::Major:              return "Major";
            case Scale::Minor:              return "Minor";
            case Scale::HarmonicMinor:      return "Harmonic Minor";
            case Scale::MelodicMinor:       return "Melodic Minor";
            case Scale::Dorian:             return "Dorian";
            case Scale::Phrygian:           return "Phrygian";
            case Scale::Lydian:             return "Lydian";
            case Scale::Mixolydian:         return "Mixolydian";
            case Scale::Locrian:            return "Locrian";
            case Scale::MajorPentatonic:    return "Major Pentatonic";
            case Scale::MinorPentatonic:    return "Minor Pentatonic";
            case Scale::Blues:              return "Blues";
            case Scale::WholeTone:          return "Whole Tone";
            case Scale::Diminished:         return "Diminished";
            case Scale::HarmonicMajor:      return "Harmonic Major";
            case Scale::Custom:             return "Custom";
            default:                        return "Unknown";
        }
    }

    /**
     * @brief Get scale intervals for current scale
     */
    const std::vector<int>& getScaleIntervals() const
    {
        if (currentScale == Scale::Custom)
            return customScaleIntervals;

        auto it = scaleIntervals.find(currentScale);
        if (it != scaleIntervals.end())
            return it->second;

        static const std::vector<int> chromatic = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        return chromatic;
    }

private:
    Scale currentScale;
    int rootNote; // Stored as pitch class (0-11)
    std::vector<int> customScaleIntervals;
    std::map<Scale, std::vector<int>> scaleIntervals;

    /**
     * @brief Initialize all scale interval patterns
     */
    void initializeScales()
    {
        // Intervals are semitones from root
        scaleIntervals[Scale::Chromatic] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        scaleIntervals[Scale::Major] = {0, 2, 4, 5, 7, 9, 11};
        scaleIntervals[Scale::Minor] = {0, 2, 3, 5, 7, 8, 10};
        scaleIntervals[Scale::HarmonicMinor] = {0, 2, 3, 5, 7, 8, 11};
        scaleIntervals[Scale::MelodicMinor] = {0, 2, 3, 5, 7, 9, 11};
        scaleIntervals[Scale::Dorian] = {0, 2, 3, 5, 7, 9, 10};
        scaleIntervals[Scale::Phrygian] = {0, 1, 3, 5, 7, 8, 10};
        scaleIntervals[Scale::Lydian] = {0, 2, 4, 6, 7, 9, 11};
        scaleIntervals[Scale::Mixolydian] = {0, 2, 4, 5, 7, 9, 10};
        scaleIntervals[Scale::Locrian] = {0, 1, 3, 5, 6, 8, 10};
        scaleIntervals[Scale::MajorPentatonic] = {0, 2, 4, 7, 9};
        scaleIntervals[Scale::MinorPentatonic] = {0, 3, 5, 7, 10};
        scaleIntervals[Scale::Blues] = {0, 3, 5, 6, 7, 10};
        scaleIntervals[Scale::WholeTone] = {0, 2, 4, 6, 8, 10};
        scaleIntervals[Scale::Diminished] = {0, 2, 3, 5, 6, 8, 9, 11};
        scaleIntervals[Scale::HarmonicMajor] = {0, 2, 4, 5, 7, 8, 11};
    }

    /**
     * @brief Find nearest interval to target pitch within scale
     */
    int findNearestInterval(int pitchClass, const std::vector<int>& intervals) const
    {
        int targetPitch = (pitchClass - rootNote + 12) % 12;
        int nearest = intervals[0];
        int minDistance = std::abs(targetPitch - intervals[0]);

        for (int interval : intervals)
        {
            int distance = std::abs(targetPitch - interval);
            if (distance < minDistance)
            {
                minDistance = distance;
                nearest = interval;
            }
        }

        return nearest;
    }
};
