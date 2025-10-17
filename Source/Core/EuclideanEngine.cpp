/*
  ==============================================================================
    EuclideanEngine.cpp

    Implementation of Björklund's algorithm for Euclidean rhythms

  ==============================================================================
*/

#include "EuclideanEngine.h"

EuclideanEngine::EuclideanEngine()
{
    regeneratePattern();
}

void EuclideanEngine::setSteps(int numSteps)
{
    steps = juce::jlimit(1, 64, numSteps);
    regeneratePattern();
}

void EuclideanEngine::setPulses(int numPulses)
{
    pulses = juce::jlimit(0, steps, numPulses);
    regeneratePattern();
}

void EuclideanEngine::setRotation(int rot)
{
    rotation = rot % steps;
    regeneratePattern();
}

void EuclideanEngine::setAccentPattern(const std::vector<float>& accents)
{
    accentPattern = accents;
    regeneratePattern();
}

bool EuclideanEngine::getStep(int stepIndex) const
{
    if (stepIndex < 0 || stepIndex >= pattern.size())
        return false;
    return pattern[stepIndex];
}

float EuclideanEngine::getVelocity(int stepIndex) const
{
    if (stepIndex < 0 || stepIndex >= velocities.size())
        return 0.8f;
    return velocities[stepIndex];
}

void EuclideanEngine::rotate(int amount)
{
    rotation = (rotation + amount) % steps;
    regeneratePattern();
}

void EuclideanEngine::randomize(float density)
{
    pulses = static_cast<int>(steps * juce::jlimit(0.0f, 1.0f, density));
    rotation = random.nextInt(steps);
    regeneratePattern();
}

void EuclideanEngine::regeneratePattern()
{
    generateEuclideanPattern();
}

void EuclideanEngine::generateEuclideanPattern()
{
    // Generate base Euclidean pattern using Björklund's algorithm
    pattern = bjorklund(pulses, steps);

    // Apply rotation
    if (rotation != 0)
    {
        std::vector<bool> rotated(steps);
        for (int i = 0; i < steps; ++i)
        {
            rotated[i] = pattern[(i + rotation) % steps];
        }
        pattern = rotated;
    }

    // Generate velocities with accent pattern
    velocities.resize(steps);
    for (int i = 0; i < steps; ++i)
    {
        if (pattern[i])
        {
            if (!accentPattern.empty())
            {
                int accentIndex = i % accentPattern.size();
                velocities[i] = accentPattern[accentIndex];
            }
            else
            {
                velocities[i] = 0.8f; // Default velocity
            }
        }
        else
        {
            velocities[i] = 0.0f;
        }
    }
}

std::vector<bool> EuclideanEngine::bjorklund(int pulses, int steps)
{
    if (pulses < 0 || steps <= 0 || pulses > steps)
        return std::vector<bool>(steps, false);

    if (pulses == 0)
        return std::vector<bool>(steps, false);

    if (pulses == steps)
        return std::vector<bool>(steps, true);

    // Björklund's algorithm using groups
    std::vector<std::vector<bool>> groups;

    // Initialize with 'pulses' groups containing [1]
    for (int i = 0; i < pulses; ++i)
        groups.push_back({true});

    // And 'steps - pulses' groups containing [0]
    for (int i = 0; i < steps - pulses; ++i)
        groups.push_back({false});

    // Distribute groups
    int splitIndex = pulses;

    while (splitIndex > 0 && splitIndex < groups.size())
    {
        int numToDistribute = std::min(splitIndex, static_cast<int>(groups.size() - splitIndex));

        if (numToDistribute <= 0)
            break;

        // Append second half groups to first half
        for (int i = 0; i < numToDistribute; ++i)
        {
            auto& target = groups[i];
            auto& source = groups[splitIndex + i];
            target.insert(target.end(), source.begin(), source.end());
        }

        // Remove appended groups
        groups.erase(groups.begin() + splitIndex, groups.begin() + splitIndex + numToDistribute);

        // Update split index
        splitIndex = numToDistribute;
    }

    // Flatten result
    std::vector<bool> result;
    for (const auto& group : groups)
    {
        for (bool val : group)
            result.push_back(val);
    }

    return result;
}
