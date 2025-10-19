/*
  ==============================================================================
    ModulationSource.h

    Base class for modulation sources (LFO, Envelope, Random, etc.)
    Provides normalized 0.0-1.0 output values

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

// ============================================================================
// Modulation Source Types
// ============================================================================
enum class ModulationSourceType
{
    LFO,
    Envelope,
    Random,
    StepSequencer,
    None
};

// ============================================================================
// Base Modulation Source
// ============================================================================
class ModulationSource
{
public:
    ModulationSource() = default;
    virtual ~ModulationSource() = default;

    // Get current modulation value (0.0 - 1.0, can be bipolar -1.0 to +1.0)
    virtual float getValue() const = 0;

    // Advance modulation state
    virtual void advance(double timeSeconds) = 0;

    // Reset to initial state
    virtual void reset() = 0;

    // Get source type
    virtual ModulationSourceType getType() const = 0;

    // Get display name
    virtual juce::String getName() const = 0;

    // Enable/disable
    void setEnabled(bool enabled) { isEnabled = enabled; }
    bool getEnabled() const { return isEnabled; }

    // Bipolar mode (-1 to +1 instead of 0 to 1)
    void setBipolar(bool bipolar) { isBipolar = bipolar; }
    bool getBipolar() const { return isBipolar; }

    // Get normalized value (handles bipolar conversion)
    float getNormalizedValue() const
    {
        if (!isEnabled)
            return isBipolar ? 0.0f : 0.5f;

        float val = getValue();
        if (isBipolar)
            return val; // Already -1 to +1
        else
            return val; // 0 to 1
    }

protected:
    bool isEnabled = true;
    bool isBipolar = false;
    juce::String customName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationSource)
};

// ============================================================================
// LFO Modulation Source
// ============================================================================
class LFOModulationSource : public ModulationSource
{
public:
    enum class Waveform
    {
        Sine,
        Triangle,
        Saw,
        Square,
        Random,
        SampleAndHold
    };

    LFOModulationSource()
    {
        setBipolar(true); // LFOs are typically bipolar
    }

    LFOModulationSource(const juce::String& sourceName, Waveform wave, float freq, bool isBipolar)
        : waveform(wave), rate(freq)
    {
        customName = sourceName;
        setBipolar(isBipolar);
    }

    float getValue() const override
    {
        return currentValue;
    }

    void advance(double timeSeconds) override
    {
        phase += rate * timeSeconds;

        // Wrap phase
        while (phase >= 1.0)
            phase -= 1.0;

        // Calculate waveform
        switch (waveform)
        {
            case Waveform::Sine:
                currentValue = std::sin(phase * juce::MathConstants<float>::twoPi);
                break;

            case Waveform::Triangle:
                currentValue = phase < 0.5
                    ? (phase * 4.0f - 1.0f)
                    : (3.0f - phase * 4.0f);
                break;

            case Waveform::Saw:
                currentValue = phase * 2.0f - 1.0f;
                break;

            case Waveform::Square:
                currentValue = phase < 0.5 ? -1.0f : 1.0f;
                break;

            case Waveform::Random:
            {
                // Smooth random interpolation
                if (phase < lastPhase) // Wrapped
                {
                    randomStart = randomEnd;
                    randomEnd = random.nextFloat() * 2.0f - 1.0f;
                }
                float t = phase;
                currentValue = randomStart + (randomEnd - randomStart) * t;
                break;
            }

            case Waveform::SampleAndHold:
                if (phase < lastPhase) // Wrapped
                    currentValue = random.nextFloat() * 2.0f - 1.0f;
                break;
        }

        lastPhase = phase;
    }

    void reset() override
    {
        phase = 0.0;
        lastPhase = 0.0;
        currentValue = 0.0f;
        randomStart = 0.0f;
        randomEnd = 0.0f;
    }

    ModulationSourceType getType() const override { return ModulationSourceType::LFO; }
    juce::String getName() const override { return customName.isEmpty() ? "LFO" : customName; }

    // LFO-specific parameters
    void setRate(float hz) { rate = juce::jlimit(0.01f, 100.0f, hz); }
    float getRate() const { return rate; }

    void setWaveform(Waveform wf) { waveform = wf; }
    Waveform getWaveform() const { return waveform; }

private:
    float rate = 1.0f; // Hz
    Waveform waveform = Waveform::Sine;
    double phase = 0.0;
    double lastPhase = 0.0;
    mutable float currentValue = 0.0f;

    // Random waveform state
    mutable float randomStart = 0.0f;
    mutable float randomEnd = 0.0f;
    juce::Random random;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LFOModulationSource)
};

// ============================================================================
// Random Modulation Source
// ============================================================================
class RandomModulationSource : public ModulationSource
{
public:
    RandomModulationSource()
    {
        currentValue = random.nextFloat();
    }

    RandomModulationSource(const juce::String& sourceName, float intervalSeconds, bool isBipolar)
        : interval(intervalSeconds)
    {
        customName = sourceName;
        setBipolar(isBipolar);
        currentValue = random.nextFloat();
    }

    float getValue() const override
    {
        return currentValue;
    }

    void advance(double timeSeconds) override
    {
        timer += timeSeconds;

        if (timer >= interval)
        {
            timer = 0.0;
            currentValue = random.nextFloat();
        }
    }

    void reset() override
    {
        timer = 0.0;
        currentValue = random.nextFloat();
    }

    ModulationSourceType getType() const override { return ModulationSourceType::Random; }
    juce::String getName() const override { return customName.isEmpty() ? "Random" : customName; }

    // Random-specific parameters
    void setInterval(float seconds) { interval = juce::jmax(0.01f, seconds); }
    float getInterval() const { return interval; }

private:
    float interval = 0.1f; // Seconds between random values
    double timer = 0.0;
    float currentValue = 0.0f;
    juce::Random random;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RandomModulationSource)
};

// ============================================================================
// Envelope Modulation Source (Simple AD envelope per note)
// ============================================================================
class EnvelopeModulationSource : public ModulationSource
{
public:
    EnvelopeModulationSource() = default;

    EnvelopeModulationSource(const juce::String& sourceName, float attack, float decay, bool isBipolar)
        : attackTime(attack), decayTime(decay)
    {
        customName = sourceName;
        setBipolar(isBipolar);
    }

    enum class Stage
    {
        Idle,
        Attack,
        Decay
    };

    float getValue() const override
    {
        return currentValue;
    }

    void advance(double timeSeconds) override
    {
        switch (stage)
        {
            case Stage::Idle:
                currentValue = 0.0f;
                break;

            case Stage::Attack:
                stageTime += timeSeconds;
                if (stageTime >= attackTime)
                {
                    stage = Stage::Decay;
                    stageTime = 0.0;
                    currentValue = 1.0f;
                }
                else
                {
                    currentValue = stageTime / attackTime;
                }
                break;

            case Stage::Decay:
                stageTime += timeSeconds;
                if (stageTime >= decayTime)
                {
                    stage = Stage::Idle;
                    currentValue = 0.0f;
                }
                else
                {
                    currentValue = 1.0f - (stageTime / decayTime);
                }
                break;
        }
    }

    void reset() override
    {
        stage = Stage::Idle;
        stageTime = 0.0;
        currentValue = 0.0f;
    }

    void trigger()
    {
        stage = Stage::Attack;
        stageTime = 0.0;
    }

    ModulationSourceType getType() const override { return ModulationSourceType::Envelope; }
    juce::String getName() const override { return customName.isEmpty() ? "Envelope" : customName; }

    // Envelope-specific parameters
    void setAttackTime(float seconds) { attackTime = juce::jmax(0.001f, seconds); }
    float getAttackTime() const { return static_cast<float>(attackTime); }

    void setDecayTime(float seconds) { decayTime = juce::jmax(0.001f, seconds); }
    float getDecayTime() const { return static_cast<float>(decayTime); }

private:
    Stage stage = Stage::Idle;
    double stageTime = 0.0;
    float currentValue = 0.0f;
    double attackTime = 0.01; // Seconds
    double decayTime = 0.5;    // Seconds

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeModulationSource)
};
