/*
  ==============================================================================
    EventScheduler.cpp

    Event scheduling implementation

  ==============================================================================
*/

#include "EventScheduler.h"

EventScheduler::EventScheduler()
{
}

void EventScheduler::scheduleEvent(const juce::MidiMessage& message, int64_t sampleTime, int priority)
{
    ScheduledEvent event;
    event.message = message;
    event.scheduledSample = sampleTime;
    event.priority = priority;

    eventQueue.push(event);
}

void EventScheduler::scheduleNoteOn(int note, float velocity, int channel, int64_t sampleTime)
{
    auto message = juce::MidiMessage::noteOn(channel, note,
        static_cast<juce::uint8>(juce::jlimit(0.0f, 1.0f, velocity) * 127.0f));
    scheduleEvent(message, sampleTime, 10); // Higher priority for note-ons
}

void EventScheduler::scheduleNoteOff(int note, int channel, int64_t sampleTime)
{
    auto message = juce::MidiMessage::noteOff(channel, note);
    scheduleEvent(message, sampleTime, 5); // Lower priority for note-offs
}

void EventScheduler::scheduleNote(int note, float velocity, int channel, int64_t startSample, int64_t duration)
{
    scheduleNoteOn(note, velocity, channel, startSample);
    scheduleNoteOff(note, channel, startSample + duration);
}

void EventScheduler::scheduleAftertouch(int note, float pressure, int channel, int64_t sampleTime)
{
    int pressureValue = static_cast<int>(juce::jlimit(0.0f, 1.0f, pressure) * 127.0f);
    auto message = juce::MidiMessage::aftertouchChange(channel, note, pressureValue);
    scheduleEvent(message, sampleTime, 7); // Medium priority
}

void EventScheduler::schedulePitchBend(float bendAmount, int channel, int64_t sampleTime)
{
    // Convert -1.0 to +1.0 range to 0-16383 MIDI pitch bend range
    int bendValue = static_cast<int>((bendAmount + 1.0f) * 0.5f * 16383.0f);
    bendValue = juce::jlimit(0, 16383, bendValue);
    auto message = juce::MidiMessage::pitchWheel(channel, bendValue);
    scheduleEvent(message, sampleTime, 8); // Medium-high priority
}

void EventScheduler::scheduleCC(int ccNumber, float value, int channel, int64_t sampleTime)
{
    int ccValue = static_cast<int>(juce::jlimit(0.0f, 1.0f, value) * 127.0f);
    auto message = juce::MidiMessage::controllerEvent(channel, ccNumber, ccValue);
    scheduleEvent(message, sampleTime, 7); // Medium priority
}

void EventScheduler::processEvents(int64_t currentSample, juce::MidiBuffer& outputBuffer, int bufferSize)
{
    int64_t endSample = currentSample + bufferSize;
    std::vector<ScheduledEvent> futureEvents;

    while (!eventQueue.empty())
    {
        const auto& event = eventQueue.top();

        // Check if event should fire in this buffer
        if (event.scheduledSample < endSample)
        {
            // Calculate sample offset within buffer
            int sampleOffset = static_cast<int>(event.scheduledSample - currentSample);
            sampleOffset = juce::jlimit(0, bufferSize - 1, sampleOffset);

            outputBuffer.addEvent(event.message, sampleOffset);
            eventQueue.pop();
        }
        else
        {
            // Event is in the future, stop processing
            break;
        }
    }
}

void EventScheduler::clearAll()
{
    while (!eventQueue.empty())
        eventQueue.pop();
}

void EventScheduler::clearFutureEvents(int64_t fromSample)
{
    std::priority_queue<ScheduledEvent> newQueue;

    while (!eventQueue.empty())
    {
        auto event = eventQueue.top();
        eventQueue.pop();

        if (event.scheduledSample < fromSample)
            newQueue.push(event);
    }

    eventQueue = std::move(newQueue);
}

int EventScheduler::getQueueSize() const
{
    return eventQueue.size();
}

void EventScheduler::setLookahead(int samples)
{
    lookaheadSamples = juce::jmax(0, samples);
}
