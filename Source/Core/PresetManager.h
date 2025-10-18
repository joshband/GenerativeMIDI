#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>

/**
 * @class PresetManager
 * @brief Manages saving, loading, and organizing presets for the GenerativeMIDI plugin
 *
 * Features:
 * - Save/load user presets to/from disk
 * - Factory presets embedded in plugin
 * - Preset browsing and categorization
 * - Automatic parameter state capture/restore
 */
class PresetManager
{
public:
    /**
     * @struct Preset
     * @brief Individual preset with metadata and parameter state
     */
    struct Preset
    {
        juce::String name;
        juce::String author;
        juce::String category;      // "Euclidean", "Polyrhythm", "Experimental", etc.
        juce::String description;
        juce::ValueTree state;      // Complete parameter state
        bool isFactory;             // True for built-in presets, false for user presets

        Preset() : isFactory(false) {}

        Preset(const juce::String& presetName,
               const juce::String& presetAuthor,
               const juce::String& presetCategory,
               const juce::String& presetDescription,
               const juce::ValueTree& presetState,
               bool factory = false)
            : name(presetName),
              author(presetAuthor),
              category(presetCategory),
              description(presetDescription),
              state(presetState),
              isFactory(factory)
        {
        }
    };

    PresetManager(juce::AudioProcessorValueTreeState& apvts);
    ~PresetManager() = default;

    // Preset Management
    void savePreset(const juce::String& name,
                   const juce::String& author,
                   const juce::String& category,
                   const juce::String& description);

    bool loadPreset(int presetIndex);
    bool loadPresetByName(const juce::String& name);
    void deletePreset(int presetIndex);

    // Preset Access
    int getNumPresets() const { return static_cast<int>(presets.size()); }
    const Preset& getPreset(int index) const { return presets.getReference(index); }
    juce::StringArray getPresetNames() const;
    juce::StringArray getCategories() const;
    juce::Array<Preset> getPresetsInCategory(const juce::String& category) const;

    int getCurrentPresetIndex() const { return currentPresetIndex; }
    const juce::String& getCurrentPresetName() const;

    // Preset Navigation
    void loadNextPreset();
    void loadPreviousPreset();

    // Factory Presets
    void initializeFactoryPresets();

    // File I/O
    juce::File getPresetDirectory() const;
    void scanUserPresets();
    bool exportPreset(int presetIndex, const juce::File& destinationFile);
    bool importPreset(const juce::File& presetFile);

private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::Array<Preset> presets;
    int currentPresetIndex;

    // Helper functions
    juce::ValueTree captureCurrentState() const;
    void restoreState(const juce::ValueTree& state);
    juce::File getDefaultPresetDirectory() const;

    // Factory preset creation helpers
    Preset createEuclideanBasicPreset();
    Preset createEuclideanComplexPreset();
    Preset createPolyrhythmPreset();
    Preset createMarkovMelodyPreset();
    Preset createLSystemPreset();
    Preset createCellularPreset();
    Preset createProbabilisticPreset();
    Preset createRatchetGroovePreset();
    Preset createAmbientPreset();
    Preset createPercussivePreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};
