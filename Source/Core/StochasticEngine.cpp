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
      rng(std::random_device{}()),
      uniform01(0.0f, 1.0f),
      normalDist(0.0f, 1.0f)
{
    buildPermutation();

    reset();
}

void StochasticEngine::buildPermutation()
{
    for (int i = 0; i < 256; ++i)
        permutation[i] = i;

    for (int i = 255; i > 0; --i)
    {
        int j = static_cast<int>(rng() % static_cast<unsigned>(i + 1));
        std::swap(permutation[i], permutation[j]);
    }

    // Duplicate permutation table for wrapping
    for (int i = 0; i < 256; ++i)
        permutation[256 + i] = permutation[i];
}

void StochasticEngine::setSeed(std::uint32_t seed)
{
    seeded = true;
    seedValue = seed;
    reset();
}

void StochasticEngine::reset()
{
    if (seeded)
    {
        rng.seed(seedValue);
        uniform01.reset();
        normalDist.reset();
        buildPermutation();
    }

    currentValue = 0.5f;
    secondaryValue = 0.5f;
    tertiaryValue = 0.5f;
    velocity = 0.0f;
    acceleration = 0.0f;
    noiseTime = 0.0f;
    drunkPosition = 0.5f;
    drunkElapsed = 0.0f;

    // Lorenz attractor: start off the origin fixed point, inside the attractor box
    lorenzX = lorenzY = lorenzZ = 1.0;
    lorenzPending = 0.0;
    currentValue = 0.525f;   // (x + 20) / 40
    secondaryValue = 0.517f; // (y + 30) / 60
    tertiaryValue = 0.02f;   // z / 50
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
    noiseTime = std::fmod(noiseTime + static_cast<double>(deltaTime) * timeScale * freq, kNoisePeriod);
    if (noiseTime < 0.0)
        noiseTime += kNoisePeriod;

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
            value += perlinNoise(static_cast<float>(std::fmod(noiseTime * frequency + timeOffset, kNoisePeriod)), i * 100.0f) * amplitude;
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
    drunkElapsed += deltaTime;

    float stepInterval = 1.0f / (timeScale * 10.0f);  // Steps per second based on time scale

    if (drunkElapsed >= stepInterval)
    {
        drunkElapsed = 0.0f;

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
    //
    // stepSize and timeScale set how fast simulated time runs (the integration
    // SPEED), never the integrator step. Integration uses RK4 with a bounded
    // sub-step, so the orbit is stable and independent of the host call rate.
    constexpr double kBaseRate = 8.0;     // simulated time units per second at default settings
    constexpr double kMinH = 0.005;       // RK4 sub-step
    constexpr double kMaxH = 0.02;        // still stable for the classic parameters
    constexpr int kMaxSteps = 100;        // bound on audio-thread work per call
    constexpr double kMaxPending = kMaxH * kMaxSteps;

    const double speed = kBaseRate * std::sqrt(static_cast<double>(timeScale))
                                   * std::sqrt(static_cast<double>(stepSize) / 0.1);
    lorenzPending += std::max(0.0, static_cast<double>(deltaTime)) * speed;
    lorenzPending = std::min(lorenzPending, kMaxPending); // drop time we cannot afford

    const double h = std::clamp(lorenzPending / kMaxSteps, kMinH, kMaxH);
    const double sg = sigma, rh = rho, bt = beta;

    auto f = [sg, rh, bt](const double s[3], double d[3])
    {
        d[0] = sg * (s[1] - s[0]);
        d[1] = s[0] * (rh - s[2]) - s[1];
        d[2] = s[0] * s[1] - bt * s[2];
    };

    double s[3] = { lorenzX, lorenzY, lorenzZ };
    for (int i = 0; i < kMaxSteps && lorenzPending >= h; ++i)
    {
        double k1[3], k2[3], k3[3], k4[3], t[3];
        f(s, k1);
        for (int j = 0; j < 3; ++j) t[j] = s[j] + 0.5 * h * k1[j];
        f(t, k2);
        for (int j = 0; j < 3; ++j) t[j] = s[j] + 0.5 * h * k2[j];
        f(t, k3);
        for (int j = 0; j < 3; ++j) t[j] = s[j] + h * k3[j];
        f(t, k4);
        for (int j = 0; j < 3; ++j) s[j] += h / 6.0 * (k1[j] + 2.0 * k2[j] + 2.0 * k3[j] + k4[j]);
        lorenzPending -= h;
    }

    // Custom sigma/rho/beta could diverge; re-seed rather than emit NaN.
    if (! (std::isfinite(s[0]) && std::isfinite(s[1]) && std::isfinite(s[2]))
        || std::abs(s[0]) > 1.0e3 || std::abs(s[1]) > 1.0e3 || std::abs(s[2]) > 1.0e3)
    {
        s[0] = s[1] = s[2] = 1.0;
    }
    lorenzX = s[0];
    lorenzY = s[1];
    lorenzZ = s[2];

    // Fixed attractor-range mapping (classic attractor: x ~ +-20, y ~ +-30, z ~ 0..50).
    float x01 = static_cast<float>((lorenzX + 20.0) / 40.0);
    float y01 = static_cast<float>((lorenzY + 30.0) / 60.0);
    float z01 = static_cast<float>(lorenzZ / 50.0);

    // Momentum damps the output toward its midpoint. 0.9 and above stay undamped.
    // Applied to the output only, so it never feeds back into the orbit.
    const float leak = std::max(0.0f, (0.9f - momentum) * 0.55f);
    x01 += (0.5f - x01) * leak;
    y01 += (0.5f - y01) * leak;
    z01 += (0.45f - z01) * leak;

    currentValue = std::clamp(x01, 0.0f, 1.0f);
    secondaryValue = std::clamp(y01, 0.0f, 1.0f);
    tertiaryValue = std::clamp(z01, 0.0f, 1.0f);
}
