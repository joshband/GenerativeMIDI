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

    // Window size (increased to accommodate all controls)
    setSize(1400, 600);
    setResizable(true, true);
    setResizeLimits(1200, 500, 2000, 1000);

    // Title - SYNAPTIK gilded brass logo
    addAndMakeVisible(titleLabel);
    titleLabel.setText("S Y N A P T I K", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(40.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::GOLD_TEMPLE));
    titleLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    // Pattern visualizer
    addAndMakeVisible(patternDisplay);

    // Generator type selector
    addAndMakeVisible(generatorLabel);
    generatorLabel.setText("Generator", juce::dontSendNotification);
    generatorLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(generatorTypeCombo);
    generatorTypeCombo.addItemList(juce::StringArray{"Euclidean", "Polyrhythm", "Markov", "L-System", "Cellular", "Probabilistic",
                                                      "Brownian", "Perlin Noise", "Drunk Walk", "Lorenz"}, 1);
    generatorAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
        audioProcessor.getValueTreeState(), "generatorType", generatorTypeCombo));

    // Listen for generator type changes to update UI
    generatorTypeCombo.onChange = [this]() {
        updateControlsForGeneratorType(generatorTypeCombo.getSelectedId() - 1);
    };

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

    // Probability control (applies to all generators)
    addAndMakeVisible(densitySlider);
    densitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    densitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    densityAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "noteDensity", densitySlider));
    addAndMakeVisible(densityLabel);
    densityLabel.setText("Probability", juce::dontSendNotification);
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

    // Scale controls
    addAndMakeVisible(scaleLabel);
    scaleLabel.setText("Scale", juce::dontSendNotification);
    scaleLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(scaleRootCombo);
    scaleRootCombo.addItemList(juce::StringArray{"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"}, 1);
    scaleRootAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
        audioProcessor.getValueTreeState(), "scaleRoot", scaleRootCombo));

    addAndMakeVisible(scaleTypeCombo);
    scaleTypeCombo.addItemList(juce::StringArray{"Chromatic", "Major", "Minor", "Harmonic Minor", "Melodic Minor",
                                                   "Dorian", "Phrygian", "Lydian", "Mixolydian", "Locrian",
                                                   "Major Pentatonic", "Minor Pentatonic", "Blues", "Whole Tone",
                                                   "Diminished", "Harmonic Major"}, 1);
    scaleTypeAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
        audioProcessor.getValueTreeState(), "scaleType", scaleTypeCombo));

    // Swing and humanization controls
    addAndMakeVisible(swingSlider);
    swingSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    swingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    swingAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "swingAmount", swingSlider));
    addAndMakeVisible(swingLabel);
    swingLabel.setText("Swing", juce::dontSendNotification);
    swingLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(timingHumanizeSlider);
    timingHumanizeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    timingHumanizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    timingHumanizeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "timingHumanize", timingHumanizeSlider));
    addAndMakeVisible(timingHumanizeLabel);
    timingHumanizeLabel.setText("Timing", juce::dontSendNotification);
    timingHumanizeLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(velocityHumanizeSlider);
    velocityHumanizeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    velocityHumanizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    velocityHumanizeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "velocityHumanize", velocityHumanizeSlider));
    addAndMakeVisible(velocityHumanizeLabel);
    velocityHumanizeLabel.setText("Vel Var", juce::dontSendNotification);
    velocityHumanizeLabel.setJustificationType(juce::Justification::centred);

    // Gate length controls
    addAndMakeVisible(gateLengthSlider);
    gateLengthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gateLengthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    gateLengthAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "gateLength", gateLengthSlider));
    addAndMakeVisible(gateLengthLabel);
    gateLengthLabel.setText("Gate", juce::dontSendNotification);
    gateLengthLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(legatoButton);
    legatoButton.setButtonText("Legato");
    legatoButton.setClickingTogglesState(true);
    legatoAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(
        audioProcessor.getValueTreeState(), "legatoMode", legatoButton));

    // Ratchet controls
    addAndMakeVisible(ratchetCountSlider);
    ratchetCountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ratchetCountSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    ratchetCountAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "ratchetCount", ratchetCountSlider));
    addAndMakeVisible(ratchetCountLabel);
    ratchetCountLabel.setText("Ratchet", juce::dontSendNotification);
    ratchetCountLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(ratchetProbabilitySlider);
    ratchetProbabilitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ratchetProbabilitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    ratchetProbabilityAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "ratchetProbability", ratchetProbabilitySlider));
    addAndMakeVisible(ratchetProbabilityLabel);
    ratchetProbabilityLabel.setText("R Prob", juce::dontSendNotification);
    ratchetProbabilityLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(ratchetDecaySlider);
    ratchetDecaySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ratchetDecaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    ratchetDecayAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTreeState(), "ratchetDecay", ratchetDecaySlider));
    addAndMakeVisible(ratchetDecayLabel);
    ratchetDecayLabel.setText("R Decay", juce::dontSendNotification);
    ratchetDecayLabel.setJustificationType(juce::Justification::centred);

    // Preset browser button
    addAndMakeVisible(presetBrowserButton);
    presetBrowserButton.setButtonText("Presets");
    presetBrowserButton.onClick = [this]()
    {
        if (!presetBrowser)
        {
            presetBrowser = std::make_unique<PresetBrowser>(audioProcessor.getPresetManager());
            presetBrowser->setSize(500, 600);
        }

        juce::DialogWindow::LaunchOptions options;
        options.content.setNonOwned(presetBrowser.get());
        options.dialogTitle = "Preset Manager";
        options.componentToCentreAround = this;
        options.dialogBackgroundColour = juce::Colour(0xff0f0f0f);
        options.escapeKeyTriggersCloseButton = true;
        options.useNativeTitleBar = true;
        options.resizable = true;

        options.launchAsync();
    };

    // Current preset label
    addAndMakeVisible(currentPresetLabel);
    currentPresetLabel.setText("No Preset", juce::dontSendNotification);
    currentPresetLabel.setFont(juce::Font(12.0f));
    currentPresetLabel.setJustificationType(juce::Justification::centred);
    currentPresetLabel.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::GREEN_VERDIGRIS));

    // Initialize UI for current generator type
    updateControlsForGeneratorType(generatorTypeCombo.getSelectedId() - 1);

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
    auto bounds = getLocalBounds().toFloat();

    // SYNAPTIK: Abyss Navy background - Victorian deep void
    g.fillAll(juce::Colour(CustomLookAndFeel::ABYSS_NAVY));

    // Brass filigree texture (ornate background pattern)
    juce::Random r(42); // Consistent seed for repeatable pattern

    // Brass rivets and ornamental dots
    g.setColour(juce::Colour(CustomLookAndFeel::BRONZE_GOTHIC).withAlpha(0.2f));
    for (int i = 0; i < 80; ++i)
    {
        float x = r.nextFloat() * bounds.getWidth();
        float y = r.nextFloat() * bounds.getHeight();
        float size = r.nextFloat() * 1.5f + 0.8f;
        // Brass rivet with highlight
        g.fillEllipse(x, y, size, size);
        g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.1f));
        g.fillEllipse(x + size * 0.2f, y + size * 0.2f, size * 0.5f, size * 0.5f);
        g.setColour(juce::Colour(CustomLookAndFeel::BRONZE_GOTHIC).withAlpha(0.2f));
    }

    // Art Deco geometric lines (brass inlay)
    g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.12f));
    for (int i = 0; i < 30; ++i)
    {
        float x1 = r.nextFloat() * bounds.getWidth();
        float y1 = r.nextFloat() * bounds.getHeight();
        float x2 = x1 + r.nextFloat() * 60.0f - 30.0f;
        float y2 = y1 + r.nextFloat() * 60.0f - 30.0f;
        g.drawLine(x1, y1, x2, y2, 0.8f);
    }

    // Subtle aether glow particles (floating energy)
    g.setColour(juce::Colour(CustomLookAndFeel::AETHER_CYAN).withAlpha(0.08f));
    for (int i = 0; i < 25; ++i)
    {
        float x = r.nextFloat() * bounds.getWidth();
        float y = r.nextFloat() * bounds.getHeight();
        float glowSize = r.nextFloat() * 8.0f + 4.0f;
        g.fillEllipse(x - glowSize * 0.5f, y - glowSize * 0.5f, glowSize, glowSize);
    }

    // Corner brass ornaments (Victorian corner brackets)
    auto drawBrassOrnament = [&g](float x, float y, bool flipX, bool flipY)
    {
        float xDir = flipX ? -1.0f : 1.0f;
        float yDir = flipY ? -1.0f : 1.0f;

        // Aether glow
        juce::ColourGradient aetherGlow(
            juce::Colour(CustomLookAndFeel::AETHER_CYAN).withAlpha(0.25f),
            x, y,
            juce::Colour(CustomLookAndFeel::AMBER_TESLA).withAlpha(0.0f),
            x + xDir * 25, y + yDir * 25,
            false);
        g.setGradientFill(aetherGlow);
        g.fillEllipse(x - 12, y - 12, 24, 24);

        // Brass outer ring
        g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.8f));
        g.drawEllipse(x - 6, y - 6, 12, 12, 2.0f);

        // Golden highlight
        g.setColour(juce::Colour(CustomLookAndFeel::GOLD_TEMPLE).withAlpha(0.6f));
        g.drawEllipse(x - 5, y - 5, 10, 10, 1.0f);

        // Bronze core
        g.setColour(juce::Colour(CustomLookAndFeel::BRONZE_GOTHIC));
        g.fillEllipse(x - 3, y - 3, 6, 6);

        // Aether crystal center
        g.setColour(juce::Colour(CustomLookAndFeel::AETHER_CYAN).withAlpha(0.7f));
        g.fillEllipse(x - 1.5f, y - 1.5f, 3, 3);
    };

    drawBrassOrnament(20, 20, false, false);
    drawBrassOrnament(getWidth() - 20, 20, true, false);
    drawBrassOrnament(20, getHeight() - 20, false, true);
    drawBrassOrnament(getWidth() - 20, getHeight() - 20, true, true);

    // Gothic brass panels (Art Deco section frames)
    auto drawBrassPanel = [&g](juce::Rectangle<float> area, juce::String label)
    {
        // Aether energy field glow
        juce::ColourGradient energyGlow(
            juce::Colour(CustomLookAndFeel::AETHER_CYAN).withAlpha(0.08f),
            area.getCentreX(), area.getY(),
            juce::Colour(CustomLookAndFeel::AMBER_TESLA).withAlpha(0.0f),
            area.getCentreX(), area.getBottom(),
            false);
        g.setGradientFill(energyGlow);
        g.fillRoundedRectangle(area.expanded(3), 8.0f);

        // Obsidian steel panel background
        juce::ColourGradient panelGrad(
            juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN).brighter(0.08f),
            area.getX(), area.getCentreY(),
            juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN).darker(0.15f),
            area.getRight(), area.getCentreY(),
            false);
        g.setGradientFill(panelGrad);
        g.fillRoundedRectangle(area, 6.0f);

        // Polished brass bezel (outer border)
        g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.7f));
        g.drawRoundedRectangle(area, 6.0f, 2.5f);

        // Golden accent line
        g.setColour(juce::Colour(CustomLookAndFeel::GOLD_TEMPLE).withAlpha(0.4f));
        g.drawRoundedRectangle(area.reduced(2.5f), 5.0f, 1.0f);

        // Bronze inner shadow (depth)
        g.setColour(juce::Colour(CustomLookAndFeel::BRONZE_GOTHIC).withAlpha(0.3f));
        g.drawRoundedRectangle(area.reduced(5.0f), 4.0f, 0.8f);

        // Ornate brass label plate
        if (label.isNotEmpty())
        {
            auto labelArea = juce::Rectangle<float>(area.getX() + 12, area.getY() + 5, area.getWidth() - 24, 24);

            // Label plate background (raised brass)
            juce::ColourGradient labelGrad(
                juce::Colour(CustomLookAndFeel::BRASS_AGED).brighter(0.15f),
                labelArea.getCentreX(), labelArea.getY(),
                juce::Colour(CustomLookAndFeel::BRASS_AGED).darker(0.1f),
                labelArea.getCentreX(), labelArea.getBottom(),
                false);
            g.setGradientFill(labelGrad);
            g.fillRoundedRectangle(labelArea, 3.0f);

            // Label engraved text
            g.setColour(juce::Colour(CustomLookAndFeel::GOLD_TEMPLE));
            g.setFont(juce::Font(14.0f, juce::Font::bold));
            g.drawText(label, labelArea, juce::Justification::centred);
        }
    };

    drawBrassPanel(juce::Rectangle<float>(25, 70, getWidth() - 50, 130), "PATTERN DISPLAY");
    drawBrassPanel(juce::Rectangle<float>(25, 220, getWidth() - 50, 190), "GENERATOR");
    drawBrassPanel(juce::Rectangle<float>(25, 430, getWidth() - 50, 150), "EXPRESSION");
}

