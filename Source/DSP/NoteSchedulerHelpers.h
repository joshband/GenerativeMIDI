/*
  ==============================================================================
    NoteSchedulerHelpers.h

    Shared realtime-safe note scheduling used by PluginProcessor generators.

  ==============================================================================
*/

#pragma once

#include "EventScheduler.h"
#include "../Core/SwingEngine.h"
#include "../Core/GateLengthController.h"
#include "../Core/RatchetEngine.h"

namespace NoteSchedulerHelpers
{
    /** Schedule a note with optional ratcheting. Uses stack buffer for offsets (max 16). */
    inline void scheduleGeneratedNote(EventScheduler& scheduler,
                                      RatchetEngine& ratchetEngine,
                                      GateLengthController& gateLengthController,
                                      int pitch,
                                      float velocity,
                                      int midiChannel,
                                      int64_t baseSample,
                                      int timingOffset,
                                      int samplesPerStep,
                                      bool useRatcheting)
    {
        int ratchetOffsets[16];
        int ratchetCount = 1;
        ratchetOffsets[0] = 0;

        if (useRatcheting)
            ratchetCount = ratchetEngine.fillRatchetOffsets(samplesPerStep, ratchetOffsets, 16);

        for (int ratchetIdx = 0; ratchetIdx < ratchetCount; ++ratchetIdx)
        {
            const float ratchetVelocity = ratchetEngine.calculateRatchetVelocity(velocity, ratchetIdx);
            const int ratchetTimingOffset = timingOffset + ratchetOffsets[ratchetIdx];

            scheduler.scheduleNoteOn(pitch, ratchetVelocity, midiChannel,
                                     baseSample + ratchetTimingOffset);

            const int noteDuration = gateLengthController.calculateGateLengthSamples(samplesPerStep);
            scheduler.scheduleNoteOff(pitch, midiChannel,
                                      baseSample + ratchetTimingOffset + noteDuration);
        }
    }
}
