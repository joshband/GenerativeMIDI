/*
  ==============================================================================
    PluginEditor.h

    Custom modern UI for generative MIDI processor

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/AccessibleComboBox.h"
#include "UI/CustomLookAndFeel.h"
#include "UI/PatternVisualizer.h"
#include "UI/PresetBrowser.h"
#include "UI/PolyrhythmLayerEditor.h"
#include "UI/MidiActivityPane.h"

class GenerativeMIDIEditor : public juce::AudioProcessorEditor,
                              private juce::Timer,
                              private PresetManager::Listener
{
public:
    GenerativeMIDIEditor(GenerativeMIDIProcessor&);
    ~GenerativeMIDIEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // Scrollable content host (DAW frames / AUv3 may be shorter than preferred height)
    class ContentPanel : public juce::Component
    {
    public:
        explicit ContentPanel(GenerativeMIDIEditor& ownerIn) : owner(ownerIn) {}
        void paint(juce::Graphics& g) override { owner.paintContent(g); }
        void resized() override { owner.layoutContent(getLocalBounds()); }

    private:
        GenerativeMIDIEditor& owner;
    };

    void timerCallback() override;
    void updateControlsForGeneratorType(int generatorType);
    void currentPresetChanged(const juce::String& presetName) override;
    void syncPresetLabel(const juce::String& presetName);
    void refreshPresetCombo();
    void notePresetDivergence();
    void updateStatusChip();
    void paintContent(juce::Graphics& g);
    void layoutContent(juce::Rectangle<int> area);
    int preferredContentHeight(bool isPolyrhythm, int width) const;
    void configureDisclosure(juce::TextButton& button, const juce::String& title, bool& expanded);

    struct SectionPlan
    {
        int header = 0;
        int gap = 0;
        int bottomPad = 0;
        int pattern = 0;
        int performance = 0;
        int musical = 0;
        int shape = 0;
        int deck = 0;
        int mod = 84;
        int minimum = 0;
        int used = 0;
        bool sideBySide = false;
    };
    SectionPlan planSections(int availableHeight, bool isPolyrhythm, int width) const;

    GenerativeMIDIProcessor& audioProcessor;
    CustomLookAndFeel customLookAndFeel;

    juce::Viewport editorViewport;
    ContentPanel contentPanel { *this };

    // Pattern area components
    PatternVisualizer patternDisplay;
    std::unique_ptr<PolyrhythmLayerEditor> polyLayerEditor;
    MidiActivityPane midiActivityPane;

    // Parameter sliders
    juce::Slider tempoSlider;
    juce::Slider stepsSlider;
    juce::Slider pulsesSlider;
    juce::Slider rotationSlider;
    juce::Slider densitySlider;
    juce::Slider velocityMinSlider;
    juce::Slider velocityMaxSlider;
    juce::Slider pitchMinSlider;
    juce::Slider pitchMaxSlider;

    // Scale and humanization controls
    juce::Slider swingSlider;
    juce::Slider timingHumanizeSlider;
    juce::Slider velocityHumanizeSlider;
    AccessibleComboBox scaleRootCombo;
    AccessibleComboBox scaleTypeCombo;

    // Gate length controls
    juce::Slider gateLengthSlider;
    juce::TextButton legatoButton;

    // MIDI expression controls (aftertouch / pitch bend / CC)
    juce::TextButton aftertouchEnableButton;
    juce::Slider aftertouchAmountSlider;
    juce::TextButton pitchbendEnableButton;
    juce::Slider pitchbendRangeSlider;
    juce::TextButton ccEnableButton;
    juce::Slider ccNumberSlider;
    juce::Slider ccAmountSlider;

    // Modulation v2 MVP (LFO → velocity)
    juce::TextButton modLfoEnableButton;
    juce::Slider modLfoRateSlider;
    juce::Slider modLfoDepthSlider;
    juce::Slider modLfoDensityDepthSlider;
    juce::Slider modShRateSlider;
    AccessibleComboBox modRoute3SourceCombo;
    AccessibleComboBox modRoute3DestCombo;
    juce::Slider modRoute3AmountSlider;
    AccessibleComboBox modRoute4SourceCombo;
    AccessibleComboBox modRoute4DestCombo;
    juce::Slider modRoute4AmountSlider;

    // Ratchet controls
    juce::Slider ratchetCountSlider;
    juce::Slider ratchetProbabilitySlider;
    juce::Slider ratchetDecaySlider;

    // Stochastic/Chaos controls
    juce::Slider stepSizeSlider;
    juce::Slider momentumSlider;
    juce::Slider timeScaleSlider;

    juce::Slider markovOrderSlider;
    juce::Slider markovStepSlider;
    juce::Slider markovSurpriseSlider;
    juce::Slider lsystemGrammarSlider;
    juce::Slider lsystemGenerationSlider;
    juce::Slider lsystemIntervalSlider;
    juce::Slider cellularRuleSlider;
    juce::Slider cellularSeedSlider;
    juce::Slider cellularListenSlider;

    AccessibleComboBox generatorTypeCombo;
    AccessibleComboBox midiChannelCombo;
    AccessibleComboBox partCountCombo;
    AccessibleComboBox voiceModeCombo;

    // Preset controls
    juce::TextButton presetBrowserButton;
    AccessibleComboBox presetCombo;
    juce::TextButton patternButton;
    juce::TextButton performanceButton;
    juce::TextButton musicalButton;
    juce::TextButton shapeButton;
    juce::TextButton pianoButton;
    juce::Label currentPresetLabel;
    juce::Label statusChipLabel;
    std::unique_ptr<PresetBrowser> presetBrowser;

    // Labels
    juce::Label titleLabel;
    juce::Label productLabel;
    juce::Label tempoLabel;
    juce::Label stepsLabel;
    juce::Label pulsesLabel;
    juce::Label rotationLabel;
    juce::Label generatorLabel;
    juce::Label densityLabel;
    juce::Label velocityLabel;
    juce::Label velocityMinCaption;
    juce::Label velocityMaxCaption;
    juce::Label pitchLabel;
    juce::Label pitchMinCaption;
    juce::Label pitchMaxCaption;
    juce::Label legatoLabel;
    juce::Label scaleLabel;
    juce::Label swingLabel;
    juce::Label timingHumanizeLabel;
    juce::Label velocityHumanizeLabel;
    juce::Label gateLengthLabel;
    juce::Label aftertouchAmountLabel;
    juce::Label pitchbendRangeLabel;
    juce::Label ccNumberLabel;
    juce::Label ccAmountLabel;
    juce::Label modLfoRateLabel;
    juce::Label modLfoDepthLabel;
    juce::Label modLfoDensityDepthLabel;
    juce::Label modShRateLabel;
    juce::Label modRoute3AmountLabel;
    juce::Label modRoute4AmountLabel;
    juce::Label ratchetCountLabel;
    juce::Label ratchetProbabilityLabel;
    juce::Label ratchetDecayLabel;
    juce::Label midiChannelLabel;
    juce::Label partCountLabel;
    juce::Label stepSizeLabel;
    juce::Label momentumLabel;
    juce::Label timeScaleLabel;
    juce::Label markovOrderLabel;
    juce::Label markovStepLabel;
    juce::Label markovSurpriseLabel;
    juce::Label lsystemGrammarLabel;
    juce::Label lsystemGenerationLabel;
    juce::Label lsystemIntervalLabel;
    juce::Label cellularRuleLabel;
    juce::Label cellularSeedLabel;
    juce::Label cellularListenLabel;
    juce::Label advancedRatchetGroupLabel;
    juce::Label advancedStochasticGroupLabel;
    juce::Label advancedLfoGroupLabel;

    // Slider attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tempoAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stepsAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pulsesAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rotationAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> densityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> velocityMinAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> velocityMaxAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchMinAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchMaxAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> swingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> timingHumanizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> velocityHumanizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gateLengthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> legatoAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> aftertouchEnableAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> aftertouchAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> pitchbendEnableAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchbendRangeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> ccEnableAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ccNumberAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ccAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> modLfoEnableAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modLfoRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modLfoDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modLfoDensityDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modShRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modRoute3SourceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modRoute3DestAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modRoute3AmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modRoute4SourceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modRoute4DestAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modRoute4AmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratchetCountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratchetProbabilityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratchetDecayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stepSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> momentumAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> timeScaleAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> markovOrderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> markovStepAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> markovSurpriseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lsystemGrammarAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lsystemGenerationAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lsystemIntervalAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cellularRuleAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cellularSeedAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cellularListenAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> generatorAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleRootAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> midiChannelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> partCountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> voiceModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> pianoAttachment;

    // Section chrome bounds (filled in resized, painted in paint)
    juce::Rectangle<float> patternPanelBounds;
    juce::Rectangle<float> generatorPanelBounds;
    juce::Rectangle<float> expressionPanelBounds;
    juce::Rectangle<float> advancedPanelBounds;
    juce::Rectangle<float> modulationPanelBounds;
    float advancedDividerX1 = 0.0f;
    float advancedDividerX2 = 0.0f;

    bool patternExpanded = true;
    bool performanceExpanded = true;
    bool musicalExpanded = true;
    bool shapeExpanded = true;
    int loadedGeneratorId = -1;
    juce::String loadedPresetName;
    bool updatingPresetCombo = false;
    juce::StringArray lSystemRows;
    int lSystemVizGrammar = -1;
    int lSystemVizGeneration = -1;
    void feedSoundingNotes(int generatorType, int partStep, int pitchMin, int pitchMax);

    int cellularSeenGen = -1;
    uint32_t markovSeenSerial = 0;
    int melodyFeedGenerator = -1;
    int melodyFeedStep = -2;
    int polyLayerFeedStep[16] {};
    bool polyLayerFeedSeen[16] {};
    int lSystemFeedNote = 0;
    uint32_t probSeenNotes = 0;
    bool probSeenPrimed = false;

    uint32_t lastNoteActivityCount = 0;
    float activityPulse = 0.0f;
    int activitySampleFrames = 0;
    bool activitySampleHit = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenerativeMIDIEditor)
};
