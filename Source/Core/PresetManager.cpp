#include "PresetManager.h"
#include "GeneratorTypeMapping.h"

namespace
{
    constexpr juce::int64 kMaxPresetFileBytes = 1024 * 1024; // 1 MiB

    bool isSafePresetFileName(const juce::String& name)
    {
        if (name.isEmpty())
            return false;

        // Reject path traversal and directory separators in names used for disk I/O.
        if (name.contains("..") || name.containsChar('/') || name.containsChar('\\'))
            return false;

        return true;
    }
}

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvts, PolyrhythmEngine& polyEngine)
    : valueTreeState(apvts), polyrhythmEngine(polyEngine), currentPresetIndex(-1)
{
    initializeFactoryPresets();
    scanUserPresets();
}

void PresetManager::savePreset(const juce::String& name,
                               const juce::String& author,
                               const juce::String& category,
                               const juce::String& description)
{
    // Capture current parameter state
    juce::ValueTree state = captureCurrentState();

    // Create preset
    Preset preset(name, author, category, description, state, false);

    // Check if preset with this name already exists
    int existingIndex = -1;
    for (int i = 0; i < presets.size(); ++i)
    {
        if (presets[i].name == name && !presets[i].isFactory)
        {
            existingIndex = i;
            break;
        }
    }

    if (existingIndex >= 0)
    {
        // Update existing preset
        presets.set(existingIndex, preset);
        currentPresetIndex = existingIndex;
    }
    else
    {
        // Add new preset
        presets.add(preset);
        currentPresetIndex = presets.size() - 1;
    }

    // Save to disk
    juce::File presetFile = getPresetDirectory().getChildFile(name + ".gmpreset");

    // Create XML from ValueTree
    juce::XmlElement xml("GenerativeMIDIPreset");
    xml.setAttribute("name", name);
    xml.setAttribute("author", author);
    xml.setAttribute("category", category);
    xml.setAttribute("description", description);
    xml.setAttribute("version", GeneratorTypeMapping::kPresetSchemaVersion);

    // Add parameter state
    auto stateXml = state.createXml();
    if (stateXml)
        xml.addChildElement(stateXml.release());

    // Write to file
    xml.writeTo(presetFile);
}

bool PresetManager::loadPreset(int presetIndex)
{
    if (presetIndex < 0 || presetIndex >= presets.size())
        return false;

    const auto& preset = presets[presetIndex];
    restoreState(preset.state);
    currentPresetIndex = presetIndex;

    return true;
}

bool PresetManager::loadPresetByName(const juce::String& name)
{
    for (int i = 0; i < presets.size(); ++i)
    {
        if (presets[i].name == name)
            return loadPreset(i);
    }
    return false;
}

void PresetManager::deletePreset(int presetIndex)
{
    if (presetIndex < 0 || presetIndex >= presets.size())
        return;

    const auto& preset = presets[presetIndex];

    // Can't delete factory presets
    if (preset.isFactory)
        return;

    // Delete file
    juce::File presetFile = getPresetDirectory().getChildFile(preset.name + ".gmpreset");
    if (presetFile.existsAsFile())
        presetFile.deleteFile();

    // Remove from array
    presets.remove(presetIndex);

    // Update current index
    if (currentPresetIndex == presetIndex)
        currentPresetIndex = -1;
    else if (currentPresetIndex > presetIndex)
        currentPresetIndex--;
}

juce::StringArray PresetManager::getPresetNames() const
{
    juce::StringArray names;
    for (const auto& preset : presets)
        names.add(preset.name);
    return names;
}

juce::StringArray PresetManager::getCategories() const
{
    juce::StringArray categories;
    for (const auto& preset : presets)
    {
        if (!categories.contains(preset.category))
            categories.add(preset.category);
    }
    return categories;
}

juce::Array<PresetManager::Preset> PresetManager::getPresetsInCategory(const juce::String& category) const
{
    juce::Array<Preset> categoryPresets;
    for (const auto& preset : presets)
    {
        if (preset.category == category)
            categoryPresets.add(preset);
    }
    return categoryPresets;
}

const juce::String& PresetManager::getCurrentPresetName() const
{
    static juce::String emptyString;
    if (currentPresetIndex >= 0 && currentPresetIndex < presets.size())
        return presets.getReference(currentPresetIndex).name;
    return emptyString;
}

