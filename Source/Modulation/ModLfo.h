/*
  ==============================================================================
    ModLfo.h

    Realtime-safe bipolar sine LFO for Modulation v2 MVP.
    Audio thread: advance() + getBipolar(). UI may read getBipolar() via atomic.

  ==============================================================================
*/

#pragma once

#include <atomic>
#include <cmath>
#include <juce_core/juce_core.h>

/**
 * Fixed sine LFO. Output is bipolar in [-1, +1].
 * No heap, no locks, no virtual calls.
 */
class ModLfo
{
public:
    void reset() noexcept
    {
        phase = 0.0;
        currentValue.store(0.0f, std::memory_order_relaxed);
    }

    void setRateHz(float hz) noexcept
    {
        rateHz = juce::jlimit(0.01f, 40.0f, hz);
    }

    float getRateHz() const noexcept { return rateHz; }

    /** Advance phase by wall-clock seconds. Call from the audio thread. */
    void advance(double deltaSeconds) noexcept
    {
        if (deltaSeconds <= 0.0)
            return;

        phase += static_cast<double>(rateHz) * deltaSeconds;
        phase -= std::floor(phase);

        const float value = static_cast<float>(
            std::sin(phase * juce::MathConstants<double>::twoPi));
        currentValue.store(value, std::memory_order_relaxed);
    }

    /** Current bipolar value in [-1, +1]. */
    float getBipolar() const noexcept
    {
        return currentValue.load(std::memory_order_relaxed);
    }

    /**
     * Apply bipolar modulation to a unipolar base (e.g. velocity).
     * result = clamp(base + bipolar * depth, 0, 1)
     */
    static float applyToUnipolar(float base, float bipolar, float depth) noexcept
    {
        return juce::jlimit(0.0f, 1.0f, base + bipolar * depth);
    }

private:
    double phase = 0.0;
    float rateHz = 1.0f;
    std::atomic<float> currentValue { 0.0f };
};
