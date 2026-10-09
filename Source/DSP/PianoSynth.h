/*
  ==============================================================================
    PianoSynth.h

    Small polyphonic tone used by the standalone app. Fixed voices, no heap.
    The plugin formats stay MIDI effects and do not open this output.

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

class PianoSynth
{
public:
    static constexpr int kVoices = 16;

    void reset() noexcept
    {
        bendRatio = 1.0f;
        for (auto& voice : voices)
            voice = {};
    }

    void noteOn(int note, float velocity) noexcept
    {
        note = juce::jlimit(0, 127, note);
        const float amp = juce::jlimit(0.0f, 1.0f, velocity);
        if (amp <= 0.0f)
        {
            noteOff(note);
            return;
        }

        Voice* slot = nullptr;
        for (auto& voice : voices)
        {
            if (voice.note == note)
            {
                slot = &voice;
                break;
            }
        }

        if (slot == nullptr)
        {
            for (auto& voice : voices)
            {
                if (!voice.active)
                {
                    slot = &voice;
                    break;
                }
            }
        }

        if (slot == nullptr)
        {
            slot = &voices[0];
            for (auto& voice : voices)
                if (voice.level < slot->level)
                    slot = &voice;
        }

        *slot = {};
        slot->active = true;
        slot->releasing = false;
        slot->note = note;
        slot->velocity = amp;
        slot->level = amp;
    }

    void noteOff(int note) noexcept
    {
        for (auto& voice : voices)
            if (voice.active && voice.note == note)
                voice.releasing = true;
    }

    void allNotesOff() noexcept
    {
        for (auto& voice : voices)
            if (voice.active)
                voice.releasing = true;
    }

    void setPitchWheel(int value) noexcept
    {
        const float bend = (static_cast<float>(juce::jlimit(0, 16383, value)) - 8192.0f) / 8192.0f;
        bendRatio = std::pow(2.0f, bend * 2.0f / 12.0f);
    }

    void render(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi, double sampleRate) noexcept
    {
        const int numSamples = buffer.getNumSamples();
        const int channels = buffer.getNumChannels();
        if (numSamples <= 0 || channels <= 0 || sampleRate <= 0.0)
            return;

        int rendered = 0;
        for (const auto metadata : midi)
        {
            const int at = juce::jlimit(0, numSamples, metadata.samplePosition);
            renderRange(buffer, rendered, at, sampleRate);
            apply(metadata.getMessage());
            rendered = at;
        }

        renderRange(buffer, rendered, numSamples, sampleRate);
    }

private:
    struct Voice
    {
        bool active = false;
        bool releasing = false;
        int note = -1;
        float velocity = 0.0f;
        float level = 0.0f;
        double phase[3] {};
    };

    void apply(const juce::MidiMessage& message) noexcept
    {
        if (message.isNoteOn())
            noteOn(message.getNoteNumber(), message.getFloatVelocity());
        else if (message.isNoteOff())
            noteOff(message.getNoteNumber());
        else if (message.isAllNotesOff() || message.isAllSoundOff())
            allNotesOff();
        else if (message.isPitchWheel())
            setPitchWheel(message.getPitchWheelValue());
    }

    void renderRange(juce::AudioBuffer<float>& buffer, int start, int end, double sampleRate) noexcept
    {
        if (end <= start)
            return;

        const float dt = static_cast<float>(1.0 / sampleRate);
        auto* left = buffer.getWritePointer(0);
        auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

        for (int i = start; i < end; ++i)
        {
            float mix = 0.0f;
            for (auto& voice : voices)
            {
                if (!voice.active)
                    continue;

                const float freq = 440.0f
                    * std::pow(2.0f, (static_cast<float>(voice.note) - 69.0f) / 12.0f)
                    * bendRatio;
                float tone = 0.0f;
                constexpr float harmonic[3] = { 1.0f, 0.32f, 0.11f };
                for (int h = 0; h < 3; ++h)
                {
                    voice.phase[h] += juce::MathConstants<double>::twoPi
                        * static_cast<double>(h + 1) * static_cast<double>(freq) / sampleRate;
                    while (voice.phase[h] > juce::MathConstants<double>::twoPi)
                        voice.phase[h] -= juce::MathConstants<double>::twoPi;
                    tone += harmonic[h] * static_cast<float>(std::sin(voice.phase[h]));
                }

                const float decay = voice.releasing ? 8.0f : (1.6f + (static_cast<float>(voice.note) - 60.0f) * 0.03f);
                voice.level *= std::exp(-decay * dt);
                if (voice.level < 0.0008f)
                {
                    voice.active = false;
                    continue;
                }

                mix += tone * voice.level * voice.velocity;
            }

            mix = std::tanh(mix * 0.18f);
            left[i] += mix;
            if (right != nullptr)
                right[i] += mix;
        }
    }

    Voice voices[kVoices];
    float bendRatio = 1.0f;
};
