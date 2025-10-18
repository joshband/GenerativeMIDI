/*
  ==============================================================================
    GateLengthController.h

    Controls note duration (gate length) as a percentage of step duration
    Supports global gate length, legato mode, and per-step variation

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <random>
#include <chrono>

class GateLengthController
{
public:
    GateLengthController() = default;

    /**
     * Set the global gate length as a percentage of step duration
     * @param percentage 0.0 (0%) to 1.0 (100%)
     */
    void setGateLength(float percentage)
    {
        gateLength = juce::jlimit(0.01f, 2.0f, percentage); // Allow > 100% for overlapping notes
    }

    /**
     * Enable/disable legato mode (notes overlap, no gaps)
     */
    void setLegatoMode(bool enabled)
    {
        legatoMode = enabled;
    }

    /**
     * Set random gate length variation
     * @param amount 0.0 (no variation) to 1.0 (maximum variation)
     */
    void setGateRandomization(float amount)
    {
        gateRandomization = juce::jlimit(0.0f, 1.0f, amount);
    }

    /**
     * Calculate the actual gate length in samples for a given step
     * @param samplesPerStep Duration of one step in samples
     * @return Gate length in samples
     */
    int calculateGateLengthSamples(int samplesPerStep)
    {
        float baseGate = legatoMode ? 1.0f : gateLength;

        // Apply randomization
        float variation = 0.0f;
        if (gateRandomization > 0.0f)
        {
            std::normal_distribution<float> dist(0.0f, gateRandomization * 0.2f);
            variation = dist(generator);
        }

        float finalGate = juce::jlimit(0.01f, 2.0f, baseGate + variation);
        return static_cast<int>(samplesPerStep * finalGate);
    }

    /**
     * Get the current gate length percentage
     */
    float getGateLength() const { return gateLength; }

    /**
     * Check if legato mode is enabled
     */
    bool isLegatoMode() const { return legatoMode; }

    /**
     * Get the gate randomization amount
     */
    float getGateRandomization() const { return gateRandomization; }

private:
    float gateLength = 0.8f;           // 80% default gate length
    bool legatoMode = false;            // Legato off by default
    float gateRandomization = 0.0f;     // No randomization by default

    std::default_random_engine generator{static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count())};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GateLengthController)
};