void PresetManager::loadNextPreset()
{
    if (presets.isEmpty())
        return;

    int nextIndex = (currentPresetIndex + 1) % presets.size();
    loadPreset(nextIndex);
}

void PresetManager::loadPreviousPreset()
{
    if (presets.isEmpty())
        return;

    int prevIndex = currentPresetIndex - 1;
    if (prevIndex < 0)
        prevIndex = presets.size() - 1;
    loadPreset(prevIndex);
}

void PresetManager::initializeFactoryPresets()
{
    presets.add(createEuclideanBasicPreset());
    presets.add(createEuclideanComplexPreset());
    presets.add(createPolyrhythmPreset());
    presets.add(createBrownianPreset());
    presets.add(createMarkovMelodyPreset());
    presets.add(createLSystemPreset());
    presets.add(createCellularPreset());
    presets.add(createProbabilisticPreset());
    presets.add(createRatchetGroovePreset());
    presets.add(createAmbientPreset());
    presets.add(createPercussivePreset());
}

juce::File PresetManager::getPresetDirectory() const
{
    return getDefaultPresetDirectory();
}

void PresetManager::scanUserPresets()
{
    juce::File presetDir = getPresetDirectory();

    if (!presetDir.exists())
        presetDir.createDirectory();

    juce::Array<juce::File> presetFiles = presetDir.findChildFiles(juce::File::findFiles, false, "*.gmpreset");

    for (const auto& file : presetFiles)
    {
        auto xml = juce::XmlDocument::parse(file);
        if (!xml)
            continue;

        if (xml->getTagName() != "GenerativeMIDIPreset")
            continue;

        juce::String name = xml->getStringAttribute("name");
        juce::String author = xml->getStringAttribute("author");
        juce::String category = xml->getStringAttribute("category");
        juce::String description = xml->getStringAttribute("description");

        auto stateXml = xml->getChildByName(valueTreeState.state.getType());
        if (!stateXml)
            continue;

        juce::ValueTree state = juce::ValueTree::fromXml(*stateXml);
        GeneratorTypeMapping::migrateApvtsStateIfNeeded(state, xml->getStringAttribute("version"));

        Preset preset(name, author, category, description, state, false);
        presets.add(preset);
    }
}

bool PresetManager::exportPreset(int presetIndex, const juce::File& destinationFile)
{
    if (presetIndex < 0 || presetIndex >= presets.size())
        return false;

    const auto& preset = presets[presetIndex];

    juce::XmlElement xml("GenerativeMIDIPreset");
    xml.setAttribute("name", preset.name);
    xml.setAttribute("author", preset.author);
    xml.setAttribute("category", preset.category);
    xml.setAttribute("description", preset.description);
    xml.setAttribute("version", GeneratorTypeMapping::kPresetSchemaVersion);

    auto stateXml = preset.state.createXml();
    if (stateXml)
        xml.addChildElement(stateXml.release());

    return xml.writeTo(destinationFile);
}

bool PresetManager::importPreset(const juce::File& presetFile)
{
    if (!presetFile.existsAsFile())
        return false;

    if (presetFile.getSize() <= 0 || presetFile.getSize() > kMaxPresetFileBytes)
        return false;

    auto xml = juce::XmlDocument::parse(presetFile);
    if (xml == nullptr || xml->getTagName() != "GenerativeMIDIPreset")
        return false;

    juce::String name = xml->getStringAttribute("name");
    if (!isSafePresetFileName(name))
        return false;

    juce::String author = xml->getStringAttribute("author");
    juce::String category = xml->getStringAttribute("category");
    juce::String description = xml->getStringAttribute("description");

    auto stateXml = xml->getChildByName(valueTreeState.state.getType());
    if (stateXml == nullptr)
        return false;

    juce::ValueTree state = juce::ValueTree::fromXml(*stateXml);
    if (!state.isValid())
        return false;

    const auto schema = xml->getStringAttribute("version");
    GeneratorTypeMapping::migrateApvtsStateIfNeeded(state, schema);

    Preset preset(name, author, category, description, state, false);
    presets.add(preset);

    // Copy to user preset directory using the sanitized base name only.
    juce::File destFile = getPresetDirectory().getChildFile(name + ".gmpreset");
    return presetFile.copyFileTo(destFile);
}

