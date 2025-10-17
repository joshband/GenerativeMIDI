/*
  ==============================================================================
    MIDIGenerator.cpp

    MIDI event generation implementation

  ==============================================================================
*/

#include "MIDIGenerator.h"
#include <cmath>

// ============================================================================
// MIDIExpressionEvent Implementation
// ============================================================================
juce::MidiMessage MIDIExpressionEvent::toMidiMessage() const
{
    switch (type)
    {
        case NoteOn:
            return juce::MidiMessage::noteOn(channel, note,
                static_cast<juce::uint8>(juce::jlimit(0.0f, 1.0f, velocity) * 127.0f));

        case NoteOff:
            return juce::MidiMessage::noteOff(channel, note);

        case PolyAftertouch:
            return juce::MidiMessage::aftertouchChange(channel, note,
                static_cast<int>(juce::jlimit(0.0f, 1.0f, pressure) * 127.0f));

        case ChannelPressure:
            return juce::MidiMessage::channelPressureChange(channel,
                static_cast<int>(juce::jlimit(0.0f, 1.0f, pressure) * 127.0f));

        case ControlChange:
            return juce::MidiMessage::controllerEvent(channel, ccNumber,
                static_cast<int>(juce::jlimit(0.0f, 1.0f, ccValue) * 127.0f));

        case PitchBend:
        {
            int bendValue = static_cast<int>((juce::jlimit(-1.0f, 1.0f, pitchBend) + 1.0f) * 8192.0f);
            return juce::MidiMessage::pitchWheel(channel, bendValue);
        }

        case ProgramChange:
            return juce::MidiMessage::programChange(channel, program);

        default:
            return juce::MidiMessage();
    }
}

// ============================================================================
// MIDIGenerator Implementation
// ============================================================================
MIDIGenerator::MIDIGenerator()
{
}

void MIDIGenerator::generateNote(int note, float velocity, int channel, int timeStamp)
{
    velocity = applyVelocityCurve(velocity);

    MIDIExpressionEvent event;
    event.type = MIDIExpressionEvent::NoteOn;
    event.note = juce::jlimit(0, 127, note);
    event.velocity = velocity;
    event.channel = juce::jlimit(1, 16, channel);
    event.timeStamp = timeStamp;

    auto message = event.toMidiMessage();
    eventBuffer.addEvent(message, timeStamp);

    // Generate initial expression events if enabled
    if (noteExpression.enablePolyAftertouch && noteExpression.aftertouchAmount > 0.0f)
    {
        generatePolyAftertouch(note, noteExpression.aftertouchAmount, channel, timeStamp);
    }
}

void MIDIGenerator::generateNoteOff(int note, int channel, int timeStamp)
{
    MIDIExpressionEvent event;
    event.type = MIDIExpressionEvent::NoteOff;
    event.note = juce::jlimit(0, 127, note);
    event.channel = juce::jlimit(1, 16, channel);
    event.timeStamp = timeStamp;

    auto message = event.toMidiMessage();
    eventBuffer.addEvent(message, timeStamp);
}

void MIDIGenerator::generateChord(const std::vector<int>& notes, float velocity, int channel, int timeStamp)
{
    for (int note : notes)
    {
        generateNote(note, velocity, channel, timeStamp);
    }
}

void MIDIGenerator::generatePolyAftertouch(int note, float pressure, int channel, int timeStamp)
{
    MIDIExpressionEvent event;
    event.type = MIDIExpressionEvent::PolyAftertouch;
    event.note = juce::jlimit(0, 127, note);
    event.pressure = juce::jlimit(0.0f, 1.0f, pressure);
    event.channel = juce::jlimit(1, 16, channel);
    event.timeStamp = timeStamp;

    auto message = event.toMidiMessage();
    eventBuffer.addEvent(message, timeStamp);
}

void MIDIGenerator::generateChannelPressure(float pressure, int channel, int timeStamp)
{
    MIDIExpressionEvent event;
    event.type = MIDIExpressionEvent::ChannelPressure;
    event.pressure = juce::jlimit(0.0f, 1.0f, pressure);
    event.channel = juce::jlimit(1, 16, channel);
    event.timeStamp = timeStamp;

    auto message = event.toMidiMessage();
    eventBuffer.addEvent(message, timeStamp);
}

