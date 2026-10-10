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
    samplesToNextSixteenth = 0.0;
}

void ClockManager::restart(double positionInSixteenths)
{
    const double position = juce::jmax(0.0, positionInSixteenths);
    const double samplesPerSixteenth = getSamplesPerSubdivision(16);

    reset();
    currentSample = static_cast<int64_t>(std::llround(position * samplesPerSixteenth));

    const double fraction = position - std::floor(position);
    samplesToNextSixteenth = fraction < 1.0e-6 ? 0.0 : (1.0 - fraction) * samplesPerSixteenth;
}

void ClockManager::advance(int numSamples)
{
    if (!playing || externalSync || numSamples <= 0)
        return;

    // Walk the sixteenth grid through this block and tell the listener where in the
    // block each hit falls. The fractional remainder carries over, so the grid stays
    // exact (no per-block rounding drift) and a tempo change takes effect on the next step.
    const double samplesPerSixteenth = getSamplesPerSubdivision(16);
    double cursor = samplesToNextSixteenth;

    while (cursor < static_cast<double>(numSamples))
    {
        const int offset = juce::jlimit(0, numSamples - 1, static_cast<int>(std::llround(cursor)));

        if (onSubdivisionHitAt)
            onSubdivisionHitAt(16, offset);
        else if (onSubdivisionHit)
            onSubdivisionHit(16); // 16th note subdivision

        cursor += samplesPerSixteenth;
    }

    samplesToNextSixteenth = cursor - static_cast<double>(numSamples);
    currentSample += numSamples;
}

double ClockManager::getPositionInBeats() const
{
    return static_cast<double>(currentSample) / getSamplesPerBeat();
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

int64_t ClockManager::quantizeToSubdivision(int subdivision) const
{
    double samplesPerSub = getSamplesPerSubdivision(subdivision);
    return static_cast<int64_t>(std::llround(std::round(static_cast<double>(currentSample) / samplesPerSub) * samplesPerSub));
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
