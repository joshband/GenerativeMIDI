#pragma once

#include <vector>
#include <cstdint>
#include <random>
#include <cmath>
#include <algorithm>

/**
 * @class StochasticEngine
 * @brief Stochastic and chaotic pattern generators for experimental generative music
 *
 * Generates patterns using:
 * - Brownian Motion (random walk with momentum)
 * - Perlin Noise (smooth, natural randomness)
 * - Drunk Walk (discrete random walk)
 * - Lorenz Attractor (deterministic chaos)
 */
class StochasticEngine
{
public:
    enum class GeneratorType
    {
        BrownianMotion,
        PerlinNoise,
        DrunkWalk,
        LorenzAttractor
    };

    StochasticEngine();
    ~StochasticEngine() = default;

    // Configuration
    void setGeneratorType(GeneratorType type) { generatorType = type; }
    void setDensity(float density) { noteDensity = std::clamp(density, 0.0f, 1.0f); }
    void setStepSize(float size) { stepSize = std::clamp(size, 0.01f, 1.0f); }
    void setMomentum(float value) { momentum = std::clamp(value, 0.0f, 1.0f); }
    void setOctaves(int count) { octaves = std::clamp(count, 1, 8); }
    void setTimeScale(float scale) { timeScale = std::clamp(scale, 0.01f, 10.0f); }

    // Lorenz attractor parameters
    void setSigma(float value) { sigma = value; }
    void setRho(float value) { rho = value; }
    void setBeta(float value) { beta = value; }

    /** Fix the RNG seed. reset() (called here too) then restarts every generator and
        the Perlin permutation table from this seed, so identical inputs give identical
        output. Without a call, each engine keeps a random seed. */
    void setSeed(std::uint32_t seed);

    // Generation
    void reset();
    void advance(float deltaTime);

    bool shouldTriggerNote() const;
    int getCurrentPitch(int minPitch, int maxPitch) const;
    float getCurrentVelocity(float minVel, float maxVel) const;

    // State access
    float getCurrentValue() const { return currentValue; }
    float getSecondaryValue() const { return secondaryValue; }
    float getTertiaryValue() const { return tertiaryValue; }

private:
    // Generator implementations
    void updateBrownianMotion(float deltaTime);
    void updatePerlinNoise(float deltaTime);
    void updateDrunkWalk(float deltaTime);
    void updateLorenzAttractor(float deltaTime);

    // Perlin noise helpers
    float perlinNoise(float x, float y) const;
    void buildPermutation();
    float fade(float t) const { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }
    float lerp(float a, float b, float t) const { return a + t * (b - a); }
    float grad(int hash, float x, float y) const;

    // State
    GeneratorType generatorType;
    float currentValue;       // Main output value (0.0-1.0)
    float secondaryValue;     // Secondary value for 2D/3D systems
    float tertiaryValue;      // Tertiary value for 3D systems (Lorenz)

    // Brownian motion state
    float velocity;
    float acceleration;

    // Perlin noise state
    double noiseTime;         // wrapped at kNoisePeriod so precision never degrades
    static constexpr double kNoisePeriod = 256.0; // permutation lattice period
    int permutation[512];     // Permutation table for Perlin noise

    // Drunk walk state
    float drunkPosition;
    float drunkElapsed = 0.0f; // seconds since the last drunk-walk step

    // Parameters
    float noteDensity;
    float stepSize;
    float momentum;
    int octaves;              // For Perlin noise multi-octave
    float timeScale;          // Time scaling factor

    // Lorenz attractor parameters
    float sigma;              // Prandtl number (default: 10.0)
    float rho;                // Rayleigh number (default: 28.0)
    float beta;               // Geometric factor (default: 8.0/3.0)

    // Lorenz state in attractor coordinates (double: RK4 sub-stepped)
    double lorenzX = 1.0, lorenzY = 1.0, lorenzZ = 1.0;
    double lorenzPending = 0.0; // simulated time not yet integrated

    // Random number generation
    bool seeded = false;
    std::uint32_t seedValue = 0;
    mutable std::mt19937 rng;
    mutable std::uniform_real_distribution<float> uniform01;
    mutable std::normal_distribution<float> normalDist;
};
