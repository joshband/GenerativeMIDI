#include "PresetManager.h"

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvts)
    : valueTreeState(apvts), currentPresetIndex(-1)
{
    // Don't initialize factory presets yet - causes issues with ValueTree structure
    // initializeFactoryPresets();
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
    xml.setAttribute("version", "1.0");

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
        return presets[currentPresetIndex].name;
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
    xml.setAttribute("version", "1.0");

    auto stateXml = preset.state.createXml();
    if (stateXml)
        xml.addChildElement(stateXml.release());

    return xml.writeTo(destinationFile);
}

bool PresetManager::importPreset(const juce::File& presetFile)
{
    if (!presetFile.existsAsFile())
        return false;

    auto xml = juce::XmlDocument::parse(presetFile);
    if (!xml || xml->getTagName() != "GenerativeMIDIPreset")
        return false;

    juce::String name = xml->getStringAttribute("name");
    juce::String author = xml->getStringAttribute("author");
    juce::String category = xml->getStringAttribute("category");
    juce::String description = xml->getStringAttribute("description");

    auto stateXml = xml->getChildByName(valueTreeState.state.getType());
    if (!stateXml)
        return false;

    juce::ValueTree state = juce::ValueTree::fromXml(*stateXml);

    Preset preset(name, author, category, description, state, false);
    presets.add(preset);

    // Copy to user preset directory
    juce::File destFile = getPresetDirectory().getChildFile(name + ".gmpreset");
    return presetFile.copyFileTo(destFile);
}

juce::ValueTree PresetManager::captureCurrentState() const
{
    return valueTreeState.copyState();
}

void PresetManager::restoreState(const juce::ValueTree& state)
{
    valueTreeState.replaceState(state);
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

PresetManager::Preset PresetManager::createEuclideanBasicPreset()
{
    // Create a simple value tree with parameter values
    juce::ValueTree state(valueTreeState.state.getType());

    // Copy parameter structure from APVTS
    for (auto* param : valueTreeState.processor.getParameters())
    {
        if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param))
        {
            juce::ValueTree paramTree("PARAM");
            paramTree.setProperty("id", rangedParam->paramID, nullptr);
            paramTree.setProperty("value", rangedParam->getDefaultValue(), nullptr);
            state.appendChild(paramTree, nullptr);
        }
    }

    // Now set specific values for this preset
    state.getChildWithProperty("id", "generatorType").setProperty("value", 0.0f, nullptr);  // Euclidean
    state.getChildWithProperty("id", "tempo").setProperty("value", 0.5f, nullptr);  // 120 BPM (normalized)
    state.getChildWithProperty("id", "euclideanSteps").setProperty("value", 0.238f, nullptr);  // 16 steps (16/64 = 0.25)
    state.getChildWithProperty("id", "euclideanPulses").setProperty("value", 0.0625f, nullptr);  // 4 pulses (4/64 ≈ 0.0625)
    state.getChildWithProperty("id", "euclideanRotation").setProperty("value", 0.0f, nullptr);  // 0 rotation
    state.getChildWithProperty("id", "velocityMin").setProperty("value", 0.63f, nullptr);  // ~80/127
    state.getChildWithProperty("id", "velocityMax").setProperty("value", 0.79f, nullptr);  // ~100/127
    state.getChildWithProperty("id", "pitchMin").setProperty("value", 0.472f, nullptr);  // 60/127
    state.getChildWithProperty("id", "pitchMax").setProperty("value", 0.567f, nullptr);  // 72/127
    state.getChildWithProperty("id", "gateLength").setProperty("value", 0.8f, nullptr);
    state.getChildWithProperty("id", "legatoMode").setProperty("value", 0.0f, nullptr);

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
    juce::ValueTree state = captureCurrentState();

    state.setProperty("generatorType", 0, nullptr);
    state.setProperty("tempo", 140.0f, nullptr);
    state.setProperty("steps", 23, nullptr);
    state.setProperty("pulses", 7, nullptr);
    state.setProperty("rotation", 3, nullptr);
    state.setProperty("velocityMin", 60, nullptr);
    state.setProperty("velocityMax", 110, nullptr);
    state.setProperty("pitchMin", 48, nullptr);
    state.setProperty("pitchMax", 84, nullptr);
    state.setProperty("gateLength", 0.5f, nullptr);

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
    juce::ValueTree state = captureCurrentState();

    state.setProperty("generatorType", 1, nullptr);  // Polyrhythm
    state.setProperty("tempo", 130.0f, nullptr);
    state.setProperty("velocityMin", 70, nullptr);
    state.setProperty("velocityMax", 105, nullptr);
    state.setProperty("pitchMin", 36, nullptr);
    state.setProperty("pitchMax", 96, nullptr);

    return Preset(
        "Polyrhythm Layers",
        "GenerativeMIDI",
        "Polyrhythm",
        "Multi-layered polyrhythmic sequencing for complex rhythmic textures",
        state,
        true
    );
}

