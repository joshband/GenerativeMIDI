/*
  ==============================================================================
    ClockManager.cpp

    Clock management implementation

  ==============================================================================
*/

#include "ClockManager.h"

ClockManager::ClockManager()
{
}

void ClockManager::setTempo(double bpm)
{
    tempo = juce::jlimit(20.0, 400.0, bpm);
}

void ClockManager::setTimeSignature(int numerator, int denominator)
{
    timeSignatureNum = juce::jlimit(1, 32, numerator);
    timeSignatureDenom = juce::jlimit(1, 32, denominator);
}

void ClockManager::setSampleRate(double rate)
{
    sampleRate = rate;
}

void ClockManager::start()
{
    playing = true;
}

void ClockManager::stop()
{
    playing = false;
}

void ClockManager::reset()
{
    currentSample = 0;
    subdivisionCounter = 0;
    midiClockCounter = 0;
}

void ClockManager::advance(int numSamples)
{
    if (!playing || externalSync)
        return;

    int64_t previousSample = currentSample;
    currentSample += numSamples;

    // Check for subdivision hits
    double samplesPerSixteenth = getSamplesPerSubdivision(16);

    int previousSubdivision = static_cast<int>(previousSample / samplesPerSixteenth);
    int currentSubdivision = static_cast<int>(currentSample / samplesPerSixteenth);

    if (currentSubdivision > previousSubdivision && onSubdivisionHit)
    {
        // Fire callback for each subdivision crossed
        for (int sub = previousSubdivision + 1; sub <= currentSubdivision; ++sub)
        {
            onSubdivisionHit(16); // 16th note subdivision
        }
    }
}

double ClockManager::getPositionInBeats() const
{
    return currentSample / getSamplesPerBeat();
}

double ClockManager::getPositionInBars() const
{
    return getPositionInBeats() / timeSignatureNum;
}

double ClockManager::getSamplesPerBeat() const
{
    // Quarter note at current tempo
    return (60.0 / tempo) * sampleRate;
}

double ClockManager::getSamplesPerBar() const
{
    return getSamplesPerBeat() * timeSignatureNum * (4.0 / timeSignatureDenom);
}

double ClockManager::getSamplesPerSubdivision(int subdivision) const
{
    // subdivision is in notes per quarter (e.g., 16 = sixteenth notes)
    return getSamplesPerBeat() / (subdivision / 4.0);
}

int ClockManager::quantizeToSubdivision(int subdivision) const
{
    double samplesPerSub = getSamplesPerSubdivision(subdivision);
    int quantized = static_cast<int>(std::round(currentSample / samplesPerSub) * samplesPerSub);
    return quantized;
}

bool ClockManager::isOnSubdivision(int subdivision) const
{
    double samplesPerSub = getSamplesPerSubdivision(subdivision);
    double remainder = std::fmod(static_cast<double>(currentSample), samplesPerSub);
    return remainder < 1.0; // Within 1 sample tolerance
}

void ClockManager::processExternalMidiClock(const juce::MidiMessage& message)
{
    if (!externalSync)
        return;

    if (message.isMidiClock())
    {
        midiClockCounter++;

        // MIDI clock runs at 24 ppqn (pulses per quarter note)
        if (midiClockCounter >= 24)
        {
            midiClockCounter = 0;
            if (onSubdivisionHit)
                onSubdivisionHit(4); // Quarter note
        }
        else if (midiClockCounter % 6 == 0 && onSubdivisionHit)
        {
            onSubdivisionHit(16); // Sixteenth note
        }
    }
    else if (message.isMidiStart())
    {
        reset();
        start();
    }
    else if (message.isMidiStop())
    {
        stop();
    }
    else if (message.isMidiContinue())
    {
        start();
    }
}

void ClockManager::setExternalSync(bool enabled)
{
    externalSync = enabled;
    if (enabled)
        midiClockCounter = 0;
}
