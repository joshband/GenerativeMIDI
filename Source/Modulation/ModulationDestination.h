/*
  ==============================================================================
    ModulationDestination.h

    Fixed destination IDs for Modulation v2 (no string lookups on audio thread).
    The router applies these by enum. Free slots use ModulationRouter::SlotDest.

  ==============================================================================
*/

#pragma once

#include <cstdint>

enum class ModulationDestination : uint8_t
{
    Velocity = 0,
    NoteDensity,
    GateLength,
    Pitch,
    Cc,
    Bend,
    Count
};

inline constexpr int kModulationDestinationCount =
    static_cast<int>(ModulationDestination::Count);
