#pragma once

#include <vector>
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
    void setMomentum(float momentum) { this->momentum = std::clamp(momentum, 0.0f, 1.0f); }
    void setOctaves(int octaves) { this->octaves = std::clamp(octaves, 1, 8); }
    void setTimeScale(float scale) { timeScale = std::clamp(scale, 0.01f, 10.0f); }

    // Lorenz attractor parameters
    void setSigma(float sigma) { this->sigma = sigma; }
    void setRho(float rho) { this->rho = rho; }
    void setBeta(float beta) { this->beta = beta; }

    // Generation
    void reset();
    void advance(float deltaTime);

    bool shouldTriggerNote() const;
    int getCurrentPitch(int minPitch, int maxPitch) const;
    float getCurrentVelocity(float minVel, float maxVel) const;

    // State access
    float getCurrentValue() const { return currentValue; }
    float getSecondaryValue() const { return secondaryValue; }

private:
    // Generator implementations
    void updateBrownianMotion(float deltaTime);
    void updatePerlinNoise(float deltaTime);
    void updateDrunkWalk(float deltaTime);
    void updateLorenzAttractor(float deltaTime);

    // Perlin noise helpers
    float perlinNoise(float x, float y) const;
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
    float noiseTime;
    int permutation[512];     // Permutation table for Perlin noise

    // Drunk walk state
    float drunkPosition;

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
    float dt;                 // Integration step size

    // Random number generation
    mutable std::mt19937 rng;
    mutable std::uniform_real_distribution<float> uniform01;
    mutable std::normal_distribution<float> normalDist;
};
