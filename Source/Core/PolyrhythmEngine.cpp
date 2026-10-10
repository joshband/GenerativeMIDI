/*
  ==============================================================================
    PolyrhythmEngine.cpp

    Polyrhythmic sequencing implementation

  ==============================================================================
*/

#include "PolyrhythmEngine.h"

#include <algorithm>

PolyrhythmEngine::PolyrhythmEngine()
{
    current.store(new Snapshot(), std::memory_order_release);
    addLayer(); // default layer
}

PolyrhythmEngine::~PolyrhythmEngine()
{
    delete current.load();
}

const PolyrhythmEngine::Snapshot* PolyrhythmEngine::pin() noexcept
{
    // Hazard-pointer protocol (single reader = audio thread): publish what we are about
    // to read, then confirm it is still current. Writers never free a pinned snapshot.
    const Snapshot* p = current.load(std::memory_order_seq_cst);
    for (;;)
    {
        hazard.store(p, std::memory_order_seq_cst);
        const Snapshot* again = current.load(std::memory_order_seq_cst);
        if (again == p)
            return p;
        p = again;
    }
}

void PolyrhythmEngine::publish(std::unique_ptr<Snapshot> next)
{
    // Caller holds writeLock.
    const Snapshot* old = current.exchange(next.release(), std::memory_order_seq_cst);
    retired.emplace_back(old);

    // Free every retired snapshot the audio thread is not currently pinning.
    const Snapshot* pinned = hazard.load(std::memory_order_seq_cst);
    retired.erase(std::remove_if(retired.begin(), retired.end(),
                                 [pinned](const std::unique_ptr<const Snapshot>& s) { return s.get() != pinned; }),
                  retired.end());
}

template <typename Fn>
bool PolyrhythmEngine::mutate(Fn&& fn)
{
    const juce::ScopedLock sl(writeLock);
    auto next = std::make_unique<Snapshot>(*current.load());
    if (!fn(*next))
        return false;
    publish(std::move(next));
    return true;
}

template <typename Fn>
void PolyrhythmEngine::editLayer(int layerIndex, Fn&& fn)
{
    mutate([&](Snapshot& s)
    {
        if (layerIndex < 0 || layerIndex >= static_cast<int>(s.layers.size()))
            return false;
        return fn(s.layers[static_cast<size_t>(layerIndex)]);
    });
}

void PolyrhythmEngine::resetPlayState(int index, const PolyrhythmLayer& layer) noexcept
{
    auto& p = play[static_cast<size_t>(index)];
    p.step.store(static_cast<int>(layer.phase * static_cast<float>(layer.length)), std::memory_order_relaxed);
    p.tick.store(0, std::memory_order_relaxed);
}

void PolyrhythmEngine::applyPendingReset(const Snapshot& snap) noexcept
{
    if (snap.loadId == appliedLoadId)
        return;
    appliedLoadId = snap.loadId;
    for (size_t i = 0; i < snap.layers.size(); ++i)
        resetPlayState(static_cast<int>(i), snap.layers[i]);
}

bool PolyrhythmEngine::tickDue(int index, const PolyrhythmLayer& layer, int clockSubdivision) noexcept
{
    const int grid = juce::jmax(1, clockSubdivision);
    const int div = juce::jmax(1, layer.division);
    const int ticksPerStep = juce::jmax(1, grid / div);

    auto& tick = play[static_cast<size_t>(index)].tick;
    const int t = tick.load(std::memory_order_relaxed) + 1;
    if (t < ticksPerStep)
    {
        tick.store(t, std::memory_order_relaxed);
        return false;
    }
    tick.store(0, std::memory_order_relaxed);
    return true;
}

void PolyrhythmEngine::stepForward(int index, const PolyrhythmLayer& layer) noexcept
{
    auto& step = play[static_cast<size_t>(index)].step;
    step.store((step.load(std::memory_order_relaxed) + 1) % layer.length, std::memory_order_relaxed);
}

