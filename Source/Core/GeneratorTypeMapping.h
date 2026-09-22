/*
  ==============================================================================
    GeneratorTypeMapping.h

    Shared mapping: APVTS generator index ↔ engine enums (matches editor UI).

  ==============================================================================
*/

#pragma once

#include "AlgorithmicEngine.h"
#include "StochasticEngine.h"

namespace GeneratorTypeMapping
{
    constexpr int kEuclidean = 0;
    constexpr int kMarkov = 1;
    constexpr int kLSystem = 2;
    constexpr int kCellular = 3;
    constexpr int kProbabilistic = 4;
    constexpr int kBrownian = 5;
    constexpr int kPerlin = 6;
    constexpr int kDrunkWalk = 7;
    constexpr int kLorenz = 8;
    constexpr int kCount = 9;

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
}
