/*
  ==============================================================================
    MidiActivityLog.h

    Single-producer (audio) / single-consumer (message thread) ring of recent
    MIDI note events. No allocations on the audio thread.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <cstdint>

struct MidiActivityEvent
{
    enum class Type : uint8_t { NoteOn = 0, NoteOff = 1 };

    Type type = Type::NoteOn;
    uint8_t note = 0;
    uint8_t velocity = 0;
    uint8_t channel = 1; // MIDI channel 1–16
};

/**
 * Fixed-capacity AbstractFifo of note-on / note-off summaries.
 * When full, newest events are dropped (tryPush returns false).
 * Note: juce::AbstractFifo holds (capacity - 1) items; kMaxEvents is usable slots.
 */
class MidiActivityLog
{
public:
    static constexpr int kMaxEvents = 63;
    static constexpr int kCapacity = kMaxEvents + 1; // AbstractFifo needs +1

    MidiActivityLog() = default;

    /** Audio-thread safe: push one event if capacity remains. */
    bool tryPush (const MidiActivityEvent& event) noexcept
    {
        int start1 = 0, size1 = 0, start2 = 0, size2 = 0;
        fifo.prepareToWrite (1, start1, size1, start2, size2);

        if (size1 + size2 < 1)
            return false;

        buffer[static_cast<size_t> (start1)] = event;
        fifo.finishedWrite (1);
        return true;
    }

    /** Audio-thread safe: push note-on / note-off; ignore other message types. */
    bool tryPushFromMessage (const juce::MidiMessage& message) noexcept
    {
        if (message.isNoteOn())
        {
            MidiActivityEvent e;
            e.type = MidiActivityEvent::Type::NoteOn;
            e.note = static_cast<uint8_t> (juce::jlimit (0, 127, message.getNoteNumber()));
            e.velocity = static_cast<uint8_t> (juce::jlimit (0, 127, static_cast<int> (message.getVelocity())));
            e.channel = static_cast<uint8_t> (juce::jlimit (1, 16, message.getChannel()));
            return tryPush (e);
        }

        if (message.isNoteOff())
        {
            MidiActivityEvent e;
            e.type = MidiActivityEvent::Type::NoteOff;
            e.note = static_cast<uint8_t> (juce::jlimit (0, 127, message.getNoteNumber()));
            e.velocity = static_cast<uint8_t> (juce::jlimit (0, 127, static_cast<int> (message.getVelocity())));
            e.channel = static_cast<uint8_t> (juce::jlimit (1, 16, message.getChannel()));
            return tryPush (e);
        }

        return false;
    }

    /** Message-thread: drain up to maxCount events into dest. Returns count. */
    int pop (MidiActivityEvent* dest, int maxCount) noexcept
    {
        if (dest == nullptr || maxCount <= 0)
            return 0;

        int start1 = 0, size1 = 0, start2 = 0, size2 = 0;
        fifo.prepareToRead (maxCount, start1, size1, start2, size2);

        int n = 0;
        for (int i = 0; i < size1; ++i)
            dest[n++] = buffer[static_cast<size_t> (start1 + i)];
        for (int i = 0; i < size2; ++i)
            dest[n++] = buffer[static_cast<size_t> (start2 + i)];

        fifo.finishedRead (n);
        return n;
    }

    int getNumReady() const noexcept { return fifo.getNumReady(); }

    /** Format a single event for UI (message thread / tests). */
    static juce::String formatEvent (const MidiActivityEvent& e)
    {
        const char* type = (e.type == MidiActivityEvent::Type::NoteOn) ? "ON " : "OFF";
        const auto name = juce::MidiMessage::getMidiNoteName (e.note, true, true, 3);
        return juce::String (type) + "  " + name
             + "  vel " + juce::String (static_cast<int> (e.velocity)).paddedLeft (' ', 3)
             + "  ch " + juce::String (static_cast<int> (e.channel));
    }

private:
    juce::AbstractFifo fifo { kCapacity };
    std::array<MidiActivityEvent, static_cast<size_t> (kCapacity)> buffer {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiActivityLog)
};
