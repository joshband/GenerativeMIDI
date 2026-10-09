#include "StochasticEngine.h"

StochasticEngine::StochasticEngine()
    : generatorType(GeneratorType::BrownianMotion),
      currentValue(0.5f),
      secondaryValue(0.5f),
      tertiaryValue(0.5f),
      velocity(0.0f),
      acceleration(0.0f),
      noiseTime(0.0f),
      drunkPosition(0.5f),
      noteDensity(0.5f),
      stepSize(0.1f),
      momentum(0.9f),
      octaves(4),
      timeScale(1.0f),
      sigma(10.0f),
      rho(28.0f),
      beta(8.0f / 3.0f),
      dt(0.01f),
      rng(std::random_device{}()),
      uniform01(0.0f, 1.0f),
      normalDist(0.0f, 1.0f)
{
    // Initialize Perlin noise permutation table
    for (int i = 0; i < 256; ++i)
        permutation[i] = i;

    // Shuffle permutation table
    for (int i = 255; i > 0; --i)
    {
        int j = rng() % (i + 1);
        std::swap(permutation[i], permutation[j]);
    }

    // Duplicate permutation table for wrapping
    for (int i = 0; i < 256; ++i)
        permutation[256 + i] = permutation[i];

    reset();
}

void StochasticEngine::reset()
{
    currentValue = 0.5f;
    secondaryValue = 0.5f;
    tertiaryValue = 0.5f;
    velocity = 0.0f;
    acceleration = 0.0f;
    noiseTime = 0.0f;
    drunkPosition = 0.5f;

    // Lorenz attractor: start near the origin with slight offset
    currentValue = 0.1f;
    secondaryValue = 0.0f;
    tertiaryValue = 0.0f;
}

void StochasticEngine::advance(float deltaTime)
{
    switch (generatorType)
    {
        case GeneratorType::BrownianMotion:
            updateBrownianMotion(deltaTime);
            break;

        case GeneratorType::PerlinNoise:
            updatePerlinNoise(deltaTime);
            break;

        case GeneratorType::DrunkWalk:
            updateDrunkWalk(deltaTime);
            break;

        case GeneratorType::LorenzAttractor:
            updateLorenzAttractor(deltaTime);
            break;
    }
}

bool StochasticEngine::shouldTriggerNote() const
{
    // Probabilistic triggering based on density
    return uniform01(rng) < noteDensity;
}

int StochasticEngine::getCurrentPitch(int minPitch, int maxPitch) const
{
    // Map current value (0.0-1.0) to pitch range
    float normalizedPitch = std::clamp(currentValue, 0.0f, 1.0f);
    int pitch = minPitch + static_cast<int>(normalizedPitch * (maxPitch - minPitch));
    return std::clamp(pitch, 0, 127);
}

float StochasticEngine::getCurrentVelocity(float minVel, float maxVel) const
{
    // Use secondary value for velocity variation
    float normalizedVel = std::clamp(secondaryValue, 0.0f, 1.0f);
    return minVel + normalizedVel * (maxVel - minVel);
}

// ============================================================================
// Brownian Motion Implementation
// ============================================================================

void StochasticEngine::updateBrownianMotion(float deltaTime)
{
    // Add random acceleration
    acceleration = normalDist(rng) * stepSize;

    // Update velocity with momentum (friction)
    velocity = velocity * momentum + acceleration;

    // Limit velocity to prevent runaway
    velocity = std::clamp(velocity, -0.5f, 0.5f);

    // Update position
    currentValue += velocity * deltaTime * timeScale;

    // Wrap around boundaries
    if (currentValue > 1.0f)
    {
        currentValue = 1.0f;
        velocity *= -0.5f;  // Bounce with energy loss
    }
    else if (currentValue < 0.0f)
    {
        currentValue = 0.0f;
        velocity *= -0.5f;
    }

    // Secondary value follows with slight delay
    secondaryValue += (currentValue - secondaryValue) * 0.1f;
}

// ============================================================================
// Perlin Noise Implementation
// ============================================================================

void StochasticEngine::updatePerlinNoise(float deltaTime)
{
    // Step size is terrain smoothness. 0.1 keeps the original sample rate.
    const float freq = std::pow(0.1f / std::max(stepSize, 0.01f), 0.85f);
    noiseTime += deltaTime * timeScale * freq;

    // Momentum is octave persistence. 0.9 keeps the original 0.5 blend.
    const float persistence = std::clamp(0.5f + (momentum - 0.9f) * 1.5f, 0.05f, 0.9f);

    auto accumulate = [this, persistence](float timeOffset)
    {
        float amplitude = 1.0f;
        float frequency = 1.0f;
        float value = 0.0f;
        float maxValue = 0.0f;

        for (int i = 0; i < octaves; ++i)
        {
            value += perlinNoise(noiseTime * frequency + timeOffset, i * 100.0f) * amplitude;
            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= 2.0f;
        }

        return (value / std::max(maxValue, 0.0001f) + 1.0f) * 0.5f;
    };

    currentValue = std::clamp(accumulate(0.0f), 0.0f, 1.0f);
    secondaryValue = std::clamp(accumulate(1000.0f), 0.0f, 1.0f);
}

