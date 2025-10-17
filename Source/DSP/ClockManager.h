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
    bool isPlaying() const { return playing; }

    // Time advancement
    void advance(int numSamples);

    // Position queries
    double getPositionInBeats() const;
    double getPositionInBars() const;
    int getPositionInSamples() const { return currentSample; }

    // Subdivision timing
    double getSamplesPerBeat() const;
    double getSamplesPerBar() const;
    double getSamplesPerSubdivision(int subdivision) const;

    // Callbacks for subdivision hits
    std::function<void(int subdivision)> onSubdivisionHit;

    // Quantization
    int quantizeToSubdivision(int subdivision) const;
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

    // External sync
    bool externalSync = false;
    int midiClockCounter = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClockManager)
};