juce::ValueTree PresetManager::captureCurrentState() const
{
    auto state = valueTreeState.copyState();

    auto existing = state.getChildWithName(PolyrhythmEngine::kStateTreeType);
    if (existing.isValid())
        state.removeChild(existing, nullptr);
    state.appendChild(polyrhythmEngine.toValueTree(), nullptr);

    return state;
}

void PresetManager::restoreState(const juce::ValueTree& state)
{
    auto tree = state.createCopy();

    auto layersNode = tree.getChildWithName(PolyrhythmEngine::kStateTreeType);
    juce::ValueTree layersCopy;
    if (layersNode.isValid())
    {
        layersCopy = layersNode.createCopy();
        tree.removeChild(layersNode, nullptr);
    }

    valueTreeState.replaceState(tree);

    if (layersCopy.isValid())
        polyrhythmEngine.loadFromValueTree(layersCopy);
}

juce::File PresetManager::getDefaultPresetDirectory() const
{
    juce::File appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);

#if JUCE_MAC
    return appDataDir.getChildFile("Application Support/GenerativeMIDI/Presets");
#elif JUCE_WINDOWS
    return appDataDir.getChildFile("GenerativeMIDI/Presets");
#else
    return appDataDir.getChildFile(".GenerativeMIDI/Presets");
#endif
}

// Factory Preset Implementations
// Generator indices: 0 Euclidean, 1 Polyrhythm, 2–5 algorithmic, 6–9 stochastic
// (see GeneratorTypeMapping.h).
// JUCE 8 APVTS stores denormalised (actual) values in PARAM children — not normalised [0,1].

juce::ValueTree PresetManager::makeFactoryParamTree() const
{
    // copyState() flushes current denormalised defaults into a tree replaceState accepts.
    return valueTreeState.copyState();
}

void PresetManager::setFactoryParam(juce::ValueTree& state, const juce::String& id, float denormalisedValue) const
{
    auto child = state.getChildWithProperty("id", id);
    if (child.isValid())
        child.setProperty("value", denormalisedValue, nullptr);
}

PresetManager::Preset PresetManager::createEuclideanBasicPreset()
{
    auto state = makeFactoryParamTree();
    setFactoryParam(state, "generatorType", 0.0f);
    setFactoryParam(state, "tempo", 120.0f);
    setFactoryParam(state, "euclideanSteps", 16.0f);
    setFactoryParam(state, "euclideanPulses", 4.0f);
    setFactoryParam(state, "euclideanRotation", 0.0f);
    setFactoryParam(state, "velocityMin", 0.63f);
    setFactoryParam(state, "velocityMax", 0.79f);
    setFactoryParam(state, "pitchMin", 60.0f);
    setFactoryParam(state, "pitchMax", 72.0f);
    setFactoryParam(state, "gateLength", 0.8f);
    setFactoryParam(state, "legatoMode", 0.0f);

    return Preset(
        "Euclidean Basic",
        "GenerativeMIDI",
        "Euclidean",
        "Simple 4-on-16 Euclidean rhythm, perfect for kick drums or basslines",
        state,
        true
    );
}

PresetManager::Preset PresetManager::createEuclideanComplexPreset()
{
    auto state = makeFactoryParamTree();
    setFactoryParam(state, "generatorType", 0.0f);
    setFactoryParam(state, "tempo", 140.0f);
    setFactoryParam(state, "euclideanSteps", 23.0f);
    setFactoryParam(state, "euclideanPulses", 7.0f);
    setFactoryParam(state, "euclideanRotation", 3.0f);
    setFactoryParam(state, "velocityMin", 60.0f / 127.0f);
    setFactoryParam(state, "velocityMax", 110.0f / 127.0f);
    setFactoryParam(state, "pitchMin", 48.0f);
    setFactoryParam(state, "pitchMax", 84.0f);
    setFactoryParam(state, "gateLength", 0.5f);

    return Preset(
        "Euclidean Complex",
        "GenerativeMIDI",
        "Euclidean",
        "Complex 7-on-23 pattern with rotation, creates shifting polyrhythmic feel",
        state,
        true
    );
}