PresetManager::Preset PresetManager::createMarkovMelodyPreset()
{
    juce::ValueTree state = captureCurrentState();

    state.setProperty("generatorType", 2, nullptr);  // Algorithmic
    state.setProperty("tempo", 95.0f, nullptr);
    state.setProperty("density", 0.65f, nullptr);
    state.setProperty("velocityMin", 75, nullptr);
    state.setProperty("velocityMax", 95, nullptr);
    state.setProperty("pitchMin", 60, nullptr);
    state.setProperty("pitchMax", 84, nullptr);
    state.setProperty("gateLength", 0.7f, nullptr);

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
    juce::ValueTree state = captureCurrentState();

    state.setProperty("generatorType", 2, nullptr);
    state.setProperty("tempo", 110.0f, nullptr);
    state.setProperty("density", 0.5f, nullptr);
    state.setProperty("velocityMin", 65, nullptr);
    state.setProperty("velocityMax", 90, nullptr);
    state.setProperty("pitchMin", 48, nullptr);
    state.setProperty("pitchMax", 96, nullptr);

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
    juce::ValueTree state = captureCurrentState();

    state.setProperty("generatorType", 2, nullptr);
    state.setProperty("tempo", 125.0f, nullptr);
    state.setProperty("density", 0.45f, nullptr);
    state.setProperty("velocityMin", 70, nullptr);
    state.setProperty("velocityMax", 100, nullptr);
    state.setProperty("pitchMin", 36, nullptr);
    state.setProperty("pitchMax", 108, nullptr);

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
    juce::ValueTree state = captureCurrentState();

    state.setProperty("generatorType", 2, nullptr);
    state.setProperty("tempo", 100.0f, nullptr);
    state.setProperty("density", 0.35f, nullptr);
    state.setProperty("velocityMin", 50, nullptr);
    state.setProperty("velocityMax", 80, nullptr);
    state.setProperty("pitchMin", 60, nullptr);
    state.setProperty("pitchMax", 96, nullptr);
    state.setProperty("gateLength", 0.9f, nullptr);

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
    juce::ValueTree state = captureCurrentState();

    state.setProperty("generatorType", 0, nullptr);
    state.setProperty("tempo", 128.0f, nullptr);
    state.setProperty("steps", 16, nullptr);
    state.setProperty("pulses", 6, nullptr);
    state.setProperty("rotation", 2, nullptr);
    state.setProperty("velocityMin", 85, nullptr);
    state.setProperty("velocityMax", 115, nullptr);
    state.setProperty("ratchetCount", 4, nullptr);
    state.setProperty("ratchetProbability", 0.4f, nullptr);
    state.setProperty("ratchetDecay", 0.6f, nullptr);
    state.setProperty("gateLength", 0.3f, nullptr);

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
    juce::ValueTree state = captureCurrentState();

    state.setProperty("generatorType", 2, nullptr);
    state.setProperty("tempo", 60.0f, nullptr);
    state.setProperty("density", 0.2f, nullptr);
    state.setProperty("velocityMin", 40, nullptr);
    state.setProperty("velocityMax", 65, nullptr);
    state.setProperty("pitchMin", 48, nullptr);
    state.setProperty("pitchMax", 84, nullptr);
    state.setProperty("gateLength", 1.5f, nullptr);
    state.setProperty("legatoMode", true, nullptr);

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
    juce::ValueTree state = captureCurrentState();

    state.setProperty("generatorType", 0, nullptr);
    state.setProperty("tempo", 145.0f, nullptr);
    state.setProperty("steps", 32, nullptr);
    state.setProperty("pulses", 11, nullptr);
    state.setProperty("rotation", 0, nullptr);
    state.setProperty("velocityMin", 95, nullptr);
    state.setProperty("velocityMax", 127, nullptr);
    state.setProperty("pitchMin", 36, nullptr);
    state.setProperty("pitchMax", 48, nullptr);
    state.setProperty("gateLength", 0.1f, nullptr);
    state.setProperty("ratchetCount", 2, nullptr);
    state.setProperty("ratchetProbability", 0.25f, nullptr);

    return Preset(
        "Percussive Hits",
        "GenerativeMIDI",
        "Rhythmic",
        "Short, punchy notes with occasional ratchets for percussion programming",
        state,
        true
    );
}
