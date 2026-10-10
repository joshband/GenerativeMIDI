/*
  ==============================================================================
    EventScheduler.h

    Schedules and manages timed MIDI events
    Handles event queuing, priority, and real-time dispatch

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "MidiActivityLog.h"
#include <algorithm>
#include <vector>

struct ScheduledEvent
{
    juce::MidiMessage message;
    int64_t scheduledSample = 0;
    int priority = 0;  // Higher priority events fire first at same time
                       // (note-off 12 > note-on 10 > ... so a retrigger never lands before its release)

    bool operator<(const ScheduledEvent& other) const
    {
        if (scheduledSample == other.scheduledSample)
            return priority < other.priority; // Lower priority value = later in queue
        return scheduledSample > other.scheduledSample; // Earlier samples first
    }
};

class EventScheduler
{
public:
    EventScheduler();
    ~EventScheduler() = default;

    /** Pre-reserve queue storage (call from prepareToPlay — not realtime). */
    void prepare(int capacity);

    // Event scheduling
    void scheduleEvent(const juce::MidiMessage& message, int64_t sampleTime, int priority = 0);
    void scheduleNoteOn(int note, float velocity, int channel, int64_t sampleTime);
    void scheduleNoteOff(int note, int channel, int64_t sampleTime);

    // Note scheduling with duration
    void scheduleNote(int note, float velocity, int channel, int64_t startSample, int64_t duration);

    // Expression scheduling
    void scheduleAftertouch(int note, float pressure, int channel, int64_t sampleTime);
    void schedulePitchBend(float bendAmount, int channel, int64_t sampleTime);
    void scheduleCC(int ccNumber, float value, int channel, int64_t sampleTime);

    // Event retrieval (optional activityLog receives note-on/off as they fire)
    void processEvents(int64_t currentSample, juce::MidiBuffer& outputBuffer, int bufferSize,
                       MidiActivityLog* activityLog = nullptr);

    // Queue management
    void clearAll();
    void clearFutureEvents(int64_t fromSample);
    int getQueueSize() const;

    /** Remove every queued note-on / note-off for this pitch and channel scheduled after
        `afterSample` (used when a mono voice steals a note). Realtime-safe, no allocation. */
    void cancelNoteEventsAfter(int note, int channel, int64_t afterSample);

    // Lookahead
    void setLookahead(int samples);
    int getLookahead() const { return lookaheadSamples; }

private:
    void emitEvent(const juce::MidiMessage& message, int sampleOffset,
                   juce::MidiBuffer& outputBuffer, MidiActivityLog* activityLog);

    static constexpr int kPriorityNoteOff = 12;
    static constexpr int kPriorityNoteOn = 10;

    std::vector<ScheduledEvent> eventStorage;

    // Notes emitted but not yet released, per channel/pitch. A note-on for a pitch that is
    // already sounding first gets a note-off, and only the last matching note-off is sent.
    uint8_t noteDepth[16][128] {};
    int lookaheadSamples = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EventScheduler)
};