PresetManager::Preset PresetManager::createPolyrhythmPreset()
{
    auto state = makeFactoryParamTree();
    setFactoryParam(state, "generatorType", 1.0f); // Polyrhythm
    setFactoryParam(state, "tempo", 120.0f);
    setFactoryParam(state, "noteDensity", 0.85f);
    setFactoryParam(state, "velocityMin", 70.0f / 127.0f);
    setFactoryParam(state, "velocityMax", 110.0f / 127.0f);
    setFactoryParam(state, "pitchMin", 48.0f);
    setFactoryParam(state, "pitchMax", 72.0f);
    setFactoryParam(state, "gateLength", 0.6f);

    // Snapshot default seeded layers so factory load restores a known pattern set.
    auto existing = state.getChildWithName(PolyrhythmEngine::kStateTreeType);
    if (existing.isValid())
        state.removeChild(existing, nullptr);
    state.appendChild(polyrhythmEngine.toValueTree(), nullptr);

    return Preset(
        "Polyrhythm Layers",
        "GenerativeMIDI",
        "Polyrhythm",
        "Experimental multi-layer polyrhythm with default seeded patterns",
        state,
        true
    );
}

PresetManager::Preset PresetManager::createBrownianPreset()
{
    auto state = makeFactoryParamTree();
    setFactoryParam(state, "generatorType", 6.0f); // Brownian
    setFactoryParam(state, "tempo", 130.0f);
    setFactoryParam(state, "noteDensity", 0.55f);
    setFactoryParam(state, "stepSize", 0.15f);
    setFactoryParam(state, "momentum", 0.85f);
    setFactoryParam(state, "velocityMin", 70.0f / 127.0f);
    setFactoryParam(state, "velocityMax", 105.0f / 127.0f);
    setFactoryParam(state, "pitchMin", 36.0f);
    setFactoryParam(state, "pitchMax", 96.0f);

    return Preset(
        "Brownian Drift",
        "GenerativeMIDI",
        "Stochastic",
        "Brownian motion pitch walks for restless stochastic textures",
        state,
        true
    );
}

PresetManager::Preset PresetManager::createMarkovMelodyPreset()
{
    auto state = makeFactoryParamTree();
    setFactoryParam(state, "generatorType", 2.0f); // Markov
    setFactoryParam(state, "tempo", 95.0f);
    setFactoryParam(state, "noteDensity", 0.65f);
    setFactoryParam(state, "velocityMin", 75.0f / 127.0f);
    setFactoryParam(state, "velocityMax", 95.0f / 127.0f);
    setFactoryParam(state, "pitchMin", 60.0f);
    setFactoryParam(state, "pitchMax", 84.0f);
    setFactoryParam(state, "gateLength", 0.7f);

    return Preset(
        "Markov Melody",
        "GenerativeMIDI",
        "Algorithmic",
        "Markov Chain melodic generation with musical phrase structure",
        state,
        true
    );
}

PresetManager::Preset PresetManager::createLSystemPreset()
{
    auto state = makeFactoryParamTree();
    setFactoryParam(state, "generatorType", 3.0f); // L-System
    setFactoryParam(state, "tempo", 110.0f);
    setFactoryParam(state, "noteDensity", 0.5f);
    setFactoryParam(state, "velocityMin", 65.0f / 127.0f);
    setFactoryParam(state, "velocityMax", 90.0f / 127.0f);
    setFactoryParam(state, "pitchMin", 48.0f);
    setFactoryParam(state, "pitchMax", 96.0f);

    return Preset(
        "L-System Fractal",
        "GenerativeMIDI",
        "Algorithmic",
        "Fractal pattern evolution using L-System grammar rules",
        state,
        true
    );
}

PresetManager::Preset PresetManager::createCellularPreset()
{
    auto state = makeFactoryParamTree();
    setFactoryParam(state, "generatorType", 4.0f); // Cellular
    setFactoryParam(state, "tempo", 125.0f);
    setFactoryParam(state, "noteDensity", 0.45f);
    setFactoryParam(state, "velocityMin", 70.0f / 127.0f);
    setFactoryParam(state, "velocityMax", 100.0f / 127.0f);
    setFactoryParam(state, "pitchMin", 36.0f);
    setFactoryParam(state, "pitchMax", 108.0f);

    return Preset(
        "Cellular Automata",
        "GenerativeMIDI",
        "Algorithmic",
        "Emergent patterns from Wolfram cellular automata rules",
        state,
        true
    );
}

