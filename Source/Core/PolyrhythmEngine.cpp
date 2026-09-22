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

namespace
{
    constexpr int kMaxPersistedLayers = 32;

    juce::String boolVectorToCsv(const std::vector<bool>& values)
    {
        juce::String out;
        for (size_t i = 0; i < values.size(); ++i)
        {
            if (i > 0)
                out << ',';
            out << (values[i] ? '1' : '0');
        }
        return out;
    }

    juce::String floatVectorToCsv(const std::vector<float>& values)
    {
        juce::String out;
        for (size_t i = 0; i < values.size(); ++i)
        {
            if (i > 0)
                out << ',';
            out << values[i];
        }
        return out;
    }

    juce::String intVectorToCsv(const std::vector<int>& values)
    {
        juce::String out;
        for (size_t i = 0; i < values.size(); ++i)
        {
            if (i > 0)
                out << ',';
            out << values[i];
        }
        return out;
    }

    void parseBoolCsv(const juce::String& csv, std::vector<bool>& out, int expectedLength)
    {
        out.assign(static_cast<size_t>(expectedLength), false);
        if (csv.isEmpty() || expectedLength <= 0)
            return;

        int index = 0;
        juce::StringArray tokens;
        tokens.addTokens(csv, ",", "");
        for (const auto& token : tokens)
        {
            if (index >= expectedLength)
                break;
            out[static_cast<size_t>(index)] = (token.getIntValue() != 0);
            ++index;
        }
    }

    void parseFloatCsv(const juce::String& csv, std::vector<float>& out, int expectedLength, float fill)
    {
        out.assign(static_cast<size_t>(expectedLength), fill);
        if (csv.isEmpty() || expectedLength <= 0)
            return;

        int index = 0;
        juce::StringArray tokens;
        tokens.addTokens(csv, ",", "");
        for (const auto& token : tokens)
        {
            if (index >= expectedLength)
                break;
            out[static_cast<size_t>(index)] = juce::jlimit(0.0f, 1.0f, token.getFloatValue());
            ++index;
        }
    }

    void parseIntCsv(const juce::String& csv, std::vector<int>& out, int expectedLength, int fill)
    {
        out.assign(static_cast<size_t>(expectedLength), fill);
        if (csv.isEmpty() || expectedLength <= 0)
            return;

        int index = 0;
        juce::StringArray tokens;
        tokens.addTokens(csv, ",", "");
        for (const auto& token : tokens)
        {
            if (index >= expectedLength)
                break;
            out[static_cast<size_t>(index)] = juce::jlimit(0, 127, token.getIntValue());
            ++index;
        }
    }
}

juce::ValueTree PolyrhythmEngine::toValueTree() const
{
    juce::ValueTree root(kStateTreeType);

    for (const auto& layer : layers)
    {
        juce::ValueTree node("Layer");
        node.setProperty("division", layer.division, nullptr);
        node.setProperty("length", layer.length, nullptr);
        node.setProperty("phase", layer.phase, nullptr);
        node.setProperty("enabled", layer.enabled, nullptr);
        node.setProperty("pitchOffset", layer.pitchOffset, nullptr);
        node.setProperty("velocityMultiplier", layer.velocityMultiplier, nullptr);
        node.setProperty("pattern", boolVectorToCsv(layer.pattern), nullptr);
        node.setProperty("velocities", floatVectorToCsv(layer.velocities), nullptr);
        node.setProperty("pitches", intVectorToCsv(layer.pitches), nullptr);
        root.appendChild(node, nullptr);
    }

    return root;
}

void PolyrhythmEngine::loadFromValueTree(const juce::ValueTree& tree)
{
    if (!tree.hasType(kStateTreeType))
        return;

    std::vector<PolyrhythmLayer> loaded;
    const int numChildren = juce::jmin(tree.getNumChildren(), kMaxPersistedLayers);

    for (int i = 0; i < numChildren; ++i)
    {
        const auto node = tree.getChild(i);
        if (!node.hasType("Layer"))
            continue;

        PolyrhythmLayer layer;
        const int length = juce::jlimit(1, 128, static_cast<int>(node.getProperty("length", 16)));
        layer.resize(length);
        layer.division = juce::jlimit(1, 64, static_cast<int>(node.getProperty("division", 4)));
        layer.phase = juce::jlimit(0.0f, 1.0f, static_cast<float>(node.getProperty("phase", 0.0f)));
        layer.enabled = static_cast<bool>(node.getProperty("enabled", true));
        layer.pitchOffset = juce::jlimit(-24, 24, static_cast<int>(node.getProperty("pitchOffset", 0)));
        layer.velocityMultiplier = juce::jlimit(
            0.0f, 2.0f, static_cast<float>(node.getProperty("velocityMultiplier", 1.0f)));

        parseBoolCsv(node.getProperty("pattern").toString(), layer.pattern, length);
        parseFloatCsv(node.getProperty("velocities").toString(), layer.velocities, length, 0.8f);
        parseIntCsv(node.getProperty("pitches").toString(), layer.pitches, length, 60);

        layer.currentStep = static_cast<int>(layer.phase * static_cast<float>(layer.length));
        layer.tickCounter = 0;
        loaded.push_back(std::move(layer));
    }

    if (loaded.empty())
        return;

    layers = std::move(loaded);
}
