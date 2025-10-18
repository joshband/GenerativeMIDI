/*
  ==============================================================================
    PluginEditor.h

    Custom modern UI for generative MIDI processor

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/CustomLookAndFeel.h"
#include "UI/PatternVisualizer.h"
#include "UI/PresetBrowser.h"

class GenerativeMIDIEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    GenerativeMIDIEditor(GenerativeMIDIProcessor&);
    ~GenerativeMIDIEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void updateControlsForGeneratorType(int generatorType);

    GenerativeMIDIProcessor& audioProcessor;
    CustomLookAndFeel customLookAndFeel;

    // Pattern visualizer
    PatternVisualizer patternDisplay;

    // Parameter attachments for sliders
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
    juce::ComboBox scaleRootCombo;
    juce::ComboBox scaleTypeCombo;

    // Gate length controls
    juce::Slider gateLengthSlider;
    juce::TextButton legatoButton;

    // Ratchet controls
    juce::Slider ratchetCountSlider;
    juce::Slider ratchetProbabilitySlider;
    juce::Slider ratchetDecaySlider;

    // Stochastic/Chaos controls
    juce::Slider stepSizeSlider;
    juce::Slider momentumSlider;
    juce::Slider timeScaleSlider;

    juce::ComboBox generatorTypeCombo;
    juce::ComboBox midiChannelCombo;

    // Preset controls
    juce::TextButton presetBrowserButton;
    juce::Label currentPresetLabel;
    std::unique_ptr<PresetBrowser> presetBrowser;

    // Labels
    juce::Label titleLabel;
    juce::Label tempoLabel;
    juce::Label stepsLabel;
    juce::Label pulsesLabel;
    juce::Label rotationLabel;
    juce::Label generatorLabel;
    juce::Label densityLabel;
    juce::Label velocityLabel;
    juce::Label pitchLabel;
    juce::Label scaleLabel;
    juce::Label swingLabel;
    juce::Label timingHumanizeLabel;
    juce::Label velocityHumanizeLabel;
    juce::Label gateLengthLabel;
    juce::Label ratchetCountLabel;
    juce::Label ratchetProbabilityLabel;
    juce::Label ratchetDecayLabel;
    juce::Label midiChannelLabel;
    juce::Label stepSizeLabel;
    juce::Label momentumLabel;
    juce::Label timeScaleLabel;

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
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratchetCountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratchetProbabilityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratchetDecayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stepSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> momentumAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> timeScaleAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> generatorAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleRootAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> midiChannelAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenerativeMIDIEditor)
};