float StochasticEngine::perlinNoise(float x, float y) const
{
    // Find grid cell coordinates
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;

    // Relative position within cell
    x -= std::floor(x);
    y -= std::floor(y);

    // Fade curves
    float u = fade(x);
    float v = fade(y);

    // Hash coordinates of 4 cell corners
    int aa = permutation[permutation[X] + Y];
    int ab = permutation[permutation[X] + Y + 1];
    int ba = permutation[permutation[X + 1] + Y];
    int bb = permutation[permutation[X + 1] + Y + 1];

    // Blend results from 4 corners
    float res = lerp(
        lerp(grad(aa, x, y), grad(ba, x - 1, y), u),
        lerp(grad(ab, x, y - 1), grad(bb, x - 1, y - 1), u),
        v
    );

    return res;
}

float StochasticEngine::grad(int hash, float x, float y) const
{
    // Convert low 3 bits of hash into 8 gradient directions
    int h = hash & 7;
    float u = (h < 4) ? x : y;
    float v = (h < 4) ? y : x;

    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

// ============================================================================
// Drunk Walk Implementation
// ============================================================================

void StochasticEngine::updateDrunkWalk(float deltaTime)
{
    // Only take steps at discrete intervals
    static float timeSinceLastStep = 0.0f;
    timeSinceLastStep += deltaTime;

    float stepInterval = 1.0f / (timeScale * 10.0f);  // Steps per second based on time scale

    if (timeSinceLastStep >= stepInterval)
    {
        timeSinceLastStep = 0.0f;

        // Random walk with variable step size
        float step = (uniform01(rng) - 0.5f) * 2.0f * stepSize;
        drunkPosition += step;

        // Bounce off boundaries
        if (drunkPosition > 1.0f)
        {
            drunkPosition = 2.0f - drunkPosition;
            if (drunkPosition < 0.0f) drunkPosition = 0.0f;
        }
        else if (drunkPosition < 0.0f)
        {
            drunkPosition = -drunkPosition;
            if (drunkPosition > 1.0f) drunkPosition = 1.0f;
        }
    }

    // Momentum is how smoothly the pitch glides onto each step.
    // 0.9 matches the previous fixed 0.3 follow.
    const float follow = std::clamp((1.0f - momentum) * 3.0f, 0.02f, 1.0f);
    currentValue += (drunkPosition - currentValue) * follow;

    // Secondary value has different step pattern
    if (uniform01(rng) < 0.3f)  // Less frequent updates
    {
        secondaryValue = uniform01(rng);
    }
}

// ============================================================================
// Lorenz Attractor Implementation
// ============================================================================

void StochasticEngine::updateLorenzAttractor(float deltaTime)
{
    // Lorenz system differential equations:
    // dx/dt = sigma * (y - x)
    // dy/dt = x * (rho - z) - y
    // dz/dt = x * y - beta * z

    // Step size is the Euler increment. 0.1 keeps the original dt.
    const float h = dt * (stepSize / 0.1f);
    int steps = static_cast<int>(deltaTime / h);
    steps = std::max(1, std::min(steps, 100));

    for (int i = 0; i < steps; ++i)
    {
        float x = (currentValue - 0.5f) * 60.0f;      // Scale to attractor space
        float y = (secondaryValue - 0.5f) * 60.0f;
        float z = (tertiaryValue * 60.0f);

        float dx = sigma * (y - x);
        float dy = x * (rho - z) - y;
        float dz = x * y - beta * z;

        x += dx * h * timeScale;
        y += dy * h * timeScale;
        z += dz * h * timeScale;

        // Scale back to 0.0-1.0 range
        currentValue = x / 60.0f + 0.5f;
        secondaryValue = y / 60.0f + 0.5f;
        tertiaryValue = z / 60.0f;

        // Keep values in reasonable range
        currentValue = std::clamp(currentValue, 0.0f, 1.0f);
        secondaryValue = std::clamp(secondaryValue, 0.0f, 1.0f);
        tertiaryValue = std::clamp(tertiaryValue, 0.0f, 1.0f);
    }

    // Momentum damps the orbit toward its midpoint. 0.9 and above stay undamped.
    const float leak = std::max(0.0f, (0.9f - momentum) * 0.55f);
    if (leak > 0.0f)
    {
        currentValue += (0.5f - currentValue) * leak;
        secondaryValue += (0.5f - secondaryValue) * leak;
        tertiaryValue += (0.45f - tertiaryValue) * leak;
    }
}
