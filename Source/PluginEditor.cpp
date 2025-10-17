/*
  ==============================================================================
    PluginEditor.cpp

    Beautiful custom UI implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GenerativeMIDIEditor::GenerativeMIDIEditor(GenerativeMIDIProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customLookAndFeel);

    // Window size
    setSize(900, 600);
    setResizable(true, true);
    setResizeLimits(800, 500, 1600, 1000);

    // Title
    addAndMakeVisible(titleLabel);
    titleLabel.setText("GENERATIVE MIDI", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(32.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00d4ff));

    // Pattern visualizer
    addAndMakeVisible(patternDisplay);

    // Generator type selector
    addAndMakeVisible(generatorLabel);
    generatorLabel.setText("Generator", juce::dontSendNotification);
    generatorLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(generatorTypeCombo);
    generatorTypeCombo.addItemList(juce::StringArray{"Euclidean", "Polyrhythm", "Markov", "L-System", "Cellular", "Probabilistic"}, 1);
    generatorAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
        audioProcessor.getValueTreeState(), "generatorType", generatorTypeCombo));

    // Tempo knob
    addAndMakeVisible(tempoSlider);
    tempoSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    tempoSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    tempoAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "tempo", tempoSlider));
    addAndMakeVisible(tempoLabel);
    tempoLabel.setText("Tempo", juce::dontSendNotification);
    tempoLabel.setJustificationType(juce::Justification::centred);

    // Euclidean controls
    addAndMakeVisible(stepsSlider);
    stepsSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    stepsSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    stepsAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "euclideanSteps", stepsSlider));
    addAndMakeVisible(stepsLabel);
    stepsLabel.setText("Steps", juce::dontSendNotification);
    stepsLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(pulsesSlider);
    pulsesSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pulsesSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    pulsesAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "euclideanPulses", pulsesSlider));
    addAndMakeVisible(pulsesLabel);
    pulsesLabel.setText("Pulses", juce::dontSendNotification);
    pulsesLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(rotationSlider);
    rotationSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rotationSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    rotationAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "euclideanRotation", rotationSlider));
    addAndMakeVisible(rotationLabel);
    rotationLabel.setText("Rotation", juce::dontSendNotification);
    rotationLabel.setJustificationType(juce::Justification::centred);

    // Density control
    addAndMakeVisible(densitySlider);
    densitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    densitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    densityAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "noteDensity", densitySlider));
    addAndMakeVisible(densityLabel);
    densityLabel.setText("Density", juce::dontSendNotification);
    densityLabel.setJustificationType(juce::Justification::centred);

    // Velocity range sliders
    addAndMakeVisible(velocityMinSlider);
    velocityMinSlider.setSliderStyle(juce::Slider::LinearVertical);
    velocityMinSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    velocityMinAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "velocityMin", velocityMinSlider));

    addAndMakeVisible(velocityMaxSlider);
    velocityMaxSlider.setSliderStyle(juce::Slider::LinearVertical);
    velocityMaxSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    velocityMaxAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "velocityMax", velocityMaxSlider));

    addAndMakeVisible(velocityLabel);
    velocityLabel.setText("Velocity Range", juce::dontSendNotification);
    velocityLabel.setJustificationType(juce::Justification::centred);

    // Pitch range sliders
    addAndMakeVisible(pitchMinSlider);
    pitchMinSlider.setSliderStyle(juce::Slider::LinearVertical);
    pitchMinSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    pitchMinAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "pitchMin", pitchMinSlider));

    addAndMakeVisible(pitchMaxSlider);
    pitchMaxSlider.setSliderStyle(juce::Slider::LinearVertical);
    pitchMaxSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    pitchMaxAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "pitchMax", pitchMaxSlider));

    addAndMakeVisible(pitchLabel);
    pitchLabel.setText("Pitch Range", juce::dontSendNotification);
    pitchLabel.setJustificationType(juce::Justification::centred);

    // Start timer for pattern updates
    startTimerHz(30);
}

GenerativeMIDIEditor::~GenerativeMIDIEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void GenerativeMIDIEditor::paint(juce::Graphics& g)
{
    // Modern gradient background
    g.fillAll(juce::Colour(0xff1a1a1a));

    // Subtle grid pattern
    g.setColour(juce::Colour(0xff242424));
    for (int i = 0; i < getWidth(); i += 20)
        g.drawVerticalLine(i, 0.0f, static_cast<float>(getHeight()));
    for (int i = 0; i < getHeight(); i += 20)
        g.drawHorizontalLine(i, 0.0f, static_cast<float>(getWidth()));

    // Section dividers
    g.setColour(juce::Colour(0xff333333));
    g.drawRect(20, 80, getWidth() - 40, 120, 2);
    g.drawRect(20, 220, getWidth() - 40, 180, 2);
    g.drawRect(20, 420, getWidth() - 40, 160, 2);

    // Section labels
    g.setColour(juce::Colour(0xff00d4ff));
    g.setFont(16.0f);
    g.drawText("PATTERN DISPLAY", 30, 85, 200, 20, juce::Justification::left);
    g.drawText("GENERATOR CONTROLS", 30, 225, 200, 20, juce::Justification::left);
    g.drawText("EXPRESSION & RANGE", 30, 425, 200, 20, juce::Justification::left);
}

void GenerativeMIDIEditor::resized()
{
    auto area = getLocalBounds();

    // Title
    titleLabel.setBounds(area.removeFromTop(60).reduced(20, 10));

    // Pattern display section
    auto patternSection = area.removeFromTop(140).reduced(40, 20);
    patternSection.removeFromTop(20); // Section label
    patternDisplay.setBounds(patternSection);

    // Generator controls section
    auto controlsSection = area.removeFromTop(200).reduced(40, 20);
    controlsSection.removeFromTop(20); // Section label

    int knobSize = 100;
    int spacing = 20;

    auto generatorArea = controlsSection.removeFromLeft(150);
    generatorLabel.setBounds(generatorArea.removeFromTop(20));
    generatorTypeCombo.setBounds(generatorArea.removeFromTop(30).reduced(10, 0));

    controlsSection.removeFromLeft(spacing);

    auto tempoArea = controlsSection.removeFromLeft(knobSize);
    tempoLabel.setBounds(tempoArea.removeFromBottom(20));
    tempoSlider.setBounds(tempoArea);

    controlsSection.removeFromLeft(spacing);

    auto stepsArea = controlsSection.removeFromLeft(knobSize);
    stepsLabel.setBounds(stepsArea.removeFromBottom(20));
    stepsSlider.setBounds(stepsArea);

    controlsSection.removeFromLeft(spacing);

    auto pulsesArea = controlsSection.removeFromLeft(knobSize);
    pulsesLabel.setBounds(pulsesArea.removeFromBottom(20));
    pulsesSlider.setBounds(pulsesArea);

    controlsSection.removeFromLeft(spacing);

    auto rotationArea = controlsSection.removeFromLeft(knobSize);
    rotationLabel.setBounds(rotationArea.removeFromBottom(20));
    rotationSlider.setBounds(rotationArea);

    controlsSection.removeFromLeft(spacing);

    auto densityArea = controlsSection.removeFromLeft(knobSize);
    densityLabel.setBounds(densityArea.removeFromBottom(20));
    densitySlider.setBounds(densityArea);

    // Expression & range section
    auto rangeSection = area.removeFromTop(180).reduced(40, 20);
    rangeSection.removeFromTop(20); // Section label

    auto velocityArea = rangeSection.removeFromLeft(120);
    velocityLabel.setBounds(velocityArea.removeFromTop(20));
    auto vSliders = velocityArea.reduced(10, 0);
    velocityMinSlider.setBounds(vSliders.removeFromLeft(50));
    vSliders.removeFromLeft(10);
    velocityMaxSlider.setBounds(vSliders);

    rangeSection.removeFromLeft(40);

    auto pitchArea = rangeSection.removeFromLeft(120);
    pitchLabel.setBounds(pitchArea.removeFromTop(20));
    auto pSliders = pitchArea.reduced(10, 0);
    pitchMinSlider.setBounds(pSliders.removeFromLeft(50));
    pSliders.removeFromLeft(10);
    pitchMaxSlider.setBounds(pSliders);
}

void GenerativeMIDIEditor::timerCallback()
{
    // Update pattern visualizer
    auto& euclidean = audioProcessor.getEuclideanEngine();
    std::vector<bool> pattern(euclidean.getSteps());
    for (int i = 0; i < euclidean.getSteps(); ++i)
        pattern[i] = euclidean.getStep(i);

    patternDisplay.setPattern(pattern);
    // Note: we'd need to expose lastSubdivisionStep to show current position
    // For now, pattern display will show the pattern without playhead
}

//==============================================================================
// Override createEditor in processor to use custom editor
