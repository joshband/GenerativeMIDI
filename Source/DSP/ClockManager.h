/*
  ==============================================================================
    ClockManager.h

    Real-time clock management for sequencing
    Handles tempo, time signature, and subdivision timing

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class ClockManager
{
public:
    ClockManager();
    ~ClockManager() = default;

    // Tempo and timing
    void setTempo(double bpm);
    double getTempo() const { return tempo; }

    void setTimeSignature(int numerator, int denominator);
    int getTimeSignatureNumerator() const { return timeSignatureNum; }
    int getTimeSignatureDenominator() const { return timeSignatureDenom; }

    // Sample rate
    void setSampleRate(double rate);
    double getSampleRate() const { return sampleRate; }

    // Playback control
    void start();
    void stop();
    void reset();

    /** Play edge: rewind the clock so the next sixteenth lands exactly where the grid says.
        `positionInSixteenths` is the song position (0 = start). If it is not on a
        sixteenth, the first hit is delayed to the next boundary. */
    void restart(double positionInSixteenths = 0.0);
    bool isPlaying() const { return playing; }

    // Time advancement
    void advance(int numSamples);

    // Position queries
    double getPositionInBeats() const;
    double getPositionInBars() const;
    int64_t getPositionInSamples() const { return currentSample; }

    // Subdivision timing
    double getSamplesPerBeat() const;
    double getSamplesPerBar() const;
    double getSamplesPerSubdivision(int subdivision) const;

    // Callbacks for subdivision hits.
    // onSubdivisionHitAt receives the sample offset of the hit inside the block being
    // advanced (0..numSamples-1). When it is set, advance() uses it instead of
    // onSubdivisionHit; onSubdivisionHit remains the callback for MIDI-clock sync,
    // which has no sample offset.
    std::function<void(int subdivision)> onSubdivisionHit;
    std::function<void(int subdivision, int sampleOffset)> onSubdivisionHitAt;

    // Quantization
    int64_t quantizeToSubdivision(int subdivision) const;
    bool isOnSubdivision(int subdivision) const;

    // MIDI clock sync
    void processExternalMidiClock(const juce::MidiMessage& message);
    void setExternalSync(bool enabled);
    bool isExternalSync() const { return externalSync; }

private:
    double tempo = 120.0;
    double sampleRate = 44100.0;
    int timeSignatureNum = 4;
    int timeSignatureDenom = 4;

    bool playing = false;
    int64_t currentSample = 0;
    int subdivisionCounter = 0;
    double samplesToNextSixteenth = 0.0; // distance from the start of the next block; 0 = hit at its first sample

    // External sync
    bool externalSync = false;
    int midiClockCounter = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClockManager)
};