PresetManager::Preset PresetManager::createProbabilisticPreset()
{
    auto state = makeFactoryParamTree();
    setFactoryParam(state, "generatorType", 5.0f); // Probabilistic
    setFactoryParam(state, "tempo", 100.0f);
    setFactoryParam(state, "noteDensity", 0.35f);
    setFactoryParam(state, "velocityMin", 50.0f / 127.0f);
    setFactoryParam(state, "velocityMax", 80.0f / 127.0f);
    setFactoryParam(state, "pitchMin", 60.0f);
    setFactoryParam(state, "pitchMax", 96.0f);
    setFactoryParam(state, "gateLength", 0.9f);

    return Preset(
        "Probabilistic Sparse",
        "GenerativeMIDI",
        "Algorithmic",
        "Sparse probabilistic note generation for ambient textures",
        state,
        true
    );
}

PresetManager::Preset PresetManager::createRatchetGroovePreset()
{
    auto state = makeFactoryParamTree();
    setFactoryParam(state, "generatorType", 0.0f);
    setFactoryParam(state, "tempo", 128.0f);
    setFactoryParam(state, "euclideanSteps", 16.0f);
    setFactoryParam(state, "euclideanPulses", 6.0f);
    setFactoryParam(state, "euclideanRotation", 2.0f);
    setFactoryParam(state, "velocityMin", 85.0f / 127.0f);
    setFactoryParam(state, "velocityMax", 115.0f / 127.0f);
    setFactoryParam(state, "ratchetCount", 4.0f);
    setFactoryParam(state, "ratchetProbability", 0.4f);
    setFactoryParam(state, "ratchetDecay", 0.6f);
    setFactoryParam(state, "gateLength", 0.3f);

    return Preset(
        "Ratchet Groove",
        "GenerativeMIDI",
        "Rhythmic",
        "Euclidean pattern with ratcheting for glitchy, energetic grooves",
        state,
        true
    );
}

PresetManager::Preset PresetManager::createAmbientPreset()
{
    auto state = makeFactoryParamTree();
    setFactoryParam(state, "generatorType", 5.0f); // Probabilistic sparse
    setFactoryParam(state, "tempo", 60.0f);
    setFactoryParam(state, "noteDensity", 0.2f);
    setFactoryParam(state, "velocityMin", 40.0f / 127.0f);
    setFactoryParam(state, "velocityMax", 65.0f / 127.0f);
    setFactoryParam(state, "pitchMin", 48.0f);
    setFactoryParam(state, "pitchMax", 84.0f);
    setFactoryParam(state, "gateLength", 1.5f);
    setFactoryParam(state, "legatoMode", 1.0f);

    return Preset(
        "Ambient Drift",
        "GenerativeMIDI",
        "Ambient",
        "Slow, sparse, overlapping notes for evolving ambient soundscapes",
        state,
        true
    );
}

PresetManager::Preset PresetManager::createPercussivePreset()
{
    auto state = makeFactoryParamTree();
    setFactoryParam(state, "generatorType", 0.0f);
    setFactoryParam(state, "tempo", 145.0f);
    setFactoryParam(state, "euclideanSteps", 32.0f);
    setFactoryParam(state, "euclideanPulses", 11.0f);
    setFactoryParam(state, "euclideanRotation", 0.0f);
    setFactoryParam(state, "velocityMin", 95.0f / 127.0f);
    setFactoryParam(state, "velocityMax", 1.0f);
    setFactoryParam(state, "pitchMin", 36.0f);
    setFactoryParam(state, "pitchMax", 48.0f);
    setFactoryParam(state, "gateLength", 0.1f);
    setFactoryParam(state, "ratchetCount", 2.0f);
    setFactoryParam(state, "ratchetProbability", 0.25f);

    return Preset(
        "Percussive Hits",
        "GenerativeMIDI",
        "Rhythmic",
        "Short, punchy notes with occasional ratchets for percussion programming",
        state,
        true
    );
}
