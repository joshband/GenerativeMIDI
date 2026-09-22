/*
  ==============================================================================
    EuclideanEngine.cpp

    Implementation of Björklund's algorithm for Euclidean rhythms
    (O(steps) remainder form — equivalent distribution, no heap)

  ==============================================================================
*/

#include "EuclideanEngine.h"

EuclideanEngine::EuclideanEngine()
{
    regeneratePattern();
}

void EuclideanEngine::setSteps(int numSteps)
{
    const int newSteps = juce::jlimit(1, kMaxSteps, numSteps);
    if (newSteps == steps)
        return;
    steps = newSteps;
    pulses = juce::jmin(pulses, steps);
    regeneratePattern();
}

void EuclideanEngine::setPulses(int numPulses)
{
    const int newPulses = juce::jlimit(0, steps, numPulses);
    if (newPulses == pulses)
        return;
    pulses = newPulses;
    regeneratePattern();
}

void EuclideanEngine::setRotation(int rot)
{
    const int newRotation = (steps > 0) ? (rot % steps) : 0;
    if (newRotation == rotation)
        return;
    rotation = newRotation;
    regeneratePattern();
}

void EuclideanEngine::setAccentPattern(const float* accents, int count)
{
    accentCount = juce::jlimit(0, kMaxSteps, count);
    for (int i = 0; i < accentCount; ++i)
        accentPattern[i] = accents[i];
    regeneratePattern();
}

bool EuclideanEngine::getStep(int stepIndex) const
{
    if (stepIndex < 0 || stepIndex >= steps)
        return false;
    return pattern[stepIndex];
}

float EuclideanEngine::getVelocity(int stepIndex) const
{
    if (stepIndex < 0 || stepIndex >= steps)
        return 0.8f;
    return velocities[stepIndex];
}

void EuclideanEngine::rotate(int amount)
{
    if (steps <= 0)
        return;
    rotation = (rotation + amount) % steps;
    if (rotation < 0)
        rotation += steps;
    regeneratePattern();
}

void EuclideanEngine::randomize(float density)
{
    pulses = static_cast<int>(steps * juce::jlimit(0.0f, 1.0f, density));
    rotation = (steps > 0) ? random.nextInt(steps) : 0;
    regeneratePattern();
}

void EuclideanEngine::regeneratePattern()
{
    generateEuclideanPattern();
}

void EuclideanEngine::generateEuclideanPattern()
{
    // Even distribution (Björklund-equivalent remainder form). Stack-only.
    if (pulses <= 0 || steps <= 0)
    {
        for (int i = 0; i < steps; ++i)
            pattern[i] = false;
    }
    else if (pulses >= steps)
    {
        for (int i = 0; i < steps; ++i)
            pattern[i] = true;
    }
    else
    {
        for (int i = 0; i < steps; ++i)
            pattern[i] = (((i * pulses) % steps) < pulses);
    }

    // Apply rotation via stack temp
    if (rotation != 0 && steps > 0)
    {
        bool rotated[kMaxSteps];
        for (int i = 0; i < steps; ++i)
            rotated[i] = pattern[(i + rotation) % steps];
        for (int i = 0; i < steps; ++i)
            pattern[i] = rotated[i];
    }

    for (int i = 0; i < steps; ++i)
    {
        if (pattern[i])
        {
            if (accentCount > 0)
                velocities[i] = accentPattern[i % accentCount];
            else
                velocities[i] = 0.8f;
        }
        else
        {
            velocities[i] = 0.0f;
        }
    }
}
