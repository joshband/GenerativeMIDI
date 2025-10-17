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

class GenerativeMIDIEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    GenerativeMIDIEditor(GenerativeMIDIProcessor&);
    ~GenerativeMIDIEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

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

    juce::ComboBox generatorTypeCombo;

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
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> generatorAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenerativeMIDIEditor)
};
