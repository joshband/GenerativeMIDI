/*
  ==============================================================================
    ModulationRouter.h

    Fixed-slot modulation for Modulation v2. Two sources, four slots.
    Slots 0 and 1 are the existing LFO velocity and density depths.
    Slots 2 and 3 are optional routes. No heap, strings, or virtual calls.

  ==============================================================================
*/

#pragma once

#include "ModLfo.h"

#include <atomic>
#include <cstdint>

/**
 * Sample-and-hold. A new bipolar value is drawn from a local LCG when phase wraps.
 */
class ModSampleHold
{
public:
    void reset() noexcept
    {
        phase = 0.0;
        state = 0xA3C59AC3u;
        current = 0.0f;
        heldValue = 0.0f;
        steppedThisBlock = false;
        currentValue.store(0.0f, std::memory_order_relaxed);
    }

    void setRateHz(float hz) noexcept
    {
        rateHz = juce::jlimit(0.05f, 20.0f, hz);
    }

    float getRateHz() const noexcept { return rateHz; }

    void advance(double deltaSeconds) noexcept
    {
        if (deltaSeconds <= 0.0)
            return;

        heldValue = current;
        steppedThisBlock = false;
        phase += static_cast<double>(rateHz) * deltaSeconds;

        int guard = 0;
        while (phase >= 1.0 && guard < 8)
        {
            phase -= 1.0;
            current = nextBipolar();
            steppedThisBlock = true;
            ++guard;
        }

        if (phase >= 1.0)
            phase -= std::floor(phase);

        currentValue.store(current, std::memory_order_relaxed);
    }

    float getBipolar() const noexcept
    {
        return currentValue.load(std::memory_order_relaxed);
    }

    /** Value at a lookback into the block that was just advanced. */
    float peekBipolar(double deltaSeconds) const noexcept
    {
        if (steppedThisBlock && deltaSeconds < 0.0)
            return heldValue;
        return current;
    }

private:
    float nextBipolar() noexcept
    {
        state = state * 1664525u + 1013904223u;
        const float unit = static_cast<float>(state >> 8) * (1.0f / 16777216.0f);
        return unit * 2.0f - 1.0f;
    }

    double phase = 0.0;
    float rateHz = 1.0f;
    uint32_t state = 0xA3C59AC3u;
    float current = 0.0f;
    float heldValue = 0.0f;
    bool steppedThisBlock = false;
    std::atomic<float> currentValue { 0.0f };
};

namespace ModulationRouter
{
enum class Source : uint8_t
{
    Lfo = 0,
    SampleHold = 1
};

/** Choice order for the two free slots. Off is not an applied destination. */
enum class SlotDest : uint8_t
{
    Off = 0,
    Gate = 1,
    Pitch = 2,
    Cc = 3,
    Bend = 4
};

struct Slot
{
    int source = 0;
    int dest = 0;
    float amount = 0.0f;
};

struct Frame
{
    bool lfoEnabled = false;
    float lfo = 0.0f;
    float sampleHold = 0.0f;
    float velocityAmount = 0.0f;
    float densityAmount = 0.0f;
    Slot extras[2] {};
};

struct Mix
{
    float velocityDelta = 0.0f;
    float densityDelta = 0.0f;
    float gateDelta = 0.0f;
    float pitchSemitones = 0.0f;
    float ccDelta = 0.0f;
    float bendDelta = 0.0f;
    bool ccRouted = false;
    bool bendRouted = false;
};

inline float sourceValue(const Frame& frame, int source) noexcept
{
    if (source == static_cast<int>(Source::SampleHold))
        return frame.sampleHold;
    return frame.lfoEnabled ? frame.lfo : 0.0f;
}

inline Mix evaluate(const Frame& frame) noexcept
{
    Mix mix;
    if (frame.lfoEnabled)
    {
        mix.velocityDelta = frame.lfo * frame.velocityAmount;
        mix.densityDelta = frame.lfo * frame.densityAmount;
    }

    for (const auto& slot : frame.extras)
    {
        if (slot.dest == static_cast<int>(SlotDest::Off) || slot.amount <= 0.0f)
            continue;

        const float delta = sourceValue(frame, slot.source) * slot.amount;
        switch (slot.dest)
        {
            case static_cast<int>(SlotDest::Gate):
                mix.gateDelta += delta;
                break;
            case static_cast<int>(SlotDest::Pitch):
                mix.pitchSemitones += delta * 12.0f;
                break;
            case static_cast<int>(SlotDest::Cc):
                mix.ccDelta += delta;
                mix.ccRouted = true;
                break;
            case static_cast<int>(SlotDest::Bend):
                mix.bendDelta += delta;
                mix.bendRouted = true;
                break;
            default:
                break;
        }
    }

    return mix;
}

inline float applyAdditive(float base, float delta, float lo, float hi) noexcept
{
    return juce::jlimit(lo, hi, base + delta);
}
} // namespace ModulationRouter
