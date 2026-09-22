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

    // Seed an audible sparse default so selecting Polyrhythm / adding a layer
    // produces MIDI without requiring step editing (MVP). Pattern editing UI TBD.
    const int layerIndex = static_cast<int>(layers.size());
    layer.division = juce::jlimit(1, 32, 3 + layerIndex); // distinct divisions per layer
    for (int i = 0; i < layer.length; ++i)
    {
        const bool hit = (i % juce::jmax(2, layer.division) == 0);
        layer.pattern[static_cast<size_t>(i)] = hit;
        layer.velocities[static_cast<size_t>(i)] = 0.8f;
        layer.pitches[static_cast<size_t>(i)] = 48 + (layerIndex * 7) + (i % 12);
    }

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

void PolyrhythmEngine::setLayerPitchOffset(int layerIndex, int semitones)
{
    auto* layer = getLayer(layerIndex);
    if (layer)
        layer->pitchOffset = juce::jlimit(-24, 24, semitones);
}

void PolyrhythmEngine::setLayerVelocityMultiplier(int layerIndex, float multiplier)
{
    auto* layer = getLayer(layerIndex);
    if (layer)
        layer->velocityMultiplier = juce::jlimit(0.0f, 2.0f, multiplier);
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
    if (shouldEmitOnThisTick(layerIndex, juce::jmax(1, subdivisions)))
        advanceStep(layerIndex);
}

bool PolyrhythmEngine::shouldEmitOnThisTick(int layerIndex, int clockSubdivision)
{
    auto* layer = getLayer(layerIndex);
    if (!layer || !layer->enabled || layer->length <= 0)
        return false;

    const int grid = juce::jmax(1, clockSubdivision);
    const int div = juce::jmax(1, layer->division);
    const int ticksPerStep = juce::jmax(1, grid / div);

    layer->tickCounter += 1;
    if (layer->tickCounter < ticksPerStep)
        return false;

    layer->tickCounter = 0;
    return true;
}

void PolyrhythmEngine::advanceStep(int layerIndex)
{
    auto* layer = getLayer(layerIndex);
    if (!layer || layer->length <= 0)
        return;

    layer->currentStep = (layer->currentStep + 1) % layer->length;
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
