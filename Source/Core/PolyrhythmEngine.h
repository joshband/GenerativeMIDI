/*
  ==============================================================================
    PolyrhythmEngine.h

    Polyrhythmic and polymeter sequencing engine
    Supports multiple simultaneous time divisions and phase relationships

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

struct PolyrhythmLayer
{
    int division = 4;           // Time division (e.g., 4 = quarter notes vs 16th clock)
    int length = 16;            // Pattern length in steps
    float phase = 0.0f;         // Phase offset (0.0 - 1.0)
    bool enabled = true;

    int pitchOffset = 0;              // Layer transpose in semitones (-24..24)
    float velocityMultiplier = 1.0f;  // Layer velocity scale (0..2)

    std::vector<bool> pattern;
    std::vector<float> velocities;
    std::vector<int> pitches;   // MIDI note numbers

    int currentStep = 0;
    int tickCounter = 0;        // Accumulator for division rate scaling

    void resize(int newLength)
    {
        length = newLength;
        pattern.resize(static_cast<size_t>(length), false);
        velocities.resize(static_cast<size_t>(length), 0.8f);
        pitches.resize(static_cast<size_t>(length), 60); // Middle C
    }
};

class PolyrhythmEngine
{
public:
    PolyrhythmEngine();
    ~PolyrhythmEngine() = default;

    // Layer management
    int addLayer();
    void removeLayer(int layerIndex);
    PolyrhythmLayer* getLayer(int layerIndex);
    int getNumLayers() const { return layers.size(); }

    // Layer configuration
    void setLayerDivision(int layerIndex, int division);
    void setLayerLength(int layerIndex, int length);
    void setLayerPhase(int layerIndex, float phase);
    void setLayerEnabled(int layerIndex, bool enabled);
    void setLayerPitchOffset(int layerIndex, int semitones);
    void setLayerVelocityMultiplier(int layerIndex, float multiplier);

    /**
     * On each clock tick (clockSubdivision usually 16 = sixteenths):
     * returns true when this layer should emit at currentStep (caller plays, then advanceStep).
     * Mapping: division 4 advances every 4 sixteenth ticks; division 16 every tick.
     */
    bool shouldEmitOnThisTick(int layerIndex, int clockSubdivision);
    void advanceStep(int layerIndex);

    // Pattern editing
    void setStep(int layerIndex, int stepIndex, bool active, float velocity = 0.8f, int pitch = 60);
    void clearLayer(int layerIndex);
    void randomizeLayer(int layerIndex, float density);

    // Playback
    void advance(int layerIndex, int subdivisions); // legacy: emit-boundary then advanceStep
    void reset();
    void resetLayer(int layerIndex);

    // Time signature
    void setTimeSignature(int numerator, int denominator);
    void setTempo(double bpm);

    /** ValueTree type used in session / preset XML (child of APVTS root). */
    static constexpr const char* kStateTreeType = "PolyrhythmLayers";

    /** Serialize editable layer fields (not live playback counters). */
    juce::ValueTree toValueTree() const;

    /** Replace all layers from a PolyrhythmLayers tree. No-op if type mismatches. */
    void loadFromValueTree(const juce::ValueTree& tree);

private:
    std::vector<PolyrhythmLayer> layers;
    int timeSignatureNum = 4;
    int timeSignatureDenom = 4;
    double tempo = 120.0;

    juce::Random random;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyrhythmEngine)
};
