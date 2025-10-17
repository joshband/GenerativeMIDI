/*
  ==============================================================================
    EuclideanEngine.h

    Euclidean rhythm generator using Björklund's algorithm
    Generates evenly-distributed rhythmic patterns

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

class EuclideanEngine
{
public:
    EuclideanEngine();
    ~EuclideanEngine() = default;

    // Pattern generation
    void setSteps(int numSteps);
    void setPulses(int numPulses);
    void setRotation(int rotation);
    void setAccentPattern(const std::vector<float>& accents);

    // Pattern retrieval
    bool getStep(int stepIndex) const;
    float getVelocity(int stepIndex) const;
    int getSteps() const { return steps; }
    int getPulses() const { return pulses; }

    // Pattern modification
    void rotate(int amount);
    void randomize(float density);

    // Reset
    void regeneratePattern();

private:
    void generateEuclideanPattern();
    std::vector<bool> bjorklund(int pulses, int steps);

    int steps = 16;
    int pulses = 4;
    int rotation = 0;

    std::vector<bool> pattern;
    std::vector<float> velocities;
    std::vector<float> accentPattern;

    juce::Random random;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EuclideanEngine)
};