void GenerativeMIDIEditor::resized()
{
    auto area = getLocalBounds();

    // Title and Preset controls
    auto titleArea = area.removeFromTop(60).reduced(20, 10);
    auto presetArea = titleArea.removeFromRight(180);
    presetBrowserButton.setBounds(presetArea.removeFromTop(30).reduced(5));
    currentPresetLabel.setBounds(presetArea.reduced(5, 0));
    titleLabel.setBounds(titleArea);

    // Pattern display section
    auto patternSection = area.removeFromTop(140).reduced(40, 20);
    patternSection.removeFromTop(20); // Section label
    patternDisplay.setBounds(patternSection);

    // Generator controls section
    auto controlsSection = area.removeFromTop(200).reduced(40, 20);
    controlsSection.removeFromTop(20); // Section label

    int knobSize = 85;  // Reduced from 100 to fit more controls
    int spacing = 15;   // Reduced from 20 to fit more controls

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

    // Expression & range section (increased height for more controls)
    auto rangeSection = area.removeFromTop(160).reduced(40, 20);
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

    rangeSection.removeFromLeft(40);

    // Scale controls
    auto scaleArea = rangeSection.removeFromLeft(150);
    scaleLabel.setBounds(scaleArea.removeFromTop(20));
    scaleRootCombo.setBounds(scaleArea.removeFromTop(25).reduced(5, 0));
    scaleArea.removeFromTop(5);
    scaleTypeCombo.setBounds(scaleArea.removeFromTop(25).reduced(5, 0));

    rangeSection.removeFromLeft(spacing);

    // Humanization knobs
    auto swingArea = rangeSection.removeFromLeft(knobSize);
    swingLabel.setBounds(swingArea.removeFromBottom(20));
    swingSlider.setBounds(swingArea);

    rangeSection.removeFromLeft(spacing);

    auto timingArea = rangeSection.removeFromLeft(knobSize);
    timingHumanizeLabel.setBounds(timingArea.removeFromBottom(20));
    timingHumanizeSlider.setBounds(timingArea);

    rangeSection.removeFromLeft(spacing);

    auto velVarArea = rangeSection.removeFromLeft(knobSize);
    velocityHumanizeLabel.setBounds(velVarArea.removeFromBottom(20));
    velocityHumanizeSlider.setBounds(velVarArea);

    rangeSection.removeFromLeft(spacing);

    auto gateArea = rangeSection.removeFromLeft(knobSize);
    gateLengthLabel.setBounds(gateArea.removeFromBottom(20));
    gateLengthSlider.setBounds(gateArea);

    rangeSection.removeFromLeft(spacing);

    auto legatoArea = rangeSection.removeFromLeft(knobSize);
    legatoButton.setBounds(legatoArea.removeFromTop(50).reduced(5));

    rangeSection.removeFromLeft(spacing);

    auto ratchetCountArea = rangeSection.removeFromLeft(knobSize);
    ratchetCountLabel.setBounds(ratchetCountArea.removeFromBottom(20));
    ratchetCountSlider.setBounds(ratchetCountArea);

    rangeSection.removeFromLeft(spacing);

    auto ratchetProbArea = rangeSection.removeFromLeft(knobSize);
    ratchetProbabilityLabel.setBounds(ratchetProbArea.removeFromBottom(20));
    ratchetProbabilitySlider.setBounds(ratchetProbArea);

    rangeSection.removeFromLeft(spacing);

    auto ratchetDecayArea = rangeSection.removeFromLeft(knobSize);
    ratchetDecayLabel.setBounds(ratchetDecayArea.removeFromBottom(20));
    ratchetDecaySlider.setBounds(ratchetDecayArea);
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

    // Update current preset label
    const juce::String& presetName = audioProcessor.getPresetManager().getCurrentPresetName();
    if (presetName.isNotEmpty())
        currentPresetLabel.setText(presetName, juce::dontSendNotification);
    else
        currentPresetLabel.setText("No Preset", juce::dontSendNotification);
}