int PolyrhythmEngine::addLayer()
{
    int result = -1;
    mutate([this, &result](Snapshot& s)
    {
        if (static_cast<int>(s.layers.size()) >= kMaxLayers)
            return false;

        PolyrhythmLayer layer;
        layer.resize(16);

        // Seed an audible sparse default. The layer editor can toggle these steps;
        // edits stay on the layer and round-trip through toValueTree.
        const int layerIndex = static_cast<int>(s.layers.size());
        layer.division = juce::jlimit(1, 32, 3 + layerIndex); // distinct divisions per layer
        for (int i = 0; i < layer.length; ++i)
        {
            const bool hit = (i % juce::jmax(2, layer.division) == 0);
            layer.pattern[static_cast<size_t>(i)] = hit;
            layer.velocities[static_cast<size_t>(i)] = 0.8f;
            layer.pitches[static_cast<size_t>(i)] = 48 + (layerIndex * 7) + (i % 12);
        }

        // Slot is not visible to the audio thread until this snapshot is published.
        resetPlayState(layerIndex, layer);
        s.layers.push_back(std::move(layer));
        result = layerIndex;
        return true;
    });
    return result;
}

void PolyrhythmEngine::removeLayer(int layerIndex)
{
    mutate([layerIndex](Snapshot& s)
    {
        if (layerIndex < 0 || layerIndex >= static_cast<int>(s.layers.size()))
            return false;
        s.layers.erase(s.layers.begin() + layerIndex);
        ++s.loadId; // indices shifted: restart playback positions
        return true;
    });
}

const PolyrhythmLayer* PolyrhythmEngine::getLayer(int layerIndex) const
{
    const auto* s = current.load(std::memory_order_acquire);
    if (layerIndex >= 0 && layerIndex < static_cast<int>(s->layers.size()))
        return &s->layers[static_cast<size_t>(layerIndex)];
    return nullptr;
}

int PolyrhythmEngine::getNumLayers() const
{
    return static_cast<int>(current.load(std::memory_order_acquire)->layers.size());
}

int PolyrhythmEngine::getCurrentStep(int layerIndex) const
{
    if (layerIndex < 0 || layerIndex >= kMaxLayers)
        return 0;
    return play[static_cast<size_t>(layerIndex)].step.load(std::memory_order_relaxed);
}

void PolyrhythmEngine::ensureLayerAudible(int layerIndex)
{
    editLayer(layerIndex, [](PolyrhythmLayer& layer)
    {
        if (std::find(layer.pattern.begin(), layer.pattern.end(), true) != layer.pattern.end())
            return false;

        for (int i = 0; i < layer.length; ++i)
        {
            layer.pattern[static_cast<size_t>(i)] = (i % 4 == 0);
            layer.velocities[static_cast<size_t>(i)] = 0.8f;
            layer.pitches[static_cast<size_t>(i)] = 60 + (i % 12);
        }
        layer.enabled = true;
        return true;
    });
}

void PolyrhythmEngine::setLayerDivision(int layerIndex, int division)
{
    editLayer(layerIndex, [=](PolyrhythmLayer& l) { l.division = juce::jlimit(1, 64, division); return true; });
}

void PolyrhythmEngine::setLayerLength(int layerIndex, int length)
{
    editLayer(layerIndex, [=](PolyrhythmLayer& l) { l.resize(juce::jlimit(1, 128, length)); return true; });
}

void PolyrhythmEngine::setLayerPhase(int layerIndex, float phase)
{
    editLayer(layerIndex, [=](PolyrhythmLayer& l) { l.phase = juce::jlimit(0.0f, 1.0f, phase); return true; });
}

void PolyrhythmEngine::setLayerEnabled(int layerIndex, bool enabled)
{
    editLayer(layerIndex, [=](PolyrhythmLayer& l) { l.enabled = enabled; return true; });
}

void PolyrhythmEngine::setLayerPitchOffset(int layerIndex, int semitones)
{
    editLayer(layerIndex, [=](PolyrhythmLayer& l) { l.pitchOffset = juce::jlimit(-24, 24, semitones); return true; });
}

