/*
  ==============================================================================
    GeneratorTypeMapping.h

    Shared mapping: APVTS generator index ↔ engine enums (matches editor UI).

  ==============================================================================
*/

#pragma once

#include "AlgorithmicEngine.h"
#include "StochasticEngine.h"
#include <cmath>

namespace GeneratorTypeMapping
{
    constexpr int kEuclidean = 0;
    constexpr int kPolyrhythm = 1;
    constexpr int kMarkov = 2;
    constexpr int kLSystem = 3;
    constexpr int kCellular = 4;
    constexpr int kProbabilistic = 5;
    constexpr int kBrownian = 6;
    constexpr int kPerlin = 7;
    constexpr int kDrunkWalk = 8;
    constexpr int kLorenz = 9;
    constexpr int kCount = 10;

    inline bool isPolyrhythm(int index)
    {
        return index == kPolyrhythm;
    }

    inline bool isAlgorithmic(int index)
    {
        return index >= kMarkov && index <= kProbabilistic;
    }

    inline bool isStochastic(int index)
    {
        return index >= kBrownian && index <= kLorenz;
    }

    inline AlgorithmicEngine::GeneratorType toAlgorithmic(int index)
    {
        switch (index)
        {
            case kMarkov: return AlgorithmicEngine::Markov;
            case kLSystem: return AlgorithmicEngine::LSystem;
            case kCellular: return AlgorithmicEngine::CellularAutomatonType;
            case kProbabilistic: return AlgorithmicEngine::Probabilistic;
            default: return AlgorithmicEngine::Probabilistic;
        }
    }

    inline StochasticEngine::GeneratorType toStochastic(int index)
    {
        switch (index)
        {
            case kBrownian: return StochasticEngine::GeneratorType::BrownianMotion;
            case kPerlin: return StochasticEngine::GeneratorType::PerlinNoise;
            case kDrunkWalk: return StochasticEngine::GeneratorType::DrunkWalk;
            case kLorenz: return StochasticEngine::GeneratorType::LorenzAttractor;
            default: return StochasticEngine::GeneratorType::BrownianMotion;
        }
    }

    /**
     * Preset / session schema:
     * - "1.1" = 10-generator layout (Polyrhythm at index 1)
     * - "1.2" = + PolyrhythmLayers ValueTree child (patterns/offsets)
     */
    constexpr const char* kPresetSchemaVersion = "1.2";

    /**
     * Map a pre-Polyrhythm (9-gen) generatorType index to the 10-gen layout.
     * Old: 0 Euclidean, 1–4 algo, 5–8 stochastic.
     * New: 0 Euclidean, 1 Polyrhythm, 2–5 algo, 6–9 stochastic.
     */
    inline int migrateGeneratorTypeIndex(int oldIndex) noexcept
    {
        if (oldIndex >= 1 && oldIndex <= 8)
            return oldIndex + 1;
        return oldIndex;
    }

    /**
     * If schema is older than 1.1 (or missing), shift generatorType PARAM values 1..8 → 2..9.
     * Expects APVTS-style children: PARAM with property "id" == "generatorType".
     * Schemas 1.1+ (including 1.2 layer persistence) are left unchanged.
     */
    inline void migrateApvtsStateIfNeeded(juce::ValueTree& state, const juce::String& schemaVersion)
    {
        const bool needsMigration = schemaVersion.isEmpty()
            || schemaVersion.compareNatural("1.1") < 0;

        if (!needsMigration)
            return;

        for (int i = 0; i < state.getNumChildren(); ++i)
        {
            auto child = state.getChild(i);
            if (child.hasType("PARAM") && child.getProperty("id").toString() == "generatorType")
            {
                const float raw = static_cast<float>(child.getProperty("value"));
                // Denormalised choice index (JUCE 8 APVTS) or legacy normalised — prefer denorm int.
                int index = static_cast<int>(std::lround(static_cast<double>(raw)));
                if (raw >= 0.0f && raw <= 1.0f && raw != static_cast<float>(index))
                {
                    // Normalised choice: index ≈ raw * (numChoices-1) for old 9-choice list
                    index = static_cast<int>(std::lround(static_cast<double>(raw) * 8.0));
                }

                const int migrated = migrateGeneratorTypeIndex(index);
                child.setProperty("value", migrated, nullptr);
                break;
            }
        }
    }
}
