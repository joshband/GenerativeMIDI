/*
  ==============================================================================
    PluginEditor.cpp

    Beautiful custom UI implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Core/GeneratorTypeMapping.h"
#include <algorithm>

namespace
{
// Pattern is the top band, about two-fifths of the window. On a wide editor the
// Performance, Musical, and Shape panels share one row beneath it.
constexpr int kEditorHeaderH = 44;
constexpr int kBottomPad = 8;
constexpr int kSectionGap = 8;
constexpr int kPanelHeaderH = 28;
constexpr int kCollapsedPanelH = 32;
constexpr int kPerformanceBody = 96;
constexpr int kMusicalBody = 96;
constexpr int kPatternBodyMin = 64;
constexpr int kPatternBodyComfort = 140;
constexpr int kShapeBodyMin = 132;
constexpr int kShapeBodyMax = 200;
constexpr int kReadoutW = 56;
constexpr int kReadoutH = 16;

template <size_t N>
void setShown(juce::Component* (&widgets)[N], bool show)
{
    for (auto* widget : widgets)
        if (widget != nullptr)
            widget->setVisible(show);
}

void placeDisclosure(juce::Rectangle<int> panel, juce::TextButton& button)
{
    const int hostH = panel.getHeight() <= kCollapsedPanelH ? panel.getHeight() : kPanelHeaderH;
    button.setBounds(panel.getX(), panel.getY(), panel.getWidth(), hostH);
}
}

//==============================================================================
GenerativeMIDIEditor::GenerativeMIDIEditor(GenerativeMIDIProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customLookAndFeel);

    // Cross-format defaults: usable in typical DAW frames; scroll when shorter
#if JUCE_IOS
    setSize(1024, 700);
    setResizeLimits(640, 480, 2000, 1400);
#else
    setSize(1280, 760);
    setResizeLimits(720, 520, 2000, 1400);
#endif
    setResizable(true, true);

    addAndMakeVisible(editorViewport);
    editorViewport.setViewedComponent(&contentPanel, false);
    editorViewport.setScrollBarsShown(true, false);
#if JUCE_IOS
    editorViewport.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::all);
#endif
    editorViewport.getVerticalScrollBar().setColour(
        juce::ScrollBar::thumbColourId, juce::Colour(CustomLookAndFeel::BRASS_AGED));
    editorViewport.getVerticalScrollBar().setColour(
        juce::ScrollBar::trackColourId, juce::Colour(CustomLookAndFeel::ABYSS_NAVY));

    // Compact brand mark (demoted) + product name
    contentPanel.addAndMakeVisible(titleLabel);
    titleLabel.setText("SYNAPTIK", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::BRASS_AGED));
    titleLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    contentPanel.addAndMakeVisible(productLabel);
    productLabel.setText("Generative MIDI", juce::dontSendNotification);
    productLabel.setFont(juce::FontOptions(17.0f).withStyle("Bold"));
    productLabel.setJustificationType(juce::Justification::centredLeft);
    productLabel.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::GOLD_TEMPLE));
    productLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    contentPanel.addAndMakeVisible(statusChipLabel);
    statusChipLabel.setFont(juce::FontOptions(11.0f));
    statusChipLabel.setJustificationType(juce::Justification::centredLeft);
    statusChipLabel.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::AETHER_CYAN));
    statusChipLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    statusChipLabel.setTitle("Transport Status");
    updateStatusChip();

    // Pattern display + polyrhythm layer editor (swapped by generator type)
    contentPanel.addAndMakeVisible(patternDisplay);
    polyLayerEditor = std::make_unique<PolyrhythmLayerEditor>(audioProcessor.getPolyrhythmEngine());
    polyLayerEditor->onLayersChanged = [this]() { resized(); };
    contentPanel.addChildComponent(polyLayerEditor.get());

    addAndMakeVisible(midiActivityPane);
    midiActivityPane.onExpandedChanged = [this]
    {
        resized();
    };

    configureDisclosure(patternButton, "Pattern", patternExpanded);
    configureDisclosure(performanceButton, "Performance", performanceExpanded);
    configureDisclosure(musicalButton, "Musical", musicalExpanded);
    configureDisclosure(shapeButton, "Shape", shapeExpanded);

    contentPanel.addAndMakeVisible(pianoButton);
    pianoButton.setButtonText("Piano");
    pianoButton.setClickingTogglesState(true);
    pianoButton.setTitle("Piano");
    pianoButton.setName("Piano");
    pianoButton.setVisible(audioProcessor.wrapperType == juce::AudioProcessor::wrapperType_Standalone);
    pianoAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(
        audioProcessor.getValueTreeState(), "pianoEnable", pianoButton));

    // Generator type selector
    contentPanel.addAndMakeVisible(generatorLabel);
    generatorLabel.setText("Generator", juce::dontSendNotification);
    generatorLabel.setJustificationType(juce::Justification::centred);

    contentPanel.addAndMakeVisible(generatorTypeCombo);
    generatorTypeCombo.addItemList(juce::StringArray{"Euclidean", "Polyrhythm", "Markov", "L-System", "Cellular", "Probabilistic",
                                                      "Brownian", "Perlin Noise", "Drunk Walk", "Lorenz"}, 1);
    generatorAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
        audioProcessor.getValueTreeState(), "generatorType", generatorTypeCombo));

    // Listen for generator type changes to update UI
    generatorTypeCombo.onChange = [this]() {
        updateControlsForGeneratorType(generatorTypeCombo.getSelectedId() - 1);
        notePresetDivergence();
    };

    // MIDI Channel selector
    contentPanel.addAndMakeVisible(midiChannelLabel);
    midiChannelLabel.setText("Ch", juce::dontSendNotification);
    midiChannelLabel.setJustificationType(juce::Justification::centred);

    contentPanel.addAndMakeVisible(midiChannelCombo);
    for (int i = 1; i <= 16; ++i)
        midiChannelCombo.addItem(juce::String(i), i);
    midiChannelAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
        audioProcessor.getValueTreeState(), "midiChannel", midiChannelCombo));

    contentPanel.addAndMakeVisible(partCountLabel);
    partCountLabel.setText("Parts", juce::dontSendNotification);
    partCountLabel.setJustificationType(juce::Justification::centred);

    contentPanel.addAndMakeVisible(partCountCombo);
    partCountCombo.addItemList(juce::StringArray { "1", "2", "3", "4" }, 1);
    partCountAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
        audioProcessor.getValueTreeState(), "partCount", partCountCombo));

    contentPanel.addAndMakeVisible(voiceModeCombo);
    voiceModeCombo.addItemList(juce::StringArray { "Poly", "Mono" }, 1);
    voiceModeAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
        audioProcessor.getValueTreeState(), "voiceMode", voiceModeCombo));

    // Tempo knob
    contentPanel.addAndMakeVisible(tempoSlider);
    tempoSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    tempoSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    tempoAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "tempo", tempoSlider));
    contentPanel.addAndMakeVisible(tempoLabel);
    tempoLabel.setText("Tempo", juce::dontSendNotification);
    tempoLabel.setJustificationType(juce::Justification::centred);

    // Euclidean controls
    contentPanel.addAndMakeVisible(stepsSlider);
    stepsSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    stepsSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    stepsAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "euclideanSteps", stepsSlider));
    contentPanel.addAndMakeVisible(stepsLabel);
    stepsLabel.setText("Steps", juce::dontSendNotification);
    stepsLabel.setJustificationType(juce::Justification::centred);

    contentPanel.addAndMakeVisible(pulsesSlider);
    pulsesSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pulsesSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    pulsesAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "euclideanPulses", pulsesSlider));
    contentPanel.addAndMakeVisible(pulsesLabel);
    pulsesLabel.setText("Pulses", juce::dontSendNotification);
    pulsesLabel.setJustificationType(juce::Justification::centred);
    pulsesLabel.setMinimumHorizontalScale(0.7f);

    contentPanel.addAndMakeVisible(rotationSlider);
    rotationSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rotationSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    rotationAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "euclideanRotation", rotationSlider));
    contentPanel.addAndMakeVisible(rotationLabel);
    rotationLabel.setText("Rotation", juce::dontSendNotification);
    rotationLabel.setJustificationType(juce::Justification::centred);

    // Probability control (applies to all generators)
    contentPanel.addAndMakeVisible(densitySlider);
    densitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    densitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    densityAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "noteDensity", densitySlider));
    contentPanel.addAndMakeVisible(densityLabel);
    densityLabel.setText("Probability", juce::dontSendNotification);
    densityLabel.setJustificationType(juce::Justification::centred);

    // Velocity range sliders
    contentPanel.addAndMakeVisible(velocityMinSlider);
    velocityMinSlider.setSliderStyle(juce::Slider::LinearVertical);
    velocityMinSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 44, 16);
    velocityMinAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "velocityMin", velocityMinSlider));

    contentPanel.addAndMakeVisible(velocityMaxSlider);
    velocityMaxSlider.setSliderStyle(juce::Slider::LinearVertical);
    velocityMaxSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 44, 16);
    velocityMaxAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "velocityMax", velocityMaxSlider));

    contentPanel.addAndMakeVisible(velocityLabel);
    velocityLabel.setText("Velocity", juce::dontSendNotification);
    velocityLabel.setJustificationType(juce::Justification::centred);
    velocityMinSlider.setTitle("Velocity Min");
    velocityMaxSlider.setTitle("Velocity Max");

    // Pitch range sliders
    contentPanel.addAndMakeVisible(pitchMinSlider);
    pitchMinSlider.setSliderStyle(juce::Slider::LinearVertical);
    pitchMinSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 44, 16);
    pitchMinAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "pitchMin", pitchMinSlider));

    contentPanel.addAndMakeVisible(pitchMaxSlider);
    pitchMaxSlider.setSliderStyle(juce::Slider::LinearVertical);
    pitchMaxSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 44, 16);
    pitchMaxAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "pitchMax", pitchMaxSlider));

    contentPanel.addAndMakeVisible(pitchLabel);
    pitchLabel.setText("Pitch", juce::dontSendNotification);
    pitchLabel.setJustificationType(juce::Justification::centred);
    pitchMinSlider.setTitle("Pitch Min");
    pitchMaxSlider.setTitle("Pitch Max");

    auto styleRangeCaption = [this](juce::Label& label, const char* text)
    {
        contentPanel.addAndMakeVisible(label);
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::FontOptions(10.0f));
        label.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::COPPER_STEAM));
    };
    styleRangeCaption(velocityMinCaption, "Min");
    styleRangeCaption(velocityMaxCaption, "Max");
    styleRangeCaption(pitchMinCaption, "Min");
    styleRangeCaption(pitchMaxCaption, "Max");

    // Scale controls
    contentPanel.addAndMakeVisible(scaleLabel);
    scaleLabel.setText("Scale", juce::dontSendNotification);
    scaleLabel.setJustificationType(juce::Justification::centred);

    contentPanel.addAndMakeVisible(scaleRootCombo);
    scaleRootCombo.addItemList(juce::StringArray{"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"}, 1);
    scaleRootAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
        audioProcessor.getValueTreeState(), "scaleRoot", scaleRootCombo));

    contentPanel.addAndMakeVisible(scaleTypeCombo);
    scaleTypeCombo.addItemList(juce::StringArray{"Chromatic", "Major", "Minor", "Harmonic Minor", "Melodic Minor",
                                                   "Dorian", "Phrygian", "Lydian", "Mixolydian", "Locrian",
                                                   "Major Pentatonic", "Minor Pentatonic", "Blues", "Whole Tone",
                                                   "Diminished", "Harmonic Major"}, 1);
    scaleTypeAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
        audioProcessor.getValueTreeState(), "scaleType", scaleTypeCombo));

    // Swing and humanization controls
    contentPanel.addAndMakeVisible(swingSlider);
    swingSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    swingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    swingAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "swingAmount", swingSlider));
    contentPanel.addAndMakeVisible(swingLabel);
    swingLabel.setText("Swing", juce::dontSendNotification);
    swingLabel.setJustificationType(juce::Justification::centred);

    contentPanel.addAndMakeVisible(timingHumanizeSlider);
    timingHumanizeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    timingHumanizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    timingHumanizeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "timingHumanize", timingHumanizeSlider));
    contentPanel.addAndMakeVisible(timingHumanizeLabel);
    timingHumanizeLabel.setText("Timing", juce::dontSendNotification);
    timingHumanizeLabel.setJustificationType(juce::Justification::centred);

    contentPanel.addAndMakeVisible(velocityHumanizeSlider);
    velocityHumanizeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    velocityHumanizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    velocityHumanizeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "velocityHumanize", velocityHumanizeSlider));
    contentPanel.addAndMakeVisible(velocityHumanizeLabel);
    velocityHumanizeLabel.setText("Vel Var", juce::dontSendNotification);
    velocityHumanizeLabel.setJustificationType(juce::Justification::centred);
    velocityHumanizeLabel.setMinimumHorizontalScale(0.7f);

    // Gate length controls
    contentPanel.addAndMakeVisible(gateLengthSlider);
    gateLengthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gateLengthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    gateLengthAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "gateLength", gateLengthSlider));
    contentPanel.addAndMakeVisible(gateLengthLabel);
    gateLengthLabel.setText("Gate", juce::dontSendNotification);
    gateLengthLabel.setJustificationType(juce::Justification::centred);

    contentPanel.addAndMakeVisible(legatoLabel);
    legatoLabel.setText("Legato", juce::dontSendNotification);
    legatoLabel.setJustificationType(juce::Justification::centred);
    contentPanel.addAndMakeVisible(legatoButton);
    legatoButton.setButtonText("Legato");
    legatoButton.setClickingTogglesState(true);
    legatoAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(
        audioProcessor.getValueTreeState(), "legatoMode", legatoButton));

    // MIDI expression (aftertouch / pitch bend / CC) — APVTS params already exist
    contentPanel.addAndMakeVisible(aftertouchEnableButton);
    aftertouchEnableButton.setButtonText("AT");
    aftertouchEnableButton.setClickingTogglesState(true);
    aftertouchEnableAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(
        audioProcessor.getValueTreeState(), "aftertouchEnable", aftertouchEnableButton));

    contentPanel.addAndMakeVisible(aftertouchAmountSlider);
    aftertouchAmountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    aftertouchAmountSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    aftertouchAmountAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "aftertouchAmount", aftertouchAmountSlider));
    contentPanel.addAndMakeVisible(aftertouchAmountLabel);
    aftertouchAmountLabel.setText("AT Amt", juce::dontSendNotification);
    aftertouchAmountLabel.setJustificationType(juce::Justification::centred);

    contentPanel.addAndMakeVisible(pitchbendEnableButton);
    pitchbendEnableButton.setButtonText("PB");
    pitchbendEnableButton.setClickingTogglesState(true);
    pitchbendEnableAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(
        audioProcessor.getValueTreeState(), "pitchbendEnable", pitchbendEnableButton));

    contentPanel.addAndMakeVisible(pitchbendRangeSlider);
    pitchbendRangeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pitchbendRangeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    pitchbendRangeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "pitchbendRange", pitchbendRangeSlider));
    contentPanel.addAndMakeVisible(pitchbendRangeLabel);
    pitchbendRangeLabel.setText("PB Semi", juce::dontSendNotification);
    pitchbendRangeLabel.setJustificationType(juce::Justification::centred);

    contentPanel.addAndMakeVisible(ccEnableButton);
    ccEnableButton.setButtonText("CC");
    ccEnableButton.setClickingTogglesState(true);
    ccEnableAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(
        audioProcessor.getValueTreeState(), "ccEnable", ccEnableButton));

    contentPanel.addAndMakeVisible(ccNumberSlider);
    ccNumberSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ccNumberSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    ccNumberSlider.setNumDecimalPlacesToDisplay(0);
    ccNumberAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "ccNumber", ccNumberSlider));
    contentPanel.addAndMakeVisible(ccNumberLabel);
    ccNumberLabel.setText("CC #", juce::dontSendNotification);
    ccNumberLabel.setJustificationType(juce::Justification::centred);

    contentPanel.addAndMakeVisible(ccAmountSlider);
    ccAmountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ccAmountSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    ccAmountAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "ccAmount", ccAmountSlider));
    contentPanel.addAndMakeVisible(ccAmountLabel);
    ccAmountLabel.setText("CC Amt", juce::dontSendNotification);
    ccAmountLabel.setJustificationType(juce::Justification::centred);

    // Modulation v2 MVP: LFO → velocity
    contentPanel.addAndMakeVisible(modLfoEnableButton);
    modLfoEnableButton.setButtonText("LFO");
    modLfoEnableButton.setClickingTogglesState(true);
    modLfoEnableAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(
        audioProcessor.getValueTreeState(), "modLfoEnable", modLfoEnableButton));

    contentPanel.addAndMakeVisible(modLfoRateSlider);
    modLfoRateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modLfoRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    modLfoRateAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "modLfoRate", modLfoRateSlider));
    contentPanel.addAndMakeVisible(modLfoRateLabel);
    modLfoRateLabel.setText("LFO Hz", juce::dontSendNotification);
    modLfoRateLabel.setJustificationType(juce::Justification::centred);

    contentPanel.addAndMakeVisible(modLfoDepthSlider);
    modLfoDepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modLfoDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    modLfoDepthAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "modLfoDepth", modLfoDepthSlider));
    contentPanel.addAndMakeVisible(modLfoDepthLabel);
    modLfoDepthLabel.setText(juce::String::fromUTF8("LFO → Velocity"), juce::dontSendNotification);
    modLfoDepthLabel.setJustificationType(juce::Justification::centred);

    contentPanel.addAndMakeVisible(modLfoDensityDepthSlider);
    modLfoDensityDepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modLfoDensityDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    modLfoDensityDepthAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "modLfoDensityDepth", modLfoDensityDepthSlider));
    contentPanel.addAndMakeVisible(modLfoDensityDepthLabel);
    modLfoDensityDepthLabel.setText(juce::String::fromUTF8("LFO → Density"), juce::dontSendNotification);
    modLfoDensityDepthLabel.setJustificationType(juce::Justification::centred);

    contentPanel.addAndMakeVisible(modShRateSlider);
    modShRateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modShRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    modShRateAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "modShRate", modShRateSlider));
    contentPanel.addAndMakeVisible(modShRateLabel);
    modShRateLabel.setText("S&H Hz", juce::dontSendNotification);
    modShRateLabel.setJustificationType(juce::Justification::centred);

    auto addRoute = [this](AccessibleComboBox& source, AccessibleComboBox& dest, juce::Slider& amount,
                           juce::Label& amountLabel, const char* sourceId, const char* destId,
                           const char* amountId,
                           std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>& sourceAttachment,
                           std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>& destAttachment,
                           std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& amountAttachment)
    {
        contentPanel.addAndMakeVisible(source);
        source.addItem("LFO", 1);
        source.addItem("S&H", 2);
        sourceAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
            audioProcessor.getValueTreeState(), sourceId, source));

        contentPanel.addAndMakeVisible(dest);
        dest.addItem("Off", 1);
        dest.addItem("Gate", 2);
        dest.addItem("Pitch", 3);
        dest.addItem("CC", 4);
        dest.addItem("Bend", 5);
        destAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
            audioProcessor.getValueTreeState(), destId, dest));

        contentPanel.addAndMakeVisible(amount);
        amount.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        amount.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
        amountAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
            audioProcessor.getValueTreeState(), amountId, amount));
        contentPanel.addAndMakeVisible(amountLabel);
        amountLabel.setText("Amount", juce::dontSendNotification);
        amountLabel.setJustificationType(juce::Justification::centred);
    };
    addRoute(modRoute3SourceCombo, modRoute3DestCombo, modRoute3AmountSlider, modRoute3AmountLabel,
             "modRoute3Source", "modRoute3Dest", "modRoute3Amount",
             modRoute3SourceAttachment, modRoute3DestAttachment, modRoute3AmountAttachment);
    addRoute(modRoute4SourceCombo, modRoute4DestCombo, modRoute4AmountSlider, modRoute4AmountLabel,
             "modRoute4Source", "modRoute4Dest", "modRoute4Amount",
             modRoute4SourceAttachment, modRoute4DestAttachment, modRoute4AmountAttachment);

    // Ratchet controls
    contentPanel.addAndMakeVisible(ratchetCountSlider);
    ratchetCountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ratchetCountSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    ratchetCountAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "ratchetCount", ratchetCountSlider));
    contentPanel.addAndMakeVisible(ratchetCountLabel);
    ratchetCountLabel.setText("Ratchet", juce::dontSendNotification);
    ratchetCountLabel.setJustificationType(juce::Justification::centred);

    contentPanel.addAndMakeVisible(ratchetProbabilitySlider);
    ratchetProbabilitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ratchetProbabilitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    ratchetProbabilityAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "ratchetProbability", ratchetProbabilitySlider));
    contentPanel.addAndMakeVisible(ratchetProbabilityLabel);
    ratchetProbabilityLabel.setText("R. Prob", juce::dontSendNotification);
    ratchetProbabilityLabel.setJustificationType(juce::Justification::centred);
    ratchetProbabilityLabel.setMinimumHorizontalScale(0.65f);

    contentPanel.addAndMakeVisible(ratchetDecaySlider);
    ratchetDecaySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ratchetDecaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    ratchetDecayAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "ratchetDecay", ratchetDecaySlider));
    contentPanel.addAndMakeVisible(ratchetDecayLabel);
    ratchetDecayLabel.setText("R. Decay", juce::dontSendNotification);
    ratchetDecayLabel.setJustificationType(juce::Justification::centred);
    ratchetDecayLabel.setMinimumHorizontalScale(0.65f);

    // Stochastic/Chaos controls
    contentPanel.addAndMakeVisible(stepSizeSlider);
    stepSizeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    stepSizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    stepSizeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "stepSize", stepSizeSlider));
    contentPanel.addAndMakeVisible(stepSizeLabel);
    stepSizeLabel.setText("Step Size", juce::dontSendNotification);
    stepSizeLabel.setJustificationType(juce::Justification::centred);
    stepSizeLabel.setMinimumHorizontalScale(0.65f);

    contentPanel.addAndMakeVisible(momentumSlider);
    momentumSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    momentumSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    momentumAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "momentum", momentumSlider));
    contentPanel.addAndMakeVisible(momentumLabel);
    momentumLabel.setText("Momentum", juce::dontSendNotification);
    momentumLabel.setJustificationType(juce::Justification::centred);

    contentPanel.addAndMakeVisible(timeScaleSlider);
    timeScaleSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    timeScaleSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    timeScaleAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "timeScale", timeScaleSlider));
    contentPanel.addAndMakeVisible(timeScaleLabel);
    timeScaleLabel.setText("Time Scale", juce::dontSendNotification);
    timeScaleLabel.setJustificationType(juce::Justification::centred);
    timeScaleLabel.setMinimumHorizontalScale(0.65f);

    auto addPerfKnob = [this](juce::Slider& slider, juce::Label& label,
                              const char* paramId, const char* title,
                              std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment)
    {
        contentPanel.addAndMakeVisible(slider);
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 20);
        slider.setTitle(title);
        attachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
            audioProcessor.getValueTreeState(), paramId, slider));
        contentPanel.addAndMakeVisible(label);
        label.setText(title, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setMinimumHorizontalScale(0.65f);
    };

    addPerfKnob(markovOrderSlider, markovOrderLabel, "markovOrder", "Order", markovOrderAttachment);
    addPerfKnob(markovStepSlider, markovStepLabel, "markovStep", "Step", markovStepAttachment);
    addPerfKnob(markovSurpriseSlider, markovSurpriseLabel, "markovSurprise", "Surprise", markovSurpriseAttachment);
    addPerfKnob(lsystemGrammarSlider, lsystemGrammarLabel, "lsystemGrammar", "Grammar", lsystemGrammarAttachment);
    addPerfKnob(lsystemGenerationSlider, lsystemGenerationLabel, "lsystemGeneration", "Generation", lsystemGenerationAttachment);
    addPerfKnob(lsystemIntervalSlider, lsystemIntervalLabel, "lsystemInterval", "Interval", lsystemIntervalAttachment);
    addPerfKnob(cellularRuleSlider, cellularRuleLabel, "cellularRule", "Rule", cellularRuleAttachment);
    addPerfKnob(cellularSeedSlider, cellularSeedLabel, "cellularSeed", "Seed", cellularSeedAttachment);
    addPerfKnob(cellularListenSlider, cellularListenLabel, "cellularListen", "Listen", cellularListenAttachment);

    lsystemGrammarSlider.textFromValueFunction = [](double value)
    {
        return juce::String(LSystemCatalog::name(static_cast<int>(value)));
    };
    lsystemGrammarSlider.valueFromTextFunction = [](const juce::String& text)
    {
        for (int i = 0; i < LSystemCatalog::kCount; ++i)
            if (text.equalsIgnoreCase(LSystemCatalog::name(i)))
                return static_cast<double>(i);
        return text.getDoubleValue();
    };
    lsystemGrammarSlider.updateText();

    // Advanced group sublabels (Ratchet | Stochastic | LFO)
    auto styleGroupLabel = [](juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::Font(juce::FontOptions(10.0f)).withExtraKerningFactor(0.14f));
        label.setJustificationType(juce::Justification::centredLeft);
        label.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::BRASS_AGED));
    };
    contentPanel.addAndMakeVisible(advancedStochasticGroupLabel);
    styleGroupLabel(advancedStochasticGroupLabel, "Expression");
    contentPanel.addAndMakeVisible(advancedRatchetGroupLabel);
    styleGroupLabel(advancedRatchetGroupLabel, "Ratchet");
    contentPanel.addAndMakeVisible(advancedLfoGroupLabel);
    styleGroupLabel(advancedLfoGroupLabel, "LFO");

    // Preset browser button
    contentPanel.addAndMakeVisible(presetBrowserButton);
    presetBrowserButton.setButtonText("Presets");
    presetBrowserButton.onClick = [this]()
    {
        if (!presetBrowser)
        {
            presetBrowser = std::make_unique<PresetBrowser>(audioProcessor.getPresetManager());
            presetBrowser->setLookAndFeel(&customLookAndFeel);
            presetBrowser->setSize(520, 640);
        }

        juce::DialogWindow::LaunchOptions options;
        options.content.setNonOwned(presetBrowser.get());
        options.dialogTitle = "Preset Manager";
        options.componentToCentreAround = this;
        options.dialogBackgroundColour = juce::Colour(CustomLookAndFeel::ABYSS_NAVY);
        options.escapeKeyTriggersCloseButton = true;
        options.useNativeTitleBar = true;
        options.resizable = true;

        options.launchAsync();
    };

    contentPanel.addAndMakeVisible(presetCombo);
    presetCombo.setTitle("Preset");
    presetCombo.onChange = [this]()
    {
        if (updatingPresetCombo)
            return;
        const auto name = presetCombo.getText();
        if (name.isEmpty() || name == audioProcessor.getPresetManager().getCurrentPresetName())
            return;
        audioProcessor.getPresetManager().loadPresetByName(name);
    };
    refreshPresetCombo();

    // Current preset label
    contentPanel.addAndMakeVisible(currentPresetLabel);
    syncPresetLabel(audioProcessor.getPresetManager().getCurrentPresetName());
    currentPresetLabel.setFont(juce::FontOptions(12.0f));
    currentPresetLabel.setJustificationType(juce::Justification::centred);
    currentPresetLabel.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::COPPER_STEAM));
    currentPresetLabel.setMinimumHorizontalScale(0.7f);

    audioProcessor.getPresetManager().addListener(this);

    // Accessibility titles for System Events / AX automation (avoid unnamed popups)
    setTitle("Generative MIDI");
    generatorTypeCombo.setTitle("Generator Type");
    midiChannelCombo.setTitle("MIDI Channel");
    partCountCombo.setTitle("Parts");
    voiceModeCombo.setTitle("Voice");
    presetBrowserButton.setTitle("Presets");
    presetCombo.setTitle("Preset");
    currentPresetLabel.setTitle("Current Preset");
    densitySlider.setTitle("Probability");
    tempoSlider.setTitle("Tempo");
    stepsSlider.setTitle("Steps");
    pulsesSlider.setTitle("Pulses");
    rotationSlider.setTitle("Rotation");
    scaleRootCombo.setTitle("Scale Root");
    scaleTypeCombo.setTitle("Scale Type");
    modLfoEnableButton.setTitle("LFO Enable");
    modLfoRateSlider.setTitle("LFO Rate");
    modLfoDepthSlider.setTitle("LFO Velocity");
    modLfoDensityDepthSlider.setTitle("LFO Density");
    modShRateSlider.setTitle("S&H Rate");
    modRoute3SourceCombo.setTitle("Mod Route 3 Source");
    modRoute3DestCombo.setTitle("Mod Route 3 Destination");
    modRoute3AmountSlider.setTitle("Mod Route 3 Amount");
    modRoute4SourceCombo.setTitle("Mod Route 4 Source");
    modRoute4DestCombo.setTitle("Mod Route 4 Destination");
    modRoute4AmountSlider.setTitle("Mod Route 4 Amount");
    ratchetCountSlider.setTitle("Ratchet Count");
    ratchetProbabilitySlider.setTitle("Ratchet Probability");
    ratchetDecaySlider.setTitle("Ratchet Decay");
    stepSizeSlider.setTitle("Step Size");
    momentumSlider.setTitle("Momentum");
    timeScaleSlider.setTitle("Time Scale");
    aftertouchEnableButton.setTitle("Aftertouch Enable");
    pitchbendEnableButton.setTitle("Pitch Bend Enable");
    ccEnableButton.setTitle("CC Enable");
    legatoButton.setTitle("Legato");
    if (polyLayerEditor != nullptr)
        polyLayerEditor->setTitle("Polyrhythm Layers");

    // Initialize UI for current generator type
    updateControlsForGeneratorType(generatorTypeCombo.getSelectedId() - 1);

    // Fixed-slot modulation lives in the editor bar (see Source/Modulation/ModulationRouter.h).

    // Start timer for pattern updates
    startTimerHz(30);
}

GenerativeMIDIEditor::~GenerativeMIDIEditor()
{
    audioProcessor.getPresetManager().removeListener(this);
    if (presetBrowser != nullptr)
        presetBrowser->setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

//==============================================================================
void GenerativeMIDIEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(CustomLookAndFeel::ABYSS_NAVY));
}

void GenerativeMIDIEditor::paintContent(juce::Graphics& g)
{
    auto bounds = contentPanel.getLocalBounds().toFloat();

    juce::ColourGradient ground(
        juce::Colour(0xff1A1F27), bounds.getCentreX(), bounds.getY(),
        juce::Colour(CustomLookAndFeel::ABYSS_NAVY), bounds.getCentreX(), bounds.getBottom(),
        false);
    g.setGradientFill(ground);
    g.fillAll();

    auto drawPatternWell = [&g](juce::Rectangle<float> area)
    {
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillRoundedRectangle(area.translated(0.0f, 1.5f), 10.0f);

        g.setColour(juce::Colour(CustomLookAndFeel::ABYSS_NAVY));
        g.fillRoundedRectangle(area, 10.0f);

        g.setColour(juce::Colours::black.withAlpha(0.55f));
        g.drawRoundedRectangle(area.reduced(1.5f), 8.5f, 3.0f);

        g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.75f));
        g.drawRoundedRectangle(area.reduced(0.5f), 10.0f, 1.0f);
    };

    auto drawPlate = [&g](juce::Rectangle<float> area)
    {
        g.setColour(juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN));
        g.fillRoundedRectangle(area, 8.0f);

        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawLine(area.getX() + 10.0f, area.getY() + 1.0f, area.getRight() - 10.0f, area.getY() + 1.0f, 1.0f);

        g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.35f));
        g.drawRoundedRectangle(area.reduced(0.5f), 8.0f, 1.0f);
    };

    if (!patternPanelBounds.isEmpty())
        drawPatternWell(patternPanelBounds);

    juce::Rectangle<float> deckPanels[3] = { generatorPanelBounds, expressionPanelBounds, advancedPanelBounds };
    int deckCount = 0;
    juce::Rectangle<float> deck[3];
    for (const auto& panel : deckPanels)
        if (!panel.isEmpty() && panel.getHeight() > 48.0f)
            deck[deckCount++] = panel;

    const bool oneFace = deckCount >= 2
                         && std::abs(deck[0].getY() - deck[1].getY()) < 4.0f
                         && (deckCount < 3 || std::abs(deck[1].getY() - deck[2].getY()) < 4.0f);

    if (oneFace)
    {
        auto plate = deck[0];
        for (int i = 1; i < deckCount; ++i)
            plate = plate.getUnion(deck[i]);
        drawPlate(plate);

        std::sort(deck, deck + deckCount, [](const juce::Rectangle<float>& a, const juce::Rectangle<float>& b)
        {
            return a.getX() < b.getX();
        });
        g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.35f));
        const float ruleTop = plate.getY() + 28.0f;
        const float ruleBottom = plate.getBottom() - 10.0f;
        for (int i = 0; i < deckCount - 1; ++i)
        {
            const float x = (deck[i].getRight() + deck[i + 1].getX()) * 0.5f;
            g.drawLine(x, ruleTop, x, ruleBottom, 1.0f);
        }
    }
    else
    {
        if (!generatorPanelBounds.isEmpty())
            drawPlate(generatorPanelBounds);
        if (!expressionPanelBounds.isEmpty())
            drawPlate(expressionPanelBounds);
        if (!advancedPanelBounds.isEmpty())
            drawPlate(advancedPanelBounds);
    }

    if (!modulationPanelBounds.isEmpty())
        drawPlate(modulationPanelBounds);

    if (shapeExpanded && advancedDividerX1 > 0.0f)
    {
        const float bottom = advancedPanelBounds.getBottom() - 10.0f;
        const float top = juce::jmax(advancedPanelBounds.getY() + 28.0f, bottom - 108.0f);
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawLine(advancedDividerX1, top, advancedDividerX1, bottom, 1.0f);
    }
}

void GenerativeMIDIEditor::resized()
{
    auto bounds = getLocalBounds();
    const int logH = midiActivityPane.getPreferredHeight();
    midiActivityPane.setBounds(bounds.removeFromBottom(logH).reduced(8, 4));
    editorViewport.setBounds(bounds);

    const bool isPolyrhythm = GeneratorTypeMapping::isPolyrhythm(generatorTypeCombo.getSelectedId() - 1);
    const int viewH = juce::jmax(1, editorViewport.getHeight());
    const auto fitted = planSections(viewH, isPolyrhythm, editorViewport.getWidth());
    const int contentH = juce::jmax(viewH, fitted.used);
    const int scrollbarW = contentH > viewH ? editorViewport.getScrollBarThickness() : 0;
    const int contentW = juce::jmax(1, editorViewport.getWidth() - scrollbarW);
    contentPanel.setSize(contentW, contentH);
    // setSize no-ops when dimensions are unchanged (generator switch), so lay out explicitly.
    contentPanel.resized();
    contentPanel.repaint();
}

void GenerativeMIDIEditor::configureDisclosure(juce::TextButton& button, const juce::String& title, bool& expanded)
{
    contentPanel.addAndMakeVisible(button);
    button.setClickingTogglesState(true);
    button.setToggleState(expanded, juce::dontSendNotification);
    button.setTitle(title);
    button.setName(title);
    button.setComponentID("section-header");
    button.setButtonText(title);
    button.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    button.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    button.onClick = [this, &button, &expanded]
    {
        expanded = button.getToggleState();
        resized();
    };
}

GenerativeMIDIEditor::SectionPlan GenerativeMIDIEditor::planSections(int availableHeight, bool isPolyrhythm, int width) const
{
    juce::ignoreUnused(isPolyrhythm);
    const bool narrow = width > 0 && width < 900;
    const bool wrapMusical = musicalExpanded && narrow;
    const int performanceBody = narrow ? 100 : kPerformanceBody;
    const int musicalRow = narrow ? 96 : kMusicalBody;
    const int shapeBodyMin = narrow ? 168 : kShapeBodyMin;

    const auto panelHeight = [](bool open, int body)
    {
        return open ? (kPanelHeaderH + body) : kCollapsedPanelH;
    };

    const int openControls = (performanceExpanded ? 1 : 0) + (musicalExpanded ? 1 : 0) + (shapeExpanded ? 1 : 0);
    const bool sideBySide = width >= 1100 && openControls >= 2;

    SectionPlan plan;
    plan.header = kEditorHeaderH;
    plan.gap = narrow ? 6 : kSectionGap;
    plan.bottomPad = narrow ? 0 : kBottomPad;
    plan.sideBySide = sideBySide;
    plan.mod = narrow ? 156 : 84;

    if (sideBySide)
    {
        const int collapsedCount = 3 - openControls;
        const int gapCount = 1 + collapsedCount + (openControls > 0 ? 1 : 0);
        const int chrome = plan.header + plan.bottomPad + plan.gap * gapCount + plan.gap + plan.mod;
        const int deckFloor = openControls > 0 ? (kPanelHeaderH + 168) : 0;
        const int patternFloor = patternExpanded ? (kPanelHeaderH + 48) : kCollapsedPanelH;

        auto finish = [&](int patternH, int deckH)
        {
            plan.pattern = patternH;
            plan.deck = deckH;
            plan.performance = performanceExpanded ? deckH : kCollapsedPanelH;
            plan.musical = musicalExpanded ? deckH : kCollapsedPanelH;
            plan.shape = shapeExpanded ? deckH : kCollapsedPanelH;
            plan.used = chrome + patternH + collapsedCount * kCollapsedPanelH + deckH;
            plan.minimum = plan.used;
            return plan;
        };

        if (availableHeight <= 0)
            return finish(juce::jmax(patternFloor, panelHeight(true, 180)), deckFloor);

        const int flex = juce::jmax(0, availableHeight - chrome - collapsedCount * kCollapsedPanelH);
        int patternH = availableHeight * 42 / 100;
        patternH = juce::jlimit(patternFloor, juce::jmax(patternFloor, flex - deckFloor), patternH);
        const int half = juce::jmax(patternFloor, availableHeight / 2);
        if (patternH > half && flex - half >= deckFloor)
            patternH = half;
        return finish(patternH, flex - patternH);
    }

    plan.performance = panelHeight(performanceExpanded, performanceBody);
    const int musicalBody = wrapMusical ? (musicalRow * 2) : musicalRow;
    plan.musical = panelHeight(musicalExpanded, musicalBody);

    const int patternMin = patternExpanded
                               ? panelHeight(true, kPatternBodyMin)
                               : kCollapsedPanelH;
    const int shapeMin = panelHeight(shapeExpanded, shapeBodyMin);
    const int shapeMax = panelHeight(shapeExpanded, kShapeBodyMax);

    plan.minimum = plan.header + plan.bottomPad + plan.gap * 5
                   + patternMin + plan.performance + plan.musical + shapeMin + plan.mod;

    int surplus = juce::jmax(availableHeight, plan.minimum) - plan.minimum;
    int patternH = patternMin;
    int shapeH = shapeMin;
    const bool patternCanGrow = patternExpanded;

    if (patternCanGrow && shapeExpanded)
    {
        const int comfort = panelHeight(true, kPatternBodyComfort);
        const int toPattern = juce::jmin(surplus, juce::jmax(0, comfort - patternMin));
        patternH += toPattern;
        surplus -= toPattern;

        const int toShape = juce::jmin(surplus, juce::jmax(0, shapeMax - shapeMin));
        shapeH += toShape;
        surplus -= toShape;

        patternH += surplus;
    }
    else if (patternCanGrow)
    {
        patternH += surplus;
    }
    else if (shapeExpanded)
    {
        shapeH += juce::jmin(surplus, juce::jmax(0, shapeMax - shapeMin));
    }

    if (availableHeight > 0)
    {
        const auto usedNow = [&]()
        {
            return plan.header + plan.bottomPad + plan.gap * 5
                   + patternH + plan.performance + plan.musical + shapeH + plan.mod;
        };
        int overflow = usedNow() - availableHeight;
        if (overflow > 0)
        {
            const int patternFloor = patternExpanded ? (kPanelHeaderH + 40) : kCollapsedPanelH;
            const int patternCut = juce::jmin(overflow, juce::jmax(0, patternH - patternFloor));
            patternH -= patternCut;
            overflow -= patternCut;

            const int shapeFloor = shapeExpanded ? shapeMin : kCollapsedPanelH;
            const int shapeCut = juce::jmin(overflow, juce::jmax(0, shapeH - shapeFloor));
            shapeH -= shapeCut;
        }
    }

    plan.pattern = patternH;
    plan.shape = shapeH;
    plan.used = plan.header + plan.bottomPad + plan.gap * 5
                + plan.pattern + plan.performance + plan.musical + plan.shape + plan.mod;
    return plan;
}

int GenerativeMIDIEditor::preferredContentHeight(bool isPolyrhythm, int width) const
{
    return planSections(0, isPolyrhythm, width).minimum;
}

void GenerativeMIDIEditor::layoutContent(juce::Rectangle<int> area)
{
    const int contentW = area.getWidth();
    const int inset = contentW < 900 ? 12 : 24;
    const int genIndex = generatorTypeCombo.getSelectedId() - 1;
    const bool showEuclideanKnobs = (genIndex == GeneratorTypeMapping::kEuclidean);
    const bool showStochasticKnobs = GeneratorTypeMapping::isStochastic(genIndex);
    const bool showMarkovKnobs = (genIndex == GeneratorTypeMapping::kMarkov);
    const bool showLSystemKnobs = (genIndex == GeneratorTypeMapping::kLSystem);
    const bool showCellularKnobs = (genIndex == GeneratorTypeMapping::kCellular);
    const bool isPolyrhythm = GeneratorTypeMapping::isPolyrhythm(genIndex);

    auto placeKnob = [](juce::Rectangle<int> col, juce::Slider& slider, juce::Label& label, int maxW = 84)
    {
        if (slider.getSliderStyle() == juce::Slider::RotaryHorizontalVerticalDrag)
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, kReadoutW, kReadoutH);
        const int blockW = juce::jmin(col.getWidth(), maxW);
        const int blockH = juce::jmin(col.getHeight(), 96);
        auto block = col.withSizeKeepingCentre(blockW, blockH);
        label.setBounds(block.removeFromBottom(16));
        slider.setBounds(block.reduced(2, 0));
    };

    // Toggle sits in the dial band of a knob cell, so it lines up with neighbouring knobs.
    auto placeToggleInKnobBand = [](juce::Rectangle<int> col, juce::Button& button, int btnW, int btnH)
    {
        auto dial = col;
        if (dial.getHeight() > 36)
            dial.removeFromBottom(juce::jmin(32, dial.getHeight() / 3));
        const int w = juce::jmin(btnW, juce::jmax(40, dial.getWidth() - 4));
        const int h = juce::jmin(btnH, juce::jmax(22, juce::jmin(28, dial.getHeight() - 4)));
        button.setBounds(dial.withSizeKeepingCentre(w, h));
    };

    const auto plan = planSections(area.getHeight(), isPolyrhythm, contentW);
    auto insetX = [inset](juce::Rectangle<int> row)
    {
        return row.withTrimmedLeft(inset).withTrimmedRight(inset);
    };

    auto setShapeFamilyVisible = [this](bool show)
    {
        aftertouchEnableButton.setVisible(show);
        aftertouchAmountSlider.setVisible(show);
        aftertouchAmountLabel.setVisible(show);
        pitchbendEnableButton.setVisible(show);
        pitchbendRangeSlider.setVisible(show);
        pitchbendRangeLabel.setVisible(show);
        ccEnableButton.setVisible(show);
        ccNumberSlider.setVisible(show);
        ccNumberLabel.setVisible(show);
        ccAmountSlider.setVisible(show);
        ccAmountLabel.setVisible(show);
        ratchetCountSlider.setVisible(show);
        ratchetProbabilitySlider.setVisible(show);
        ratchetDecaySlider.setVisible(show);
        ratchetCountLabel.setVisible(show);
        ratchetProbabilityLabel.setVisible(show);
        ratchetDecayLabel.setVisible(show);
        advancedRatchetGroupLabel.setVisible(show);
        advancedStochasticGroupLabel.setVisible(show);
        advancedLfoGroupLabel.setVisible(false);
    };

    // Header: one row. Product and transport on the left. Generator, channel, and preset on the right.
    titleLabel.setVisible(false);
    auto header = area.removeFromTop(plan.header).reduced(inset, 8);
    const int controlH = juce::jmin(26, header.getHeight());
    auto presetComboW = juce::jlimit(120, 200, contentW / 6);
    auto presetCol = header.removeFromRight(72 + 8 + presetComboW + (currentPresetLabel.getText() == "Edited" ? 56 : 0));
    if (currentPresetLabel.getText() == "Edited")
        currentPresetLabel.setBounds(presetCol.removeFromLeft(52).withSizeKeepingCentre(52, controlH));
    else
        currentPresetLabel.setBounds(0, 0, 0, 0);
    presetBrowserButton.setBounds(presetCol.removeFromLeft(72).withSizeKeepingCentre(70, controlH));
    presetCol.removeFromLeft(6);
    presetCombo.setBounds(presetCol.withSizeKeepingCentre(presetCol.getWidth(), controlH));

    header.removeFromRight(8);
    const int chW = contentW < 900 ? 52 : 64;
    auto chCol = header.removeFromRight(36 + chW);
    midiChannelLabel.setBounds(chCol.removeFromLeft(36).withSizeKeepingCentre(34, controlH));
    midiChannelCombo.setBounds(chCol.withSizeKeepingCentre(chCol.getWidth(), controlH));

    header.removeFromRight(8);
    const int partsW = contentW < 900 ? 40 : 48;
    auto partsCol = header.removeFromRight(44 + partsW);
    partCountLabel.setBounds(partsCol.removeFromLeft(44).withSizeKeepingCentre(42, controlH));
    partCountCombo.setBounds(partsCol.withSizeKeepingCentre(partsCol.getWidth(), controlH));

    header.removeFromRight(8);
    const int genW = contentW < 900 ? 112 : 156;
    auto genCol = header.removeFromRight(72 + genW);
    generatorLabel.setBounds(genCol.removeFromLeft(72).withSizeKeepingCentre(70, controlH));
    generatorTypeCombo.setBounds(genCol.withSizeKeepingCentre(genCol.getWidth(), controlH));

    auto brand = header.withSizeKeepingCentre(header.getWidth(), controlH);
    productLabel.setBounds(brand.removeFromLeft(juce::jmin(150, brand.getWidth() / 2)));
    if (pianoButton.isVisible())
    {
        pianoButton.setBounds(brand.removeFromLeft(64).withSizeKeepingCentre(60, controlH));
        brand.removeFromLeft(6);
    }
    else
    {
        pianoButton.setBounds(0, 0, 0, 0);
    }
    statusChipLabel.setBounds(brand);

    area.removeFromTop(plan.gap);

    auto patternSection = insetX(area.removeFromTop(plan.pattern));
    patternPanelBounds = patternSection.toFloat();
    placeDisclosure(patternSection, patternButton);
    if (patternExpanded)
    {
        patternSection.removeFromTop(kPanelHeaderH);
        patternSection = patternSection.reduced(8, 0);
        patternDisplay.setBounds(patternSection);
        patternDisplay.setVisible(!isPolyrhythm);
        if (polyLayerEditor)
        {
            polyLayerEditor->setBounds(patternSection);
            polyLayerEditor->setVisible(isPolyrhythm);
        }
    }
    else
    {
        patternDisplay.setVisible(false);
        if (polyLayerEditor)
            polyLayerEditor->setVisible(false);
    }

    juce::Rectangle<int> perfSlot;
    juce::Rectangle<int> musicalSlot;
    juce::Rectangle<int> shapeSlot;
    if (plan.sideBySide)
    {
        auto takeBar = [&](bool open, juce::Rectangle<int>& slot)
        {
            if (open)
                return;
            area.removeFromTop(plan.gap);
            slot = insetX(area.removeFromTop(kCollapsedPanelH));
        };
        takeBar(performanceExpanded, perfSlot);
        takeBar(musicalExpanded, musicalSlot);
        takeBar(shapeExpanded, shapeSlot);

        if ((performanceExpanded ? 1 : 0) + (musicalExpanded ? 1 : 0) + (shapeExpanded ? 1 : 0) > 0)
        {
            area.removeFromTop(plan.gap);
            auto row = insetX(area.removeFromTop(plan.deck));
            const int gutter = 8;
            int weightLeft = (performanceExpanded ? 22 : 0) + (musicalExpanded ? 34 : 0) + (shapeExpanded ? 44 : 0);
            int columnsLeft = (performanceExpanded ? 1 : 0) + (musicalExpanded ? 1 : 0) + (shapeExpanded ? 1 : 0);
            auto takeCol = [&](int weight)
            {
                const int guttersAfter = juce::jmax(0, columnsLeft - 1) * gutter;
                const int width = (row.getWidth() - guttersAfter) * weight / juce::jmax(1, weightLeft);
                weightLeft -= weight;
                --columnsLeft;
                auto col = row.removeFromLeft(width);
                if (columnsLeft > 0)
                    row.removeFromLeft(gutter);
                return col;
            };
            if (performanceExpanded)
                perfSlot = takeCol(22);
            if (musicalExpanded)
                musicalSlot = takeCol(34);
            if (shapeExpanded)
                shapeSlot = takeCol(44);
        }
    }

    if (!plan.sideBySide)
        area.removeFromTop(plan.gap);

    auto primary = plan.sideBySide ? perfSlot : insetX(area.removeFromTop(plan.performance));
    generatorPanelBounds = primary.toFloat();
    placeDisclosure(primary, performanceButton);

    juce::Component* perfWidgets[] = {
        &tempoSlider, &tempoLabel, &stepsSlider, &stepsLabel, &pulsesSlider, &pulsesLabel,
        &rotationSlider, &rotationLabel, &densitySlider, &densityLabel,
        &stepSizeSlider, &stepSizeLabel, &momentumSlider, &momentumLabel, &timeScaleSlider, &timeScaleLabel,
        &markovOrderSlider, &markovOrderLabel, &markovStepSlider, &markovStepLabel,
        &markovSurpriseSlider, &markovSurpriseLabel,
        &lsystemGrammarSlider, &lsystemGrammarLabel, &lsystemGenerationSlider, &lsystemGenerationLabel,
        &lsystemIntervalSlider, &lsystemIntervalLabel,
        &cellularRuleSlider, &cellularRuleLabel, &cellularSeedSlider, &cellularSeedLabel,
        &cellularListenSlider, &cellularListenLabel
    };

    if (!performanceExpanded)
    {
        setShown(perfWidgets, false);
    }
    else
    {
    primary.removeFromTop(kPanelHeaderH);
    primary = primary.reduced(8, 0);

    tempoSlider.setVisible(true);
    tempoLabel.setVisible(true);
    densitySlider.setVisible(true);
    densityLabel.setVisible(true);
    stepsSlider.setVisible(showEuclideanKnobs);
    pulsesSlider.setVisible(showEuclideanKnobs);
    rotationSlider.setVisible(showEuclideanKnobs);
    stepsLabel.setVisible(showEuclideanKnobs);
    pulsesLabel.setVisible(showEuclideanKnobs);
    rotationLabel.setVisible(showEuclideanKnobs);
    stepSizeSlider.setVisible(showStochasticKnobs);
    momentumSlider.setVisible(showStochasticKnobs);
    timeScaleSlider.setVisible(showStochasticKnobs);
    stepSizeLabel.setVisible(showStochasticKnobs);
    momentumLabel.setVisible(showStochasticKnobs);
    timeScaleLabel.setVisible(showStochasticKnobs);
    markovOrderSlider.setVisible(showMarkovKnobs);
    markovStepSlider.setVisible(showMarkovKnobs);
    markovSurpriseSlider.setVisible(showMarkovKnobs);
    markovOrderLabel.setVisible(showMarkovKnobs);
    markovStepLabel.setVisible(showMarkovKnobs);
    markovSurpriseLabel.setVisible(showMarkovKnobs);
    lsystemGrammarSlider.setVisible(showLSystemKnobs);
    lsystemGenerationSlider.setVisible(showLSystemKnobs);
    lsystemIntervalSlider.setVisible(showLSystemKnobs);
    lsystemGrammarLabel.setVisible(showLSystemKnobs);
    lsystemGenerationLabel.setVisible(showLSystemKnobs);
    lsystemIntervalLabel.setVisible(showLSystemKnobs);
    cellularRuleSlider.setVisible(showCellularKnobs);
    cellularSeedSlider.setVisible(showCellularKnobs);
    cellularListenSlider.setVisible(showCellularKnobs);
    cellularRuleLabel.setVisible(showCellularKnobs);
    cellularSeedLabel.setVisible(showCellularKnobs);
    cellularListenLabel.setVisible(showCellularKnobs);

    juce::Array<juce::Component*> perfSliders;
    juce::Array<juce::Component*> perfLabels;
    auto queuePerf = [&](bool show, juce::Slider& slider, juce::Label& label)
    {
        if (!show)
            return;
        perfSliders.add(&slider);
        perfLabels.add(&label);
    };
    queuePerf(true, tempoSlider, tempoLabel);
    queuePerf(showEuclideanKnobs, stepsSlider, stepsLabel);
    queuePerf(showEuclideanKnobs, pulsesSlider, pulsesLabel);
    queuePerf(showEuclideanKnobs, rotationSlider, rotationLabel);
    queuePerf(showStochasticKnobs, stepSizeSlider, stepSizeLabel);
    queuePerf(showStochasticKnobs, momentumSlider, momentumLabel);
    queuePerf(showStochasticKnobs, timeScaleSlider, timeScaleLabel);
    queuePerf(showMarkovKnobs, markovOrderSlider, markovOrderLabel);
    queuePerf(showMarkovKnobs, markovStepSlider, markovStepLabel);
    queuePerf(showMarkovKnobs, markovSurpriseSlider, markovSurpriseLabel);
    queuePerf(showLSystemKnobs, lsystemGrammarSlider, lsystemGrammarLabel);
    queuePerf(showLSystemKnobs, lsystemGenerationSlider, lsystemGenerationLabel);
    queuePerf(showLSystemKnobs, lsystemIntervalSlider, lsystemIntervalLabel);
    queuePerf(showCellularKnobs, cellularRuleSlider, cellularRuleLabel);
    queuePerf(showCellularKnobs, cellularSeedSlider, cellularSeedLabel);
    queuePerf(showCellularKnobs, cellularListenSlider, cellularListenLabel);
    queuePerf(true, densitySlider, densityLabel);

    const int perfCount = perfSliders.size();
    if (perfCount > 0)
    {
        int knobW = 80;
        if (perfCount >= 5 && primary.getWidth() < perfCount * knobW && primary.getWidth() >= 3 * 74)
            knobW = juce::jmax(74, primary.getWidth() / 3);
        int cols = juce::jmax(1, juce::jmin(perfCount, primary.getWidth() / knobW));
        if (perfCount >= 5 && primary.getWidth() >= 3 * 74)
            cols = juce::jmin(perfCount, juce::jmax(cols, 3));
        const int rows = (perfCount + cols - 1) / cols;
        const int rowH = juce::jmin(100, juce::jmax(1, primary.getHeight() / rows));
        const int gridH = rowH * rows;
        const int y0 = primary.getY() + juce::jmax(0, (primary.getHeight() - gridH) / 2);
        for (int i = 0; i < perfCount; ++i)
        {
            const int r = i / cols;
            const int c = i % cols;
            const int inRow = juce::jmin(cols, perfCount - r * cols);
            const int rowX = primary.getX() + (primary.getWidth() - inRow * knobW) / 2;
            auto cell = juce::Rectangle<int>(rowX + c * knobW, y0 + r * rowH, knobW, rowH).reduced(2, 2);
            placeKnob(cell, *static_cast<juce::Slider*>(perfSliders[i]),
                      *static_cast<juce::Label*>(perfLabels[i]), knobW - 4);
        }
    }
    }

    if (!plan.sideBySide)
        area.removeFromTop(plan.gap);

    auto musical = plan.sideBySide ? musicalSlot : insetX(area.removeFromTop(plan.musical));
    expressionPanelBounds = musical.toFloat();
    placeDisclosure(musical, musicalButton);

    juce::Component* musicalWidgets[] = {
        &velocityMinSlider, &velocityMaxSlider, &velocityLabel, &velocityMinCaption, &velocityMaxCaption,
        &pitchMinSlider, &pitchMaxSlider, &pitchLabel, &pitchMinCaption, &pitchMaxCaption,
        &scaleLabel, &scaleRootCombo, &scaleTypeCombo,
        &swingSlider, &swingLabel, &timingHumanizeSlider, &timingHumanizeLabel,
        &velocityHumanizeSlider, &velocityHumanizeLabel,
        &gateLengthSlider, &gateLengthLabel, &voiceModeCombo, &legatoButton, &legatoLabel
    };
    setShown(musicalWidgets, musicalExpanded);

    if (musicalExpanded)
    {
    musical.removeFromTop(kPanelHeaderH);
    musical = musical.reduced(8, 0);

    const bool wrapMusical = musical.getWidth() < 780;
    // Centre the two musical bands on a fixed rhythm so faders do not stretch.
    juce::Rectangle<int> musicalTop;
    juce::Rectangle<int> musicalBottom;
    if (wrapMusical)
    {
        const int rangeH = juce::jmin(112, musical.getHeight() / 2);
        const int knobH = juce::jmin(100, juce::jmax(72, musical.getHeight() - rangeH));
        auto block = musical.withSizeKeepingCentre(musical.getWidth(), juce::jmin(musical.getHeight(), rangeH + knobH));
        musicalTop = block.removeFromTop(rangeH);
        musicalBottom = block;
    }
    else
    {
        musicalTop = musical;
    }

    auto layoutRange = [](juce::Rectangle<int> slot, juce::Label& title, juce::Label& minCaption,
                          juce::Slider& minSlider, juce::Label& maxCaption, juce::Slider& maxSlider)
    {
        const int blockH = juce::jmin(slot.getHeight(), 112);
        auto block = slot.withSizeKeepingCentre(slot.getWidth(), blockH);
        title.setBounds(block.removeFromTop(14));
        auto captions = block.removeFromTop(14);
        const int half = block.getWidth() / 2;
        minCaption.setBounds(captions.removeFromLeft(half));
        maxCaption.setBounds(captions);
        minSlider.setBounds(block.removeFromLeft(half).reduced(4, 0));
        maxSlider.setBounds(block.reduced(4, 0));
    };

    auto layoutScale = [](juce::Rectangle<int> slot, juce::Label& title, juce::ComboBox& root, juce::ComboBox& type)
    {
        const int blockH = juce::jmin(slot.getHeight(), 112);
        auto block = slot.withSizeKeepingCentre(slot.getWidth(), blockH);
        title.setBounds(block.removeFromTop(14));
        block.removeFromTop(8);
        const int comboH = 26;
        root.setBounds(block.removeFromTop(comboH).reduced(2, 0));
        block.removeFromTop(6);
        type.setBounds(block.removeFromTop(comboH).reduced(2, 0));
    };

    auto layoutLegato = [](juce::Rectangle<int> col, juce::ComboBox& voice, juce::TextButton& button, juce::Label& label)
    {
        label.setBounds(col.removeFromBottom(16));
        const int voiceH = juce::jmin(26, juce::jmax(22, col.getHeight() / 2));
        voice.setBounds(col.removeFromTop(voiceH).reduced(2, 1));
        const int w = juce::jmin(76, juce::jmax(48, col.getWidth() - 8));
        const int h = juce::jmin(28, juce::jmax(22, col.getHeight() - 4));
        button.setBounds(col.withSizeKeepingCentre(w, h));
    };

    auto fillRow = [&](juce::Rectangle<int> row, bool rangesAndScale)
    {
        const int weights[] = { 18, 18, 16, 12, 12, 12, 12, 14 };
        const int first = rangesAndScale ? 0 : 3;
        const int count = rangesAndScale ? (wrapMusical ? 3 : 8) : 5;
        int left = 0;
        for (int i = 0; i < count; ++i)
            left += weights[first + i];
        auto take = [&](int weight)
        {
            const int width = row.getWidth() * weight / juce::jmax(1, left);
            left -= weight;
            return row.removeFromLeft(width);
        };

        if (rangesAndScale)
        {
            layoutRange(take(weights[0]).reduced(4, 0), velocityLabel, velocityMinCaption,
                        velocityMinSlider, velocityMaxCaption, velocityMaxSlider);
            layoutRange(take(weights[1]).reduced(4, 0), pitchLabel, pitchMinCaption,
                        pitchMinSlider, pitchMaxCaption, pitchMaxSlider);
            layoutScale(take(weights[2]).reduced(4, 0), scaleLabel, scaleRootCombo, scaleTypeCombo);
        }
        if (!rangesAndScale || !wrapMusical)
        {
            placeKnob(take(weights[3]), swingSlider, swingLabel);
            placeKnob(take(weights[4]), timingHumanizeSlider, timingHumanizeLabel);
            placeKnob(take(weights[5]), velocityHumanizeSlider, velocityHumanizeLabel);
            placeKnob(take(weights[6]), gateLengthSlider, gateLengthLabel);
            layoutLegato(take(weights[7]), voiceModeCombo, legatoButton, legatoLabel);
        }
    };

    fillRow(musicalTop, true);
    if (wrapMusical)
        fillRow(musicalBottom, false);
    }

    if (!plan.sideBySide)
        area.removeFromTop(plan.gap);

    auto shape = plan.sideBySide ? shapeSlot : insetX(area.removeFromTop(plan.shape));
    advancedPanelBounds = shape.toFloat();
    placeDisclosure(shape, shapeButton);
    setShapeFamilyVisible(shapeExpanded);

    if (!shapeExpanded)
    {
        advancedDividerX1 = 0.0f;
        advancedDividerX2 = 0.0f;
    }
    else
    {
    shape.removeFromTop(kPanelHeaderH);
    shape = shape.reduced(8, 0);

#if JUCE_IOS
    const int midiBtnH = 36;
    const int midiBtnW = 64;
#else
    const int midiBtnH = 30;
    const int midiBtnW = 52;
#endif
    const int exprKnobW = 76;
    const int exprGap = 8;

    auto layoutExprGroup = [&](juce::Rectangle<int> group, juce::Button& toggle, juce::Slider& amount,
                               juce::Label& amountLabel, juce::Slider* extra, juce::Label* extraLabel)
    {
        const int knobs = extra != nullptr ? 2 : 1;
        const int used = midiBtnW + exprGap + exprKnobW * knobs + exprGap * (knobs - 1);
        if (group.getWidth() > used)
            group = group.withSizeKeepingCentre(used, group.getHeight());

        placeToggleInKnobBand(group.removeFromLeft(midiBtnW), toggle, midiBtnW - 4, midiBtnH);
        group.removeFromLeft(exprGap);
        placeKnob(group.removeFromLeft(exprKnobW), amount, amountLabel, exprKnobW);
        if (extra != nullptr && extraLabel != nullptr)
        {
            group.removeFromLeft(exprGap);
            placeKnob(group.removeFromLeft(exprKnobW), *extra, *extraLabel, exprKnobW);
        }
    };

    auto placeThree = [&](juce::Rectangle<int> row, juce::Slider& a, juce::Label& aLabel,
                          juce::Slider& b, juce::Label& bLabel, juce::Slider& c, juce::Label& cLabel)
    {
        const int col = row.getWidth() / 3;
        placeKnob(row.removeFromLeft(col), a, aLabel, exprKnobW);
        placeKnob(row.removeFromLeft(col), b, bLabel, exprKnobW);
        placeKnob(row, c, cLabel, exprKnobW);
    };

    advancedDividerX1 = 0.0f;
    advancedDividerX2 = 0.0f;

    auto placeRowCaption = [](juce::Rectangle<int>& row, juce::Label& label)
    {
        auto cap = row.removeFromTop(14);
        label.setBounds(cap.removeFromLeft(juce::jmin(88, cap.getWidth())));
    };

    const bool exprOnOneRow = shape.getWidth() >= 500;
    const int shapeRows = exprOnOneRow ? 2 : 3;
    const int shapeRowH = juce::jmin(96, juce::jmax(64, shape.getHeight() / shapeRows));
    auto shapeBlock = shape.withSizeKeepingCentre(shape.getWidth(), juce::jmin(shape.getHeight(), shapeRowH * shapeRows));
    auto takeShapeRow = [&]()
    {
        return shapeBlock.removeFromTop(shapeRowH);
    };

    auto layoutRatchet = [&](juce::Rectangle<int> row)
    {
        placeRowCaption(row, advancedRatchetGroupLabel);
        placeThree(row, ratchetCountSlider, ratchetCountLabel,
                   ratchetProbabilitySlider, ratchetProbabilityLabel,
                   ratchetDecaySlider, ratchetDecayLabel);
    };

    if (exprOnOneRow)
    {
        auto expr = takeShapeRow();
        placeRowCaption(expr, advancedStochasticGroupLabel);
        const int atW = midiBtnW + exprGap + exprKnobW;
        const int ccW = midiBtnW + exprGap + exprKnobW * 2 + exprGap;
        const int packed = atW * 2 + ccW + exprGap * 2;
        if (expr.getWidth() > packed)
            expr = expr.withSizeKeepingCentre(packed, expr.getHeight());
        layoutExprGroup(expr.removeFromLeft(atW), aftertouchEnableButton,
                        aftertouchAmountSlider, aftertouchAmountLabel, nullptr, nullptr);
        expr.removeFromLeft(exprGap);
        layoutExprGroup(expr.removeFromLeft(atW), pitchbendEnableButton,
                        pitchbendRangeSlider, pitchbendRangeLabel, nullptr, nullptr);
        expr.removeFromLeft(exprGap);
        layoutExprGroup(expr, ccEnableButton, ccNumberSlider, ccNumberLabel,
                        &ccAmountSlider, &ccAmountLabel);

        if (shapeRows == 2)
            layoutRatchet(takeShapeRow());
    }
    else
    {
        auto exprTop = takeShapeRow();
        placeRowCaption(exprTop, advancedStochasticGroupLabel);
        const int half = exprTop.getWidth() / 2;
        layoutExprGroup(exprTop.removeFromLeft(half).reduced(2, 0), aftertouchEnableButton,
                        aftertouchAmountSlider, aftertouchAmountLabel, nullptr, nullptr);
        layoutExprGroup(exprTop.reduced(2, 0), pitchbendEnableButton,
                        pitchbendRangeSlider, pitchbendRangeLabel, nullptr, nullptr);
        layoutExprGroup(takeShapeRow().reduced(4, 0), ccEnableButton, ccNumberSlider, ccNumberLabel,
                        &ccAmountSlider, &ccAmountLabel);
        layoutRatchet(takeShapeRow());
    }
    }

    juce::Component* modWidgets[] = {
        &modLfoEnableButton, &modLfoRateSlider, &modLfoRateLabel,
        &modShRateSlider, &modShRateLabel,
        &modLfoDepthSlider, &modLfoDepthLabel,
        &modLfoDensityDepthSlider, &modLfoDensityDepthLabel,
        &modRoute3SourceCombo, &modRoute3DestCombo, &modRoute3AmountSlider, &modRoute3AmountLabel,
        &modRoute4SourceCombo, &modRoute4DestCombo, &modRoute4AmountSlider, &modRoute4AmountLabel
    };
    setShown(modWidgets, true);

    if (area.getHeight() > plan.mod)
        area.removeFromTop(juce::jmin(plan.gap, area.getHeight() - plan.mod));
    auto modSection = insetX(area.removeFromTop(juce::jmin(plan.mod, juce::jmax(0, area.getHeight()))));
    modulationPanelBounds = modSection.toFloat();
    auto modRow = modSection.reduced(8, 4);

    auto layoutSources = [&](juce::Rectangle<int> row)
    {
        placeToggleInKnobBand(row.removeFromLeft(juce::jmin(64, juce::jmax(40, row.getWidth() / 3))),
                              modLfoEnableButton, 56, 28);
        row.removeFromLeft(4);
        const int col = juce::jmax(1, row.getWidth() / 2);
        placeKnob(row.removeFromLeft(col), modLfoRateSlider, modLfoRateLabel, 76);
        placeKnob(row, modShRateSlider, modShRateLabel, 76);
    };
    auto layoutFree = [&](juce::Rectangle<int> cell, juce::ComboBox& source, juce::ComboBox& dest,
                          juce::Slider& amount, juce::Label& label)
    {
        auto combos = cell.removeFromTop(juce::jmin(22, juce::jmax(16, cell.getHeight() / 3)));
        source.setBounds(combos.removeFromLeft(combos.getWidth() / 2).reduced(2, 0));
        dest.setBounds(combos.reduced(2, 0));
        placeKnob(cell, amount, label, 72);
    };
    auto layoutRoutes = [&](juce::Rectangle<int> row)
    {
        const int cellW = juce::jmax(1, row.getWidth() / 4);
        placeKnob(row.removeFromLeft(cellW), modLfoDepthSlider, modLfoDepthLabel, cellW);
        placeKnob(row.removeFromLeft(cellW), modLfoDensityDepthSlider, modLfoDensityDepthLabel, cellW);
        layoutFree(row.removeFromLeft(cellW), modRoute3SourceCombo, modRoute3DestCombo,
                   modRoute3AmountSlider, modRoute3AmountLabel);
        layoutFree(row, modRoute4SourceCombo, modRoute4DestCombo,
                   modRoute4AmountSlider, modRoute4AmountLabel);
    };

    if (plan.mod > 100)
    {
        auto top = modRow.removeFromTop(modRow.getHeight() / 2);
        layoutSources(top);
        layoutRoutes(modRow);
    }
    else
    {
        auto sources = modRow.removeFromLeft(juce::jmin(230, juce::jmax(120, modRow.getWidth() / 4)));
        layoutSources(sources);
        layoutRoutes(modRow);
    }

    patternButton.toFront(false);
    performanceButton.toFront(false);
    musicalButton.toFront(false);
    shapeButton.toFront(false);
}

void GenerativeMIDIEditor::feedSoundingNotes(int generatorType, int partStep, int pitchMin, int pitchMax)
{
    pitchMin = juce::jlimit(0, 126, pitchMin);
    pitchMax = juce::jlimit(pitchMin + 1, 127, pitchMax);
    auto& scale = audioProcessor.getScaleQuantizer();
    auto quantize = [&](int note)
    {
        return scale.quantize(juce::jlimit(0, 127, note));
    };

    if (generatorType != melodyFeedGenerator)
    {
        melodyFeedGenerator = generatorType;
        melodyFeedStep = -2;
        lSystemFeedNote = 0;
        for (int i = 0; i < 16; ++i)
            polyLayerFeedSeen[i] = false;
    }

    const bool stepChanged = partStep >= 0 && partStep != melodyFeedStep;
    if (partStep >= 0)
        melodyFeedStep = partStep;

    if (generatorType == GeneratorTypeMapping::kEuclidean && stepChanged)
    {
        auto& euclidean = audioProcessor.getEuclideanEngine();
        const int steps = juce::jmax(1, euclidean.getSteps());
        const int step = partStep % steps;
        if (euclidean.getStep(step))
        {
            const int range = pitchMax - pitchMin;
            patternDisplay.noteMelody(quantize(pitchMin + (step % (range + 1))));
        }
    }
    else if (generatorType == GeneratorTypeMapping::kPolyrhythm)
    {
        auto& engine = audioProcessor.getPolyrhythmEngine();
        const int layers = juce::jmin(16, engine.getNumLayers());
        for (int i = 0; i < layers; ++i)
        {
            auto* layer = engine.getLayer(i);
            if (layer == nullptr || !layer->enabled || layer->length <= 0)
                continue;
            const int curStep = engine.getCurrentStep(i);
            if (!polyLayerFeedSeen[i])
            {
                polyLayerFeedSeen[i] = true;
                polyLayerFeedStep[i] = curStep;
                continue;
            }
            if (curStep == polyLayerFeedStep[i])
                continue;

            const int length = juce::jmax(1, layer->length);
            const int played = (curStep - 1 + length) % length;
            polyLayerFeedStep[i] = curStep;
            if (played >= static_cast<int>(layer->pattern.size())
                || played >= static_cast<int>(layer->pitches.size())
                || !layer->pattern[static_cast<size_t>(played)])
                continue;

            const int raw = juce::jlimit(pitchMin, pitchMax,
                                         layer->pitches[static_cast<size_t>(played)] + layer->pitchOffset);
            patternDisplay.noteMelody(quantize(raw));
        }
    }
    else if (generatorType == GeneratorTypeMapping::kLSystem && stepChanged && !lSystemRows.isEmpty())
    {
        const auto& text = lSystemRows[lSystemRows.size() - 1];
        const int length = text.length();
        const int interval = juce::jmax(1, static_cast<int>(lsystemIntervalSlider.getValue()));
        int noteCount = 0;
        for (int i = 0; i < length; ++i)
        {
            const auto ch = text[i];
            if (ch >= 'A' && ch <= 'D')
                ++noteCount;
        }

        if (noteCount > 0)
        {
            const int want = lSystemFeedNote % noteCount;
            int cursor = pitchMin + juce::jmax(0, pitchMax - pitchMin) / 2;
            int seen = 0;
            int found = -1;
            for (int i = 0; i < length; ++i)
            {
                const auto ch = text[i];
                int emitted = -1;
                switch (ch)
                {
                    case 'A': emitted = cursor; break;
                    case 'B': emitted = cursor + interval; break;
                    case 'C': emitted = cursor + interval * 2; break;
                    case 'D': emitted = cursor + interval * 3; break;
                    case '+': cursor = juce::jlimit(0, 127, cursor + 12); break;
                    case '-': cursor = juce::jlimit(0, 127, cursor - 12); break;
                    case '[': cursor = juce::jlimit(0, 127, cursor + 1); break;
                    case ']': cursor = juce::jlimit(0, 127, cursor - 1); break;
                    default: break;
                }
                if (emitted < 0)
                    continue;
                if (seen == want)
                {
                    found = juce::jlimit(pitchMin, pitchMax, emitted);
                    break;
                }
                ++seen;
            }
            if (found >= 0)
                patternDisplay.noteMelody(quantize(found));
            lSystemFeedNote = want + 1;
        }
    }
    else if (stepChanged
             && (generatorType == GeneratorTypeMapping::kBrownian
                 || generatorType == GeneratorTypeMapping::kPerlin
                 || generatorType == GeneratorTypeMapping::kDrunkWalk
                 || generatorType == GeneratorTypeMapping::kLorenz))
    {
        patternDisplay.noteMelody(quantize(audioProcessor.getStochasticEngine().getCurrentPitch(pitchMin, pitchMax)));
    }

    int newest = patternDisplay.newestMelody();
    if (newest < 0)
        newest = juce::jlimit(0, 127, 60 + scale.getRootNote());

    const bool chromatic = scale.getScale() == ScaleQuantizer::Scale::Chromatic;
    int intervals[12];
    int intervalCount = 0;
    if (!chromatic)
    {
        const auto& src = scale.getScaleIntervals();
        intervalCount = juce::jmin(12, static_cast<int>(src.size()));
        for (int i = 0; i < intervalCount; ++i)
            intervals[i] = src[static_cast<size_t>(i)];
    }

    const auto triad = HarmonyParts::triadForMelody(
        newest, scale.getRootNote(), intervals, intervalCount, chromatic);
    patternDisplay.setPitchSpan(pitchMin, pitchMax);
    patternDisplay.setHarmony(triad.root, triad.third, triad.fifth);

    if (polyLayerEditor != nullptr)
    {
        PatternVisualizer::StackMark marks[16];
        const int n = patternDisplay.fillStack(marks, 16);
        polyLayerEditor->setStack(marks, n, pitchMin, pitchMax);
    }
}

void GenerativeMIDIEditor::timerCallback()
{
    const int generatorType = generatorTypeCombo.getSelectedId() - 1;

    const uint32_t noteCount = audioProcessor.getNoteActivityCount();
    if (noteCount != lastNoteActivityCount)
    {
        activityPulse = 1.0f;
        lastNoteActivityCount = noteCount;
    }
    else
    {
        activityPulse = juce::jmax(0.0f, activityPulse - 0.08f);
    }

    const float pitchMin = static_cast<float>(pitchMinSlider.getValue());
    const float pitchMax = juce::jmax(pitchMin + 1.0f, static_cast<float>(pitchMaxSlider.getValue()));
    auto normPitch = [pitchMin, pitchMax](float note)
    {
        return juce::jlimit(0.0f, 1.0f, (note - pitchMin) / (pitchMax - pitchMin));
    };

    patternDisplay.setGenerator(generatorType);
    patternDisplay.setShowDensity(false);
    const bool monoVoice = voiceModeCombo.getSelectedItemIndex() == 1;
    const float gateLength = static_cast<float>(gateLengthSlider.getValue());
    patternDisplay.setVoice(monoVoice, gateLength);
    const int partCount = juce::jlimit(1, 4, partCountCombo.getSelectedItemIndex() + 1);
    const int timeNum = juce::jmax(1, static_cast<int>(audioProcessor.getValueTreeState().getRawParameterValue("timeSigNum")->load()));
    const int timeDen = juce::jmax(1, static_cast<int>(audioProcessor.getValueTreeState().getRawParameterValue("timeSigDenom")->load()));
    const int barSixteenths = juce::jmax(1, timeNum * 16 / timeDen);
    const int partStep = audioProcessor.isClockAdvancing()
                             ? juce::jmax(0, audioProcessor.getCurrentStep() - 1)
                             : -1;
    const float tempo = juce::jmax(20.0f, static_cast<float>(tempoSlider.getValue()));
    const float gateSeconds = gateLength * (60.0f / tempo) / 4.0f;
    patternDisplay.setParts(partCount, partStep, barSixteenths, gateSeconds);
    if (polyLayerEditor != nullptr)
    {
        polyLayerEditor->setVoice(monoVoice, gateLength);
        polyLayerEditor->setParts(partCount, partStep, barSixteenths, gateSeconds);
    }
    if (generatorType != GeneratorTypeMapping::kCellular)
        cellularSeenGen = -1;
    if (generatorType != GeneratorTypeMapping::kMarkov)
        markovSeenSerial = 0;
    if (generatorType != GeneratorTypeMapping::kProbabilistic)
        probSeenPrimed = false;

    if (generatorType == GeneratorTypeMapping::kEuclidean)
    {
        auto& euclidean = audioProcessor.getEuclideanEngine();
        const int steps = juce::jlimit(1, 64, euclidean.getSteps());
        bool hits[64];
        for (int i = 0; i < steps; ++i)
            hits[i] = euclidean.getStep(i);

        patternDisplay.setMode(PatternVisualizer::Mode::Steps);
        patternDisplay.setSteps(hits, steps, audioProcessor.getCurrentStep() % steps);
        patternDisplay.setCaption("Euclidean  " + juce::String(euclidean.getPulses())
                                   + " / " + juce::String(steps));
    }
    else if (generatorType == GeneratorTypeMapping::kMarkov)
    {
        auto& algo = audioProcessor.getAlgorithmicEngine();
        patternDisplay.setMode(PatternVisualizer::Mode::Stems);
        const uint32_t serial = algo.getHistorySerial();
        if (serial != 0 && serial != markovSeenSerial)
        {
            uint32_t pending = serial - markovSeenSerial;
            if (markovSeenSerial == 0)
                pending = 1;
            const int count = algo.getHistoryCount();
            const int pushN = juce::jmin(count, static_cast<int>(juce::jmin<uint32_t>(pending, 16u)));
            for (int age = pushN - 1; age >= 0; --age)
            {
                const int note = algo.getHistoryNoteFromNewest(age);
                patternDisplay.pushSample(0.0f, normPitch(static_cast<float>(note)));
                patternDisplay.noteMelody(audioProcessor.getScaleQuantizer().quantize(note));
            }
            markovSeenSerial = serial;
        }
        patternDisplay.setCaption("Markov  order " + juce::String(static_cast<int>(markovOrderSlider.getValue()))
                                   + "  step " + juce::String(static_cast<int>(markovStepSlider.getValue())));
    }
    else if (generatorType == GeneratorTypeMapping::kLSystem)
    {
        const int grammar = static_cast<int>(lsystemGrammarSlider.getValue());
        const int generation = static_cast<int>(lsystemGenerationSlider.getValue());
        if (lSystemVizGrammar != grammar || lSystemVizGeneration != generation || lSystemRows.isEmpty())
        {
            lSystemRows.clear();
            char symbols[LSystemCatalog::kMaxSymbols];
            for (int row = 0; row <= generation; ++row)
            {
                int length = 0;
                LSystemCatalog::expand(grammar, row, symbols, LSystemCatalog::kMaxSymbols, length);
                lSystemRows.add(juce::String(symbols));
            }
            lSystemVizGrammar = grammar;
            lSystemVizGeneration = generation;
            lSystemFeedNote = 0;
        }
        const int n = juce::jmax(1, lSystemRows[lSystemRows.size() - 1].length());
        patternDisplay.setMode(PatternVisualizer::Mode::Symbols);
        patternDisplay.setGrowth(lSystemRows, audioProcessor.getCurrentStep() % n);
        patternDisplay.setCaption("L-System   " + juce::String(LSystemCatalog::name(grammar))
                                   + "  gen " + juce::String(generation));
    }
    else if (generatorType == GeneratorTypeMapping::kCellular)
    {
        auto& cells = audioProcessor.getAlgorithmicEngine().getCellularAutomaton();
        bool row[64];
        int count = 0;
        const int generation = cells.copyCells(row, 64, count);
        patternDisplay.setMode(PatternVisualizer::Mode::Cells);
        if (generation != cellularSeenGen && count > 0)
        {
            const int listen = juce::jlimit(0, count - 1, static_cast<int>(cellularListenSlider.getValue()));
            patternDisplay.pushCells(row, count, listen);
            if (row[static_cast<size_t>(listen)])
            {
                int weighted = 0;
                for (int i = 0; i < count; ++i)
                    if (row[static_cast<size_t>((listen + i) % count)])
                        weighted += i + 1;
                const int maxWeight = count * (count + 1) / 2;
                const int span = juce::jmax(0, static_cast<int>(pitchMax - pitchMin));
                const int raw = static_cast<int>(pitchMin) + (weighted * span) / juce::jmax(1, maxWeight);
                patternDisplay.noteMelody(audioProcessor.getScaleQuantizer().quantize(raw));
            }
            cellularSeenGen = generation;
        }
        patternDisplay.setCaption("Cellular   rule " + juce::String(static_cast<int>(cellularRuleSlider.getValue()))
                                   + "  listen " + juce::String(static_cast<int>(cellularListenSlider.getValue())));
    }
    else if (generatorType == GeneratorTypeMapping::kProbabilistic)
    {
        auto& algo = audioProcessor.getAlgorithmicEngine();
        patternDisplay.setMode(PatternVisualizer::Mode::Stems);
        patternDisplay.setShowDensity(true);
        patternDisplay.setDensity(static_cast<float>(densitySlider.getValue()));
        const uint32_t notes = audioProcessor.getNoteActivityCount();
        if (!probSeenPrimed)
        {
            probSeenNotes = notes;
            probSeenPrimed = true;
            patternDisplay.pushSample(0.0f, normPitch(static_cast<float>(algo.getLastProbNote())));
            patternDisplay.noteMelody(audioProcessor.getScaleQuantizer().quantize(algo.getLastProbNote()));
        }
        else if (notes != probSeenNotes)
        {
            patternDisplay.pushSample(0.0f, normPitch(static_cast<float>(algo.getLastProbNote())));
            patternDisplay.noteMelody(audioProcessor.getScaleQuantizer().quantize(algo.getLastProbNote()));
            probSeenNotes = notes;
        }
        patternDisplay.setCaption("Probabilistic");
    }
    else if (generatorType == GeneratorTypeMapping::kBrownian
             || generatorType == GeneratorTypeMapping::kPerlin
             || generatorType == GeneratorTypeMapping::kDrunkWalk)
    {
        auto& stochastic = audioProcessor.getStochasticEngine();
        const auto mode = generatorType == GeneratorTypeMapping::kDrunkWalk
                              ? PatternVisualizer::Mode::Stairs
                              : PatternVisualizer::Mode::Curve;
        patternDisplay.setMode(mode);
        patternDisplay.pushSample(0.0f, stochastic.getCurrentValue());
        patternDisplay.setCaption(generatorType == GeneratorTypeMapping::kBrownian ? "Brownian"
                                 : generatorType == GeneratorTypeMapping::kPerlin ? "Perlin"
                                 : "Drunk Walk");
    }
    else if (generatorType == GeneratorTypeMapping::kLorenz)
    {
        auto& stochastic = audioProcessor.getStochasticEngine();
        patternDisplay.setMode(PatternVisualizer::Mode::Orbit);
        patternDisplay.pushSample(stochastic.getCurrentValue(), stochastic.getTertiaryValue());
        patternDisplay.setCaption("Lorenz");
    }

    feedSoundingNotes(generatorType, partStep, static_cast<int>(pitchMin), static_cast<int>(pitchMax));

    updateStatusChip();

    // Drain RT-safe MIDI activity FIFO into the UI pane (message thread)
    MidiActivityEvent drained[MidiActivityLog::kMaxEvents];
    const int n = audioProcessor.getMidiActivityLog().pop(drained, MidiActivityLog::kMaxEvents);
    if (n > 0)
        midiActivityPane.ingest(drained, n);
}

void GenerativeMIDIEditor::currentPresetChanged(const juce::String& presetName)
{
    syncPresetLabel(presetName);
    refreshPresetCombo();
}

void GenerativeMIDIEditor::refreshPresetCombo()
{
    const auto names = audioProcessor.getPresetManager().getPresetNames();
    const auto current = audioProcessor.getPresetManager().getCurrentPresetName();

    juce::ScopedValueSetter<bool> guard(updatingPresetCombo, true);
    presetCombo.clear(juce::dontSendNotification);
    presetCombo.addItemList(names, 1);
    const int index = names.indexOf(current);
    if (index >= 0)
        presetCombo.setSelectedItemIndex(index, juce::dontSendNotification);
    else
        presetCombo.setTextWhenNothingSelected(current.isNotEmpty() ? current : "No Preset");
}

void GenerativeMIDIEditor::syncPresetLabel(const juce::String& presetName)
{
    loadedPresetName = presetName;
    loadedGeneratorId = presetName.isNotEmpty() ? generatorTypeCombo.getSelectedId() : -1;
    currentPresetLabel.setColour(juce::Label::textColourId,
                                 juce::Colour(CustomLookAndFeel::COPPER_STEAM));
    if (presetName.isNotEmpty())
        currentPresetLabel.setText(presetName, juce::dontSendNotification);
    else
        currentPresetLabel.setText("No Preset", juce::dontSendNotification);
    resized();
}

void GenerativeMIDIEditor::notePresetDivergence()
{
    if (loadedPresetName.isEmpty())
        return;

    if (generatorTypeCombo.getSelectedId() == loadedGeneratorId)
    {
        currentPresetLabel.setColour(juce::Label::textColourId,
                                     juce::Colour(CustomLookAndFeel::COPPER_STEAM));
        currentPresetLabel.setText(loadedPresetName, juce::dontSendNotification);
        resized();
        return;
    }

    currentPresetLabel.setColour(juce::Label::textColourId,
                                 juce::Colour(CustomLookAndFeel::AETHER_CYAN));
    currentPresetLabel.setText("Edited", juce::dontSendNotification);
    resized();
}

void GenerativeMIDIEditor::updateStatusChip()
{
    const bool standalone = (audioProcessor.wrapperType
                             == juce::AudioProcessor::wrapperType_Standalone);
    const bool advancing = audioProcessor.isClockAdvancing();
    const juce::String context = standalone ? "Standalone" : "Host";
    juce::String state;
    if (!advancing)
        state = "stopped";
    else if (activityPulse > 0.35f)
        state = "note";
    else if (standalone)
        state = "free-run";
    else
        state = "generating";

    const juce::String chip = context + juce::String::fromUTF8(" · ") + state;
    if (statusChipLabel.getText() != chip)
        statusChipLabel.setText(chip, juce::dontSendNotification);
}

void GenerativeMIDIEditor::updateControlsForGeneratorType(int generatorType)
{
    const bool isEuclideanGen = (generatorType == GeneratorTypeMapping::kEuclidean);
    const bool isPolyrhythmGen = GeneratorTypeMapping::isPolyrhythm(generatorType);
    const bool isStochasticGen = GeneratorTypeMapping::isStochastic(generatorType);

    stepsSlider.setEnabled(isEuclideanGen);
    pulsesSlider.setEnabled(isEuclideanGen);
    rotationSlider.setEnabled(isEuclideanGen);
    stepsLabel.setEnabled(isEuclideanGen);
    pulsesLabel.setEnabled(isEuclideanGen);
    rotationLabel.setEnabled(isEuclideanGen);

    float euclideanAlpha = isEuclideanGen ? 1.0f : 0.3f;
    stepsSlider.setAlpha(euclideanAlpha);
    pulsesSlider.setAlpha(euclideanAlpha);
    rotationSlider.setAlpha(euclideanAlpha);
    stepsLabel.setAlpha(euclideanAlpha);
    pulsesLabel.setAlpha(euclideanAlpha);
    rotationLabel.setAlpha(euclideanAlpha);

    stepSizeSlider.setEnabled(isStochasticGen);
    momentumSlider.setEnabled(isStochasticGen);
    timeScaleSlider.setEnabled(isStochasticGen);
    stepSizeLabel.setEnabled(isStochasticGen);
    momentumLabel.setEnabled(isStochasticGen);
    timeScaleLabel.setEnabled(isStochasticGen);

    float stochasticAlpha = isStochasticGen ? 1.0f : 0.3f;
    stepSizeSlider.setAlpha(stochasticAlpha);
    momentumSlider.setAlpha(stochasticAlpha);
    timeScaleSlider.setAlpha(stochasticAlpha);
    stepSizeLabel.setAlpha(stochasticAlpha);
    momentumLabel.setAlpha(stochasticAlpha);
    timeScaleLabel.setAlpha(stochasticAlpha);

    densityLabel.setEnabled(true);
    densitySlider.setEnabled(true);
    densitySlider.setAlpha(1.0f);
    densityLabel.setAlpha(1.0f);

    if (isEuclideanGen)
        densityLabel.setText("Probability", juce::dontSendNotification);
    else
        densityLabel.setText("Density", juce::dontSendNotification);

    const auto family = CustomLookAndFeel::familyAccent(generatorType);
    generatorLabel.setColour(juce::Label::textColourId, family);
    generatorTypeCombo.setColour(juce::ComboBox::textColourId, family);

    patternDisplay.setVisible(!isPolyrhythmGen);
    if (polyLayerEditor)
    {
        polyLayerEditor->setVisible(isPolyrhythmGen);
        if (isPolyrhythmGen)
            polyLayerEditor->rebuildLayers();
    }

    resized();
    repaint();
}

//==============================================================================
// Override createEditor in processor to use custom editor
