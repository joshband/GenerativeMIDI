/*
  ==============================================================================
    PolyrhythmEngine.cpp

    Polyrhythmic sequencing implementation

  ==============================================================================
*/

#include "PolyrhythmEngine.h"

PolyrhythmEngine::PolyrhythmEngine()
{
    // Create default layer
    addLayer();
}

int PolyrhythmEngine::addLayer()
{
    PolyrhythmLayer layer;
    layer.resize(16);
    layers.push_back(layer);
    return layers.size() - 1;
}

void PolyrhythmEngine::removeLayer(int layerIndex)
{
    if (layerIndex >= 0 && layerIndex < layers.size())
        layers.erase(layers.begin() + layerIndex);
}

PolyrhythmLayer* PolyrhythmEngine::getLayer(int layerIndex)
{
    if (layerIndex >= 0 && layerIndex < layers.size())
        return &layers[layerIndex];
    return nullptr;
}

void PolyrhythmEngine::setLayerDivision(int layerIndex, int division)
{
    auto* layer = getLayer(layerIndex);
    if (layer)
        layer->division = juce::jlimit(1, 64, division);
}

void PolyrhythmEngine::setLayerLength(int layerIndex, int length)
{
    auto* layer = getLayer(layerIndex);
    if (layer)
        layer->resize(juce::jlimit(1, 128, length));
}

void PolyrhythmEngine::setLayerPhase(int layerIndex, float phase)
{
    auto* layer = getLayer(layerIndex);
    if (layer)
        layer->phase = juce::jlimit(0.0f, 1.0f, phase);
}

void PolyrhythmEngine::setLayerEnabled(int layerIndex, bool enabled)
{
    auto* layer = getLayer(layerIndex);
    if (layer)
        layer->enabled = enabled;
}

void PolyrhythmEngine::setStep(int layerIndex, int stepIndex, bool active, float velocity, int pitch)
{
    auto* layer = getLayer(layerIndex);
    if (layer && stepIndex >= 0 && stepIndex < layer->length)
    {
        layer->pattern[stepIndex] = active;
        layer->velocities[stepIndex] = juce::jlimit(0.0f, 1.0f, velocity);
        layer->pitches[stepIndex] = juce::jlimit(0, 127, pitch);
    }
}

void PolyrhythmEngine::clearLayer(int layerIndex)
{
    auto* layer = getLayer(layerIndex);
    if (layer)
    {
        std::fill(layer->pattern.begin(), layer->pattern.end(), false);
        std::fill(layer->velocities.begin(), layer->velocities.end(), 0.8f);
        std::fill(layer->pitches.begin(), layer->pitches.end(), 60);
    }
}

void PolyrhythmEngine::randomizeLayer(int layerIndex, float density)
{
    auto* layer = getLayer(layerIndex);
    if (!layer)
        return;

    density = juce::jlimit(0.0f, 1.0f, density);

    for (int i = 0; i < layer->length; ++i)
    {
        layer->pattern[i] = random.nextFloat() < density;

        if (layer->pattern[i])
        {
            layer->velocities[i] = 0.5f + random.nextFloat() * 0.5f; // 0.5 - 1.0
            layer->pitches[i] = 36 + random.nextInt(49); // C2 - C6
        }
    }
}

void PolyrhythmEngine::advance(int layerIndex, int subdivisions)
{
    auto* layer = getLayer(layerIndex);
    if (!layer || !layer->enabled)
        return;

    // Calculate effective subdivisions based on division
    int effectiveSubdivisions = (subdivisions * layer->division) / (timeSignatureDenom / 4);

    layer->currentStep = (layer->currentStep + effectiveSubdivisions) % layer->length;
}

void PolyrhythmEngine::reset()
{
    for (auto& layer : layers)
    {
        layer.currentStep = static_cast<int>(layer.phase * layer.length);
    }
}

void PolyrhythmEngine::resetLayer(int layerIndex)
{
    auto* layer = getLayer(layerIndex);
    if (layer)
        layer->currentStep = static_cast<int>(layer->phase * layer->length);
}

void PolyrhythmEngine::setTimeSignature(int numerator, int denominator)
{
    timeSignatureNum = juce::jlimit(1, 32, numerator);
    timeSignatureDenom = juce::jlimit(1, 32, denominator);
}

void PolyrhythmEngine::setTempo(double bpm)
{
    tempo = juce::jlimit(20.0, 400.0, bpm);
}