void MIDIGenerator::generatePitchBend(float amount, int channel, int timeStamp)
{
    MIDIExpressionEvent event;
    event.type = MIDIExpressionEvent::PitchBend;
    event.pitchBend = juce::jlimit(-1.0f, 1.0f, amount);
    event.channel = juce::jlimit(1, 16, channel);
    event.timeStamp = timeStamp;

    auto message = event.toMidiMessage();
    eventBuffer.addEvent(message, timeStamp);
}

void MIDIGenerator::generateCC(int ccNumber, float value, int channel, int timeStamp)
{
    MIDIExpressionEvent event;
    event.type = MIDIExpressionEvent::ControlChange;
    event.ccNumber = juce::jlimit(0, 127, ccNumber);
    event.ccValue = juce::jlimit(0.0f, 1.0f, value);
    event.channel = juce::jlimit(1, 16, channel);
    event.timeStamp = timeStamp;

    auto message = event.toMidiMessage();
    eventBuffer.addEvent(message, timeStamp);
}

void MIDIGenerator::generateProgramChange(int program, int channel, int timeStamp)
{
    MIDIExpressionEvent event;
    event.type = MIDIExpressionEvent::ProgramChange;
    event.program = juce::jlimit(0, 127, program);
    event.channel = juce::jlimit(1, 16, channel);
    event.timeStamp = timeStamp;

    auto message = event.toMidiMessage();
    eventBuffer.addEvent(message, timeStamp);
}

void MIDIGenerator::addCCModulation(int ccNumber, float rate, float depth, CCModulation::WaveShape shape)
{
    CCModulation mod;
    mod.ccNumber = ccNumber;
    mod.rate = rate;
    mod.depth = depth;
    mod.shape = shape;
    mod.phase = 0.0f;
    mod.enabled = true;

    ccModulations[ccNumber] = mod;
}

void MIDIGenerator::removeCCModulation(int ccNumber)
{
    ccModulations.erase(ccNumber);
}

void MIDIGenerator::updateCCModulation(double sampleRate, int numSamples)
{
    for (auto& [ccNum, mod] : ccModulations)
    {
        if (!mod.enabled)
            continue;

        // Update phase
        float phaseIncrement = (mod.rate * numSamples) / static_cast<float>(sampleRate);
        mod.phase = std::fmod(mod.phase + phaseIncrement, 1.0f);

        // Calculate modulation value
        float modValue = calculateLFO(mod);
        float ccValue = 0.5f + (modValue * mod.depth * 0.5f);

        // Generate CC event
        generateCC(mod.ccNumber, ccValue, 1, 0);
    }
}

CCModulation* MIDIGenerator::getCCModulation(int ccNumber)
{
    auto it = ccModulations.find(ccNumber);
    if (it != ccModulations.end())
        return &it->second;
    return nullptr;
}

void MIDIGenerator::setNoteExpression(const NoteExpression& expression)
{
    noteExpression = expression;
}

void MIDIGenerator::clearEvents()
{
    eventBuffer.clear();
    queuedEvents.clear();
}

float MIDIGenerator::applyVelocityCurve(float inputVelocity)
{
    float scaled = inputVelocity * noteExpression.velocityScale + noteExpression.velocityOffset;
    scaled = juce::jlimit(0.0f, 1.0f, scaled);

    switch (noteExpression.velocityCurve)
    {
        case NoteExpression::Linear:
            return scaled;

        case NoteExpression::Exponential:
            return scaled * scaled;

        case NoteExpression::Logarithmic:
            return std::sqrt(scaled);

        case NoteExpression::Custom:
        default:
            return scaled;
    }
}

int MIDIGenerator::floatToMidiValue(float value)
{
    return static_cast<int>(juce::jlimit(0.0f, 1.0f, value) * 127.0f);
}

float MIDIGenerator::midiValueToFloat(int value)
{
    return juce::jlimit(0, 127, value) / 127.0f;
}

float MIDIGenerator::calculateLFO(const CCModulation& mod)
{
    float phase = mod.phase * juce::MathConstants<float>::twoPi;

    switch (mod.shape)
    {
        case CCModulation::Sine:
            return std::sin(phase);

        case CCModulation::Triangle:
            return 1.0f - std::abs(std::fmod(mod.phase * 4.0f + 1.0f, 4.0f) - 2.0f);

        case CCModulation::Square:
            return mod.phase < 0.5f ? 1.0f : -1.0f;

        case CCModulation::Saw:
            return 2.0f * mod.phase - 1.0f;

        case CCModulation::Random:
            return juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;

        default:
            return 0.0f;
    }
}
