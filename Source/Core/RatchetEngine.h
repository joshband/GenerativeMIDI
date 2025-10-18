/*
  ==============================================================================
    RatchetEngine.h

    Controls note ratcheting (repeats) at subdivisions
    Adds rhythmic complexity by retriggering notes multiple times per step

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <random>
#include <chrono>

class RatchetEngine
{
public:
    RatchetEngine() = default;

    /**
     * Set the number of ratchet repeats per step
     * @param count 1 (no ratcheting) to 16 repeats
     */
    void setRatchetCount(int count)
    {
        ratchetCount = juce::jlimit(1, 16, count);
    }

    /**
     * Set the subdivision for ratchet repeats
     * @param division 0=16th notes, 1=32nd notes, 2=64th notes
     */
    void setRatchetDivision(int division)
    {
        ratchetDivision = juce::jlimit(0, 2, division);
    }

    /**
     * Set the probability that ratcheting will occur
     * @param probability 0.0 (never) to 1.0 (always)
     */
    void setRatchetProbability(float probability)
    {
        ratchetProbability = juce::jlimit(0.0f, 1.0f, probability);
    }

    /**
     * Set velocity decay for ratchet repeats
     * @param decay 0.0 (no decay) to 1.0 (maximum decay)
     */
    void setVelocityDecay(float decay)
    {
        velocityDecay = juce::jlimit(0.0f, 1.0f, decay);
    }

    /**
     * Check if this step should have ratcheting applied
     * Uses probability to determine if ratcheting occurs
     */
    bool shouldRatchet()
    {
        if (ratchetCount <= 1 || ratchetProbability <= 0.0f)
            return false;

        return distribution(generator) < ratchetProbability;
    }

    /**
     * Calculate ratchet timing offsets for a given step
     * @param samplesPerStep Duration of one step in samples
     * @return Vector of sample offsets for each ratchet repeat
     */
    std::vector<int> calculateRatchetOffsets(int samplesPerStep) const
    {
        std::vector<int> offsets;

        if (ratchetCount <= 1)
        {
            offsets.push_back(0); // No ratcheting, single trigger
            return offsets;
        }

        // Calculate subdivision based on ratchet division setting
        // 0 = 16th notes (no subdivision)
        // 1 = 32nd notes (2x subdivision)
        // 2 = 64th notes (4x subdivision)
        int subdivisionMultiplier = 1 << ratchetDivision; // 1, 2, or 4
        int samplesPerRatchet = samplesPerStep / (ratchetCount * subdivisionMultiplier);

        for (int i = 0; i < ratchetCount; ++i)
        {
            offsets.push_back(i * samplesPerRatchet * subdivisionMultiplier);
        }

        return offsets;
    }

    /**
     * Calculate velocity for a specific ratchet repeat
     * @param baseVelocity Original note velocity
     * @param ratchetIndex Which repeat this is (0 = first, 1 = second, etc.)
     * @return Modified velocity with decay applied
     */
    float calculateRatchetVelocity(float baseVelocity, int ratchetIndex) const
    {
        if (ratchetIndex == 0 || velocityDecay <= 0.0f)
            return baseVelocity;

        // Apply exponential decay: velocity = base * (1 - decay)^index
        float decayFactor = std::pow(1.0f - velocityDecay, static_cast<float>(ratchetIndex));
        float newVelocity = baseVelocity * decayFactor;

        return juce::jlimit(0.01f, 1.0f, newVelocity);
    }

    /**
     * Get current ratchet count
     */
    int getRatchetCount() const { return ratchetCount; }

    /**
     * Get current ratchet division
     */
    int getRatchetDivision() const { return ratchetDivision; }

    /**
     * Get current ratchet probability
     */
    float getRatchetProbability() const { return ratchetProbability; }

    /**
     * Get current velocity decay
     */
    float getVelocityDecay() const { return velocityDecay; }

private:
    int ratchetCount = 1;              // Number of repeats (1 = no ratcheting)
    int ratchetDivision = 0;           // 0=16th, 1=32nd, 2=64th
    float ratchetProbability = 0.0f;   // Chance of ratcheting (0-1)
    float velocityDecay = 0.5f;        // Velocity reduction per repeat

    std::default_random_engine generator{static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count())};
    std::uniform_real_distribution<float> distribution{0.0f, 1.0f};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RatchetEngine)
};