void PolyrhythmEngine::setLayerVelocityMultiplier(int layerIndex, float multiplier)
{
    editLayer(layerIndex, [=](PolyrhythmLayer& l) { l.velocityMultiplier = juce::jlimit(0.0f, 2.0f, multiplier); return true; });
}

void PolyrhythmEngine::setStep(int layerIndex, int stepIndex, bool active, float velocity, int pitch)
{
    editLayer(layerIndex, [=](PolyrhythmLayer& l)
    {
        if (stepIndex < 0 || stepIndex >= l.length)
            return false;
        const auto i = static_cast<size_t>(stepIndex);
        l.pattern[i] = active;
        l.velocities[i] = juce::jlimit(0.0f, 1.0f, velocity);
        l.pitches[i] = juce::jlimit(0, 127, pitch);
        return true;
    });
}

void PolyrhythmEngine::clearLayer(int layerIndex)
{
    editLayer(layerIndex, [](PolyrhythmLayer& l)
    {
        std::fill(l.pattern.begin(), l.pattern.end(), false);
        std::fill(l.velocities.begin(), l.velocities.end(), 0.8f);
        std::fill(l.pitches.begin(), l.pitches.end(), 60);
        return true;
    });
}

void PolyrhythmEngine::randomizeLayer(int layerIndex, float density)
{
    density = juce::jlimit(0.0f, 1.0f, density);
    // `random` is only touched while holding writeLock (inside mutate).
    editLayer(layerIndex, [this, density](PolyrhythmLayer& l)
    {
        for (int i = 0; i < l.length; ++i)
        {
            const auto k = static_cast<size_t>(i);
            l.pattern[k] = random.nextFloat() < density;
            if (l.pattern[k])
            {
                l.velocities[k] = 0.5f + random.nextFloat() * 0.5f; // 0.5 - 1.0
                l.pitches[k] = 36 + random.nextInt(49);             // C2 - C6
            }
        }
        return true;
    });
}

void PolyrhythmEngine::advance(int layerIndex, int subdivisions)
{
    if (shouldEmitOnThisTick(layerIndex, juce::jmax(1, subdivisions)))
        advanceStep(layerIndex);
}

bool PolyrhythmEngine::shouldEmitOnThisTick(int layerIndex, int clockSubdivision)
{
    const Snapshot* snap = pin();
    applyPendingReset(*snap);
    bool due = false;
    if (layerIndex >= 0 && layerIndex < static_cast<int>(snap->layers.size()))
    {
        const auto& layer = snap->layers[static_cast<size_t>(layerIndex)];
        due = layer.enabled && layer.length > 0 && tickDue(layerIndex, layer, clockSubdivision);
    }
    unpin();
    return due;
}

void PolyrhythmEngine::advanceStep(int layerIndex)
{
    const Snapshot* snap = pin();
    if (layerIndex >= 0 && layerIndex < static_cast<int>(snap->layers.size()))
    {
        const auto& layer = snap->layers[static_cast<size_t>(layerIndex)];
        if (layer.length > 0)
            stepForward(layerIndex, layer);
    }
    unpin();
}

void PolyrhythmEngine::reset()
{
    const juce::ScopedLock sl(writeLock);
    const auto* s = current.load();
    for (size_t i = 0; i < s->layers.size(); ++i)
        resetPlayState(static_cast<int>(i), s->layers[i]);
}

void PolyrhythmEngine::resetLayer(int layerIndex)
{
    const juce::ScopedLock sl(writeLock);
    const auto* s = current.load();
    if (layerIndex >= 0 && layerIndex < static_cast<int>(s->layers.size()))
        resetPlayState(layerIndex, s->layers[static_cast<size_t>(layerIndex)]);
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

    const juce::ScopedLock sl(writeLock);
    for (const auto& layer : current.load()->layers)
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

        loaded.push_back(std::move(layer));
    }

    if (loaded.empty())
        return;

    mutate([&loaded](Snapshot& s)
    {
        s.layers = std::move(loaded);
        ++s.loadId; // audio thread restarts playback positions at phase on next tick
        return true;
    });
}
