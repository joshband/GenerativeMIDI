/*
  ==============================================================================
    SwingEngine.h

    Swing and humanization engine for adding groove and timing variation

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <random>

/**
 * @brief Adds swing, groove, and humanization to MIDI timing and velocity
 *
 * Swing: Delays off-beat notes to create rhythmic feel
 * Timing humanization: Adds random micro-timing variations
 * Velocity humanization: Randomizes velocities for natural feel
 */
class SwingEngine
{
public:
    SwingEngine()
        : swingAmount(0.0f)
        , timingRandomness(0.0f)
        , velocityRandomness(0.0f)
        , subdivision(16) // 16th notes by default
        , generator(std::random_device{}())
        , timingDistribution(-1.0f, 1.0f)
        , velocityDistribution(-1.0f, 1.0f)
    {
    }

    /**
     * @brief Set swing amount (0.0 = no swing, 0.5 = triplet feel, 1.0 = maximum swing)
     * @param amount Swing amount (0.0-1.0)
     */
    void setSwingAmount(float amount)
    {
        swingAmount = juce::jlimit(0.0f, 1.0f, amount);
    }

    /**
     * @brief Get current swing amount
     */
    float getSwingAmount() const { return swingAmount; }

    /**
     * @brief Set timing randomness in milliseconds
     * @param milliseconds Random timing variation +/- this amount (0-50ms recommended)
     */
    void setTimingRandomness(float milliseconds)
    {
        timingRandomness = juce::jlimit(0.0f, 100.0f, milliseconds);
    }

    /**
     * @brief Get current timing randomness
     */
    float getTimingRandomness() const { return timingRandomness; }

    /**
     * @brief Set velocity randomness (0.0 = no variation, 1.0 = maximum variation)
     * @param amount Velocity variation amount (0.0-1.0)
     */
    void setVelocityRandomness(float amount)
    {
        velocityRandomness = juce::jlimit(0.0f, 1.0f, amount);
    }

    /**
     * @brief Get current velocity randomness
     */
    float getVelocityRandomness() const { return velocityRandomness; }

    /**
     * @brief Set the subdivision for swing calculation
     * @param div Subdivision (8 = 8th notes, 16 = 16th notes, etc.)
     */
    void setSubdivision(int div)
    {
        subdivision = juce::jmax(1, div);
    }

    /**
     * @brief Get current subdivision
     */
    int getSubdivision() const { return subdivision; }

    /**
     * @brief Calculate swing timing offset for a given step
     * @param stepNumber The step number in the sequence
     * @param samplesPerStep Number of samples per step
     * @return Sample offset to add to note timing
     */
    int calculateSwingOffset(int stepNumber, int samplesPerStep) const
    {
        // Swing only affects off-beats (odd steps in pairs)
        bool isOffBeat = (stepNumber % 2) == 1;

        if (!isOffBeat || swingAmount <= 0.0f)
            return 0;

        // Calculate swing delay
        // At 50% swing, off-beats are delayed by 1/6 of a beat (triplet feel)
        // At 100% swing, off-beats are delayed by 1/3 of a beat
        float swingRatio = 0.166f + (swingAmount * 0.167f); // 16.6% to 33.3%
        int swingOffset = static_cast<int>(samplesPerStep * swingRatio);

        return swingOffset;
    }

    /**
     * @brief Calculate timing humanization offset
     * @param sampleRate Current sample rate
     * @return Random sample offset based on timing randomness
     */
    int calculateTimingHumanization(double sampleRate)
    {
        if (timingRandomness <= 0.0f)
            return 0;

        // Convert milliseconds to samples
        float randomMs = timingDistribution(generator) * timingRandomness;
        int sampleOffset = static_cast<int>((randomMs / 1000.0) * sampleRate);

        return sampleOffset;
    }

    /**
     * @brief Apply velocity humanization to a velocity value
     * @param velocity Input velocity (0.0-1.0)
     * @return Humanized velocity (0.0-1.0)
     */
    float humanizeVelocity(float velocity)
    {
        if (velocityRandomness <= 0.0f)
            return velocity;

        // Apply random variation
        float variation = velocityDistribution(generator) * velocityRandomness * 0.2f; // +/- 20% max
        float humanized = velocity + variation;

        return juce::jlimit(0.0f, 1.0f, humanized);
    }

    /**
     * @brief Calculate combined timing offset (swing + humanization)
     * @param stepNumber Step number in sequence
     * @param samplesPerStep Samples per step
     * @param sampleRate Current sample rate
     * @return Total sample offset
     */
    int calculateTotalTimingOffset(int stepNumber, int samplesPerStep, double sampleRate)
    {
        int swingOffset = calculateSwingOffset(stepNumber, samplesPerStep);
        int humanOffset = calculateTimingHumanization(sampleRate);

        return swingOffset + humanOffset;
    }

    /**
     * @brief Apply groove template (preset swing patterns)
     */
    enum class GrooveTemplate
    {
        None,           // No groove
        Classic,        // 50% swing (triplet feel)
        HardSwing,      // 66% swing
        LightSwing,     // 33% swing
        Shuffle,        // Heavy triplet shuffle
        Dotted,         // Dotted rhythm feel
        Drunk,          // High timing randomness
        Tight,          // Minimal humanization (quantized)
        Loose,          // High humanization (sloppy feel)
    };

    /**
     * @brief Apply a preset groove template
     */
    void applyGrooveTemplate(GrooveTemplate template_type)
    {
        switch (template_type)
        {
            case GrooveTemplate::None:
                setSwingAmount(0.0f);
                setTimingRandomness(0.0f);
                setVelocityRandomness(0.0f);
                break;

            case GrooveTemplate::Classic:
                setSwingAmount(0.5f);
                setTimingRandomness(2.0f);
                setVelocityRandomness(0.1f);
                break;

            case GrooveTemplate::HardSwing:
                setSwingAmount(0.66f);
                setTimingRandomness(3.0f);
                setVelocityRandomness(0.15f);
                break;

            case GrooveTemplate::LightSwing:
                setSwingAmount(0.33f);
                setTimingRandomness(1.5f);
                setVelocityRandomness(0.08f);
                break;

            case GrooveTemplate::Shuffle:
                setSwingAmount(0.75f);
                setTimingRandomness(5.0f);
                setVelocityRandomness(0.2f);
                break;

            case GrooveTemplate::Dotted:
                setSwingAmount(0.4f);
                setTimingRandomness(1.0f);
                setVelocityRandomness(0.05f);
                break;

            case GrooveTemplate::Drunk:
                setSwingAmount(0.0f);
                setTimingRandomness(15.0f);
                setVelocityRandomness(0.3f);
                break;

            case GrooveTemplate::Tight:
                setSwingAmount(0.0f);
                setTimingRandomness(0.5f);
                setVelocityRandomness(0.02f);
                break;

            case GrooveTemplate::Loose:
                setSwingAmount(0.3f);
                setTimingRandomness(10.0f);
                setVelocityRandomness(0.25f);
                break;
        }
    }

    /**
     * @brief Reset randomness seed for consistent patterns
     */
    void resetSeed(unsigned int seed = 0)
    {
        if (seed == 0)
            generator.seed(std::random_device{}());
        else
            generator.seed(seed);
    }

private:
    float swingAmount;           // 0.0-1.0
    float timingRandomness;      // milliseconds
    float velocityRandomness;    // 0.0-1.0
    int subdivision;             // Steps per beat

    // Random number generation
    std::mt19937 generator;
    std::uniform_real_distribution<float> timingDistribution;
    std::uniform_real_distribution<float> velocityDistribution;
};
