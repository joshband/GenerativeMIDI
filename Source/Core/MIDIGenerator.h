/*
  ==============================================================================
    MIDIGenerator.h

    MIDI event generation with extended expression capabilities
    Supports notes, aftertouch, CC, pitch bend, and more

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

// ============================================================================
// MIDI Expression Event
// ============================================================================
struct MIDIExpressionEvent
{
    enum Type
    {
        NoteOn,
        NoteOff,
        PolyAftertouch,    // Per-note pressure
        ChannelPressure,   // Channel-wide pressure
        ControlChange,     // CC messages
        PitchBend,
        ProgramChange
    };

    Type type = NoteOn;
    int channel = 1;           // MIDI channel (1-16)
    int note = 60;             // Note number (0-127)
    float velocity = 0.8f;     // Velocity (0.0-1.0)
    int ccNumber = 1;          // CC number (0-127)
    float ccValue = 0.5f;      // CC value (0.0-1.0)
    float pitchBend = 0.0f;    // Pitch bend (-1.0 to +1.0)
    float pressure = 0.0f;     // Aftertouch pressure (0.0-1.0)
    int program = 0;           // Program number (0-127)
    int timeStamp = 0;         // Sample offset

    juce::MidiMessage toMidiMessage() const;
};

// ============================================================================
// CC Modulation Configuration
// ============================================================================
struct CCModulation
{
    int ccNumber = 1;          // CC to modulate
    float rate = 1.0f;         // LFO rate in Hz
    float depth = 0.5f;        // Modulation depth (0.0-1.0)
    float phase = 0.0f;        // Current phase
    bool enabled = true;

    enum WaveShape
    {
        Sine,
        Triangle,
        Square,
        Saw,
        Random
    };

    WaveShape shape = Sine;
};

// ============================================================================
// Note Expression Configuration
// ============================================================================
struct NoteExpression
{
    bool enablePolyAftertouch = false;
    bool enableChannelPressure = false;
    bool enablePitchBend = false;

    float aftertouchAmount = 0.0f;
    float pitchBendAmount = 0.0f;

    // Velocity curve
    enum VelocityCurve
    {
        Linear,
        Exponential,
        Logarithmic,
        Custom
    };

    VelocityCurve velocityCurve = Linear;
    float velocityScale = 1.0f;
    float velocityOffset = 0.0f;
};

// ============================================================================
// Main MIDI Generator
// ============================================================================
class MIDIGenerator
{
public:
    MIDIGenerator();
    ~MIDIGenerator() = default;

    // Note generation
    void generateNote(int note, float velocity, int channel = 1, int timeStamp = 0);
    void generateNoteOff(int note, int channel = 1, int timeStamp = 0);
    void generateChord(const std::vector<int>& notes, float velocity, int channel = 1, int timeStamp = 0);

    // Expression generation
    void generatePolyAftertouch(int note, float pressure, int channel = 1, int timeStamp = 0);
    void generateChannelPressure(float pressure, int channel = 1, int timeStamp = 0);
    void generatePitchBend(float amount, int channel = 1, int timeStamp = 0);
    void generateCC(int ccNumber, float value, int channel = 1, int timeStamp = 0);
    void generateProgramChange(int program, int channel = 1, int timeStamp = 0);

    // CC Modulation
    void addCCModulation(int ccNumber, float rate, float depth, CCModulation::WaveShape shape);
    void removeCCModulation(int ccNumber);
    void updateCCModulation(double sampleRate, int numSamples);
    CCModulation* getCCModulation(int ccNumber);

    // Note expression
    void setNoteExpression(const NoteExpression& expression);
    NoteExpression& getNoteExpression() { return noteExpression; }

    // Event retrieval
    juce::MidiBuffer& getEventBuffer() { return eventBuffer; }
    void clearEvents();

    // Utility
    float applyVelocityCurve(float inputVelocity);
    static int floatToMidiValue(float value);
    static float midiValueToFloat(int value);

private:
    juce::MidiBuffer eventBuffer;
    std::vector<MIDIExpressionEvent> queuedEvents;
    std::map<int, CCModulation> ccModulations;
    NoteExpression noteExpression;

    float currentPhase = 0.0f;

    float calculateLFO(const CCModulation& mod);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MIDIGenerator)
};