void GenerativeMIDIEditor::updateControlsForGeneratorType(int generatorType)
{
    // Generator types:
    // 0 = Euclidean
    // 1 = Polyrhythm
    // 2-5 = Algorithmic (Markov, L-System, Cellular, Probabilistic)
    // 6-9 = Stochastic (Brownian, Perlin, Drunk Walk, Lorenz)

    bool isEuclidean = (generatorType == 0);
    bool isPolyrhythm = (generatorType == 1);
    bool isAlgorithmic = (generatorType >= 2 && generatorType <= 5);
    bool isStochastic = (generatorType >= 6 && generatorType <= 9);

    // Euclidean-specific controls (steps, pulses, rotation)
    stepsSlider.setEnabled(isEuclidean);
    pulsesSlider.setEnabled(isEuclidean);
    rotationSlider.setEnabled(isEuclidean);
    stepsLabel.setEnabled(isEuclidean);
    pulsesLabel.setEnabled(isEuclidean);
    rotationLabel.setEnabled(isEuclidean);

    // Set alpha for visual feedback
    float euclideanAlpha = isEuclidean ? 1.0f : 0.4f;
    stepsSlider.setAlpha(euclideanAlpha);
    pulsesSlider.setAlpha(euclideanAlpha);
    rotationSlider.setAlpha(euclideanAlpha);
    stepsLabel.setAlpha(euclideanAlpha);
    pulsesLabel.setAlpha(euclideanAlpha);
    rotationLabel.setAlpha(euclideanAlpha);

    // Density control applies to all generators but with different meanings
    densityLabel.setEnabled(true);
    densitySlider.setEnabled(true);
    densitySlider.setAlpha(1.0f);
    densityLabel.setAlpha(1.0f);

    // Update label based on generator type
    if (isEuclidean)
        densityLabel.setText("Probability", juce::dontSendNotification);
    else if (isStochastic)
        densityLabel.setText("Density", juce::dontSendNotification);
    else
        densityLabel.setText("Density", juce::dontSendNotification);

    // All controls that work across all generator types remain fully enabled
    // (tempo, velocity range, pitch range, scale, swing, humanization, gate length, ratcheting)

    // Force repaint to show changes
    repaint();
}

//==============================================================================
// Override createEditor in processor to use custom editor
