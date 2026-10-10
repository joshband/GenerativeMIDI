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

bool EventScheduler::scheduleEvent(const juce::MidiMessage& message, int64_t sampleTime, int priority)
{
    // Stay within reserved capacity — never grow on the audio thread. The last slice of
    // the queue is kept for note-offs: a dropped note-on is a missing note, a dropped
    // note-off is a stuck one.
    const size_t capacity = eventStorage.capacity();
    const bool isOff = message.isNoteOff();
    const size_t reserve = isOff ? 0 : juce::jmax<size_t>(8, capacity / 8);
    if (eventStorage.size() + 1 + reserve > capacity)
    {
        droppedEvents.fetch_add(1, std::memory_order_relaxed);
        if (isOff)
            droppedNoteOffs.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    ScheduledEvent event;
    event.message = message;
    event.scheduledSample = sampleTime;
    event.priority = priority;

    eventStorage.push_back(event);
    std::push_heap(eventStorage.begin(), eventStorage.end());
    return true;
}

bool EventScheduler::scheduleNoteOn(int note, float velocity, int channel, int64_t sampleTime)
{
    auto message = juce::MidiMessage::noteOn(channel, note,
        // Velocity 0 is a note-off to most receivers, so a note-on is never below 1.
        static_cast<juce::uint8>(juce::jmax(1, juce::roundToInt(juce::jlimit(0.0f, 1.0f, velocity) * 127.0f))));
    return scheduleEvent(message, sampleTime, kPriorityNoteOn);
}

void EventScheduler::scheduleNoteOff(int note, int channel, int64_t sampleTime)
{
    auto message = juce::MidiMessage::noteOff(channel, note);
    // Fires before a note-on at the same sample so a retrigger is never swallowed.
    scheduleEvent(message, sampleTime, kPriorityNoteOff);
}

void EventScheduler::scheduleNote(int note, float velocity, int channel, int64_t startSample, int64_t duration)
{
    if (!scheduleNoteOn(note, velocity, channel, startSample))
        return; // dropped: do not queue an orphan note-off for it
    // A zero-length note would put the off ahead of its own on at the same sample.
    scheduleNoteOff(note, channel, startSample + juce::jmax<int64_t>(1, duration));
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

            emitEvent(event.message, sampleOffset, outputBuffer, activityLog);

            std::pop_heap(eventStorage.begin(), eventStorage.end());
            eventStorage.pop_back();
        }
        else
        {
            break;
        }
    }
}

void EventScheduler::emitEvent(const juce::MidiMessage& message, int sampleOffset,
                               juce::MidiBuffer& outputBuffer, MidiActivityLog* activityLog)
{
    const bool isOn = message.isNoteOn();
    const bool isOff = !isOn && message.isNoteOff();

    if (isOn || isOff)
    {
        const int ch = juce::jlimit(1, 16, message.getChannel()) - 1;
        const int note = message.getNoteNumber() & 127;
        auto& depth = noteDepth[ch][note];

        if (isOn)
        {
            if (depth > 0)
            {
                // Same pitch still sounding: release it first so the new note is heard.
                const auto release = juce::MidiMessage::noteOff(ch + 1, note);
                outputBuffer.addEvent(release, sampleOffset);
                if (activityLog != nullptr)
                    activityLog->tryPushFromMessage(release);
            }
            if (depth < 255)
                ++depth;
        }
        else
        {
            if (depth == 0)
                return; // stale off for a note that was already released
            --depth;
            if (depth > 0)
                return; // an older note's late off must not cut the retriggered note
        }
    }

    if (message.isPitchWheel())
        bendMoved[juce::jlimit(1, 16, message.getChannel()) - 1] = message.getPitchWheelValue() != 8192;

    outputBuffer.addEvent(message, sampleOffset);
    if (activityLog != nullptr)
        activityLog->tryPushFromMessage(message);
}

bool EventScheduler::hasSoundingNotes() const
{
    for (const auto& channel : noteDepth)
        for (const auto depth : channel)
            if (depth > 0)
                return true;
    return false;
}

void EventScheduler::centrePitchBend(juce::MidiBuffer& outputBuffer, int sampleOffset,
                                     MidiActivityLog* activityLog)
{
    for (int ch = 0; ch < 16; ++ch)
    {
        if (!bendMoved[ch])
            continue;
        bendMoved[ch] = false;
        const auto centre = juce::MidiMessage::pitchWheel(ch + 1, 8192);
        outputBuffer.addEvent(centre, sampleOffset);
        if (activityLog != nullptr)
            activityLog->tryPushFromMessage(centre);
    }
}

void EventScheduler::allNotesOff(juce::MidiBuffer& outputBuffer, int sampleOffset,
                                 MidiActivityLog* activityLog)
{
    eventStorage.clear();

    for (int ch = 0; ch < 16; ++ch)
    {
        bool anySounding = false;
        for (int note = 0; note < 128; ++note)
        {
            if (noteDepth[ch][note] == 0)
                continue;
            noteDepth[ch][note] = 0;
            anySounding = true;
            const auto off = juce::MidiMessage::noteOff(ch + 1, note);
            outputBuffer.addEvent(off, sampleOffset);
            if (activityLog != nullptr)
                activityLog->tryPushFromMessage(off);
        }

        if (anySounding)
        {
            // Many synths ignore individual offs after a panic; CC123 covers them.
            const auto panic = juce::MidiMessage::allNotesOff(ch + 1);
            outputBuffer.addEvent(panic, sampleOffset);
            if (activityLog != nullptr)
                activityLog->tryPushFromMessage(panic);
        }
    }

    centrePitchBend(outputBuffer, sampleOffset, activityLog);
}

void EventScheduler::cancelNoteEventsAfter(int note, int channel, int64_t afterSample)
{
    size_t write = 0;
    for (size_t read = 0; read < eventStorage.size(); ++read)
    {
        const auto& m = eventStorage[read].message;
        const bool match = eventStorage[read].scheduledSample > afterSample
                           && (m.isNoteOn() || m.isNoteOff())
                           && m.getChannel() == channel && m.getNoteNumber() == note;
        if (!match)
        {
            if (write != read)
                eventStorage[write] = std::move(eventStorage[read]);
            ++write;
        }
    }
    eventStorage.resize(write);
    std::make_heap(eventStorage.begin(), eventStorage.end());
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
