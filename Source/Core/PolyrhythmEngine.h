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
    int division = 4;           // Time division (e.g., 4 = quarter notes)
    int length = 16;            // Pattern length in subdivisions
    float phase = 0.0f;         // Phase offset (0.0 - 1.0)
    bool enabled = true;

    std::vector<bool> pattern;
    std::vector<float> velocities;
    std::vector<int> pitches;   // MIDI note numbers

    int currentStep = 0;

    void resize(int newLength)
    {
        length = newLength;
        pattern.resize(length, false);
        velocities.resize(length, 0.8f);
        pitches.resize(length, 60); // Middle C
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

    // Pattern editing
    void setStep(int layerIndex, int stepIndex, bool active, float velocity = 0.8f, int pitch = 60);
    void clearLayer(int layerIndex);
    void randomizeLayer(int layerIndex, float density);

    // Playback
    void advance(int layerIndex, int subdivisions);
    void reset();
    void resetLayer(int layerIndex);

    // Time signature
    void setTimeSignature(int numerator, int denominator);
    void setTempo(double bpm);

private:
    std::vector<PolyrhythmLayer> layers;
    int timeSignatureNum = 4;
    int timeSignatureDenom = 4;
    double tempo = 120.0;

    juce::Random random;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyrhythmEngine)
};
