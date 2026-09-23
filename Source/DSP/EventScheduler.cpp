/*
  ==============================================================================
    EventScheduler.cpp

    Event scheduling implementation — vector heap with reserved capacity

  ==============================================================================
*/

#include "EventScheduler.h"

EventScheduler::EventScheduler()
{
    eventStorage.reserve(256);
}

void EventScheduler::prepare(int capacity)
{
    eventStorage.reserve(static_cast<size_t>(juce::jmax(16, capacity)));
}

void EventScheduler::scheduleEvent(const juce::MidiMessage& message, int64_t sampleTime, int priority)
{
    // Stay within reserved capacity — never grow on the audio thread
    if (eventStorage.size() >= eventStorage.capacity())
        return;

    ScheduledEvent event;
    event.message = message;
    event.scheduledSample = sampleTime;
    event.priority = priority;

    eventStorage.push_back(event);
    std::push_heap(eventStorage.begin(), eventStorage.end());
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

void EventScheduler::processEvents(int64_t currentSample, juce::MidiBuffer& outputBuffer, int bufferSize,
                                   MidiActivityLog* activityLog)
{
    int64_t endSample = currentSample + bufferSize;

    while (!eventStorage.empty())
    {
        // Peek heap top (front after make/push_heap)
        const auto& event = eventStorage.front();

        if (event.scheduledSample < endSample)
        {
            int sampleOffset = static_cast<int>(event.scheduledSample - currentSample);
            sampleOffset = juce::jlimit(0, bufferSize - 1, sampleOffset);

            outputBuffer.addEvent(event.message, sampleOffset);

            if (activityLog != nullptr)
                activityLog->tryPushFromMessage(event.message);

            std::pop_heap(eventStorage.begin(), eventStorage.end());
            eventStorage.pop_back();
        }
        else
        {
            break;
        }
    }
}

void EventScheduler::clearAll()
{
    eventStorage.clear(); // retains capacity
}

void EventScheduler::clearFutureEvents(int64_t fromSample)
{
    // Filter in place — no secondary queue allocation
    size_t write = 0;
    for (size_t read = 0; read < eventStorage.size(); ++read)
    {
        if (eventStorage[read].scheduledSample < fromSample)
        {
            if (write != read)
                eventStorage[write] = std::move(eventStorage[read]);
            ++write;
        }
    }
    eventStorage.resize(write);
    std::make_heap(eventStorage.begin(), eventStorage.end());
}

int EventScheduler::getQueueSize() const
{
    return static_cast<int>(eventStorage.size());
}

void EventScheduler::setLookahead(int samples)
{
    lookaheadSamples = juce::jmax(0, samples);
}
