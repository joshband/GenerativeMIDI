/*
  ==============================================================================
    EuclideanEngine.h

    Euclidean rhythm generator using Björklund's algorithm
    Generates evenly-distributed rhythmic patterns

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class EuclideanEngine
{
public:
    static constexpr int kMaxSteps = 64;

    EuclideanEngine();
    ~EuclideanEngine() = default;

    // Pattern generation
    void setSteps(int numSteps);
    void setPulses(int numPulses);
    void setRotation(int rotation);
    void setAccentPattern(const float* accents, int count);

    // Pattern retrieval
    bool getStep(int stepIndex) const;
    float getVelocity(int stepIndex) const;
    int getSteps() const { return steps; }
    int getPulses() const { return pulses; }
    int getRotation() const { return rotation; }

    // Pattern modification
    void rotate(int amount);
    void randomize(float density);

    // Reset
    void regeneratePattern();

private:
    void generateEuclideanPattern();

    int steps = 16;
    int pulses = 4;
    int rotation = 0;

    // Fixed-capacity storage — regenerate never heap-allocates after construction
    bool pattern[kMaxSteps] {};
    float velocities[kMaxSteps] {};
    float accentPattern[kMaxSteps] {};
    int accentCount = 0;

    juce::Random random;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EuclideanEngine)
};
