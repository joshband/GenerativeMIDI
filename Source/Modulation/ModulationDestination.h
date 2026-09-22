/*
  ==============================================================================
    ModulationDestination.h

    Fixed destination IDs for Modulation v2 (no string lookups on audio thread).
    MVP only wires Velocity; other values are stubs for the future router.

  ==============================================================================
*/

#pragma once

#include <cstdint>

enum class ModulationDestination : uint8_t
{
    Velocity = 0,
    NoteDensity,   // stub — not applied in MVP
    GateLength,    // stub
    Count
};

inline constexpr int kModulationDestinationCount =
    static_cast<int>(ModulationDestination::Count);
