/*
  ==============================================================================
    TimeSignature.h
    Shared time-signature helpers.
  ==============================================================================
*/

#pragma once

#include <algorithm>
#include <cstdlib>

namespace TimeSignature
{
    /** A musical denominator is a power of two (1, 2, 4, 8, 16, 32). The automatable
        parameter is a plain integer 1..16, so 3, 5, 6, 7, 9..15 can arrive from a host or an
        old preset; snap those to the nearest power of two (ties go to the smaller one) instead
        of computing bar lengths from a note value that does not exist. */
    inline int sanitizeDenominator(int denominator) noexcept
    {
        const int clamped = std::clamp(denominator, 1, 32);
        int best = 1;
        for (int p = 1; p <= 32; p *= 2)
            if (std::abs(clamped - p) < std::abs(clamped - best))
                best = p;
        return best;
    }
}
