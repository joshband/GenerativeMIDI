/*
  ==============================================================================
    EventScheduler.h

    Schedules and manages timed MIDI events
    Handles event queuing, priority, and real-time dispatch

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <queue>
#include <vector>

struct ScheduledEvent
{
    juce::MidiMessage message;
    int64_t scheduledSample = 0;
    int priority = 0;  // Higher priority events fire first at same time

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

    // Event scheduling
    void scheduleEvent(const juce::MidiMessage& message, int64_t sampleTime, int priority = 0);
    void scheduleNoteOn(int note, float velocity, int channel, int64_t sampleTime);
    void scheduleNoteOff(int note, int channel, int64_t sampleTime);

    // Note scheduling with duration
    void scheduleNote(int note, float velocity, int channel, int64_t startSample, int64_t duration);

    // Event retrieval
    void processEvents(int64_t currentSample, juce::MidiBuffer& outputBuffer, int bufferSize);

    // Queue management
    void clearAll();
    void clearFutureEvents(int64_t fromSample);
    int getQueueSize() const;

    // Lookahead
    void setLookahead(int samples);
    int getLookahead() const { return lookaheadSamples; }

private:
    std::priority_queue<ScheduledEvent> eventQueue;
    int lookaheadSamples = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EventScheduler)
};
