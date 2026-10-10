/*
  ==============================================================================
    PolyrhythmLayerEditor.h

    One step row per polyrhythm layer. Row length is that layer's step count
    so unequal lengths read as polymeter. Clicks toggle hits in the engine.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "CustomLookAndFeel.h"
#include "PatternVisualizer.h"
#include "../Core/PolyrhythmEngine.h"
#include <cmath>
#include <functional>

class PolyrhythmLayerRow : public juce::Component
{
public:
    PolyrhythmLayerRow(int layerIdx, PolyrhythmEngine& engine)
        : layerIndex(layerIdx), polyEngine(engine)
    {
        addAndMakeVisible(enableButton);
        enableButton.setButtonText("");
        enableButton.setClickingTogglesState(true);
        enableButton.setToggleState(true, juce::dontSendNotification);
        enableButton.onClick = [this]() { onEnableChanged(); };

        addAndMakeVisible(layerLabel);
        layerLabel.setText(juce::String(layerIndex + 1), juce::dontSendNotification);
        layerLabel.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
        layerLabel.setJustificationType(juce::Justification::centred);
        layerLabel.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::GOLD_TEMPLE));

        addAndMakeVisible(divisionSlider);
        divisionSlider.setRange(1, 32, 1);
        divisionSlider.setValue(4, juce::dontSendNotification);
        divisionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        divisionSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        divisionSlider.onValueChange = [this]() { onDivisionChanged(); };

        addAndMakeVisible(lengthSlider);
        lengthSlider.setRange(1, 32, 1);
        lengthSlider.setValue(16, juce::dontSendNotification);
        lengthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        lengthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        lengthSlider.onValueChange = [this]() { onLengthChanged(); };

        addAndMakeVisible(pitchSlider);
        pitchSlider.setRange(-24, 24, 1);
        pitchSlider.setValue(0, juce::dontSendNotification);
        pitchSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        pitchSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        pitchSlider.onValueChange = [this]() { onPitchChanged(); };

        addAndMakeVisible(velocitySlider);
        velocitySlider.setRange(0.0, 2.0, 0.01);
        velocitySlider.setValue(1.0, juce::dontSendNotification);
        velocitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
        velocitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        velocitySlider.onValueChange = [this]() { onVelocityChanged(); };

        auto styleName = [](juce::Label& label, const juce::String& text)
        {
            label.setText(text, juce::dontSendNotification);
            label.setFont(juce::FontOptions(10.0f));
            label.setJustificationType(juce::Justification::centredRight);
            label.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::COPPER_STEAM));
            label.setInterceptsMouseClicks(false, false);
        };
        addAndMakeVisible(divisionName);
        addAndMakeVisible(lengthName);
        addAndMakeVisible(pitchName);
        addAndMakeVisible(velocityName);
        styleName(divisionName, "Div");
        styleName(lengthName, "Len");
        styleName(pitchName, "Pitch");
        styleName(velocityName, "Vel");

        addAndMakeVisible(patternDisplay);
        patternDisplay.onSetStep = [this](int step, bool active) { setStepActive(step, active); };

        const auto layerPrefix = "Layer " + juce::String(layerIndex + 1) + " ";
        setTitle(layerPrefix.trimEnd());
        enableButton.setTitle(layerPrefix + "Enable");
        divisionSlider.setTitle(layerPrefix + "Division");
        lengthSlider.setTitle(layerPrefix + "Length");
        pitchSlider.setTitle(layerPrefix + "Pitch");
        velocitySlider.setTitle(layerPrefix + "Velocity");
        patternDisplay.setTitle(layerPrefix + "Pattern");
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.22f));
        g.drawHorizontalLine(getHeight() - 1, 4.0f, static_cast<float>(getWidth()) - 4.0f);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(4, 3);
        auto badge = area.removeFromLeft(52);
        auto enableSlot = badge.removeFromLeft(26);
        enableButton.setBounds(enableSlot.withSizeKeepingCentre(18, 18));
        layerLabel.setBounds(badge.withSizeKeepingCentre(juce::jmax(16, badge.getWidth()), 18));

        area.removeFromLeft(8);
        auto controls = area.removeFromBottom(juce::jmin(24, juce::jmax(18, area.getHeight() / 3)));
        area.removeFromBottom(3);
        patternDisplay.setBounds(area);

        const int gap = 8;
        const int cols = 4;
        const int each = juce::jmax(88, (controls.getWidth() - gap * (cols - 1)) / cols);
        auto place = [&](juce::Label& name, juce::Slider& slider)
        {
            auto col = controls.removeFromLeft(juce::jmin(each, controls.getWidth()));
            controls.removeFromLeft(gap);
            name.setBounds(col.removeFromLeft(36));
            slider.setBounds(col.reduced(2, 1));
        };
        place(divisionName, divisionSlider);
        place(lengthName, lengthSlider);
        place(pitchName, pitchSlider);
        place(velocityName, velocitySlider);
    }

    void setStepPitch(float pitch)
    {
        patternDisplay.setStepPitch(pitch);
    }

    void setVoice(bool mono, int sounding)
    {
        patternDisplay.setVoice(mono, sounding);
    }

    void updateFromEngine()
    {
        auto* layer = polyEngine.getLayer(layerIndex);
        if (layer == nullptr)
            return;

        enableButton.setToggleState(layer->enabled, juce::dontSendNotification);
        divisionSlider.setValue(layer->division, juce::dontSendNotification);
        lengthSlider.setValue(layer->length, juce::dontSendNotification);
        pitchSlider.setValue(layer->pitchOffset, juce::dontSendNotification);
        velocitySlider.setValue(layer->velocityMultiplier, juce::dontSendNotification);
        patternDisplay.setPattern(layer->pattern);
        patternDisplay.setCurrentStep(polyEngine.getCurrentStep(layerIndex));
    }

    std::function<void()> onGeometryChanged;

private:
    void onEnableChanged()
    {
        polyEngine.setLayerEnabled(layerIndex, enableButton.getToggleState());
    }

    void onDivisionChanged()
    {
        polyEngine.setLayerDivision(layerIndex, static_cast<int>(divisionSlider.getValue()));
    }

    void onLengthChanged()
    {
        polyEngine.setLayerLength(layerIndex, static_cast<int>(lengthSlider.getValue()));
        if (onGeometryChanged)
            onGeometryChanged();
        updateFromEngine();
    }

    void onPitchChanged()
    {
        polyEngine.setLayerPitchOffset(layerIndex, static_cast<int>(pitchSlider.getValue()));
    }

    void onVelocityChanged()
    {
        polyEngine.setLayerVelocityMultiplier(layerIndex, static_cast<float>(velocitySlider.getValue()));
    }

    void setStepActive(int step, bool active)
    {
        auto* layer = polyEngine.getLayer(layerIndex);
        if (layer == nullptr || step < 0 || step >= layer->length)
            return;

        const auto index = static_cast<size_t>(step);
        const float velocity = index < layer->velocities.size() ? layer->velocities[index] : 0.8f;
        const int pitch = index < layer->pitches.size() ? layer->pitches[index] : 60;
        polyEngine.setStep(layerIndex, step, active, velocity, pitch);
        updateFromEngine();
    }

    int layerIndex;
    PolyrhythmEngine& polyEngine;

    juce::ToggleButton enableButton;
    juce::Label layerLabel;
    juce::Label divisionName;
    juce::Label lengthName;
    juce::Label pitchName;
    juce::Label velocityName;
    juce::Slider divisionSlider;
    juce::Slider lengthSlider;
    juce::Slider pitchSlider;
    juce::Slider velocitySlider;

    class StepRowDisplay : public juce::Component
    {
    public:
        StepRowDisplay()
        {
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
        }

        std::function<void(int step, bool active)> onSetStep;

        void setPattern(const std::vector<bool>& newPattern)
        {
            pattern = newPattern;
            repaint();
        }

        void setCurrentStep(int step)
        {
            if (currentStep == step)
                return;
            currentStep = step;
            repaint();
        }

        void setVoice(bool mono, int sounding)
        {
            sounding = juce::jlimit(1, 8, sounding);
            if (monoVoice == mono && soundingCount == sounding)
                return;
            monoVoice = mono;
            soundingCount = sounding;
            repaint();
        }

        void setStepPitch(float pitch)
        {
            stepPitch = juce::jmax(8.0f, pitch);
            repaint();
        }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            if (pattern.empty() || stepPitch <= 0.0f)
                return;

            const int n = static_cast<int>(pattern.size());
            const float gap = 2.0f;
            const float cellW = juce::jmax(4.0f, stepPitch - gap);
            const float cellH = bounds.getHeight();
            const auto playhead = CustomLookAndFeel::familyAccent(1);

            for (int i = 0; i < n; ++i)
            {
                auto cell = juce::Rectangle<float>(bounds.getX() + stepPitch * static_cast<float>(i),
                                                   bounds.getY(), cellW, cellH);
                const int behind = (currentStep - i + n) % n;
                const bool held = pattern[static_cast<size_t>(i)] && behind > 0 && behind < soundingCount;
                PatternVisualizer::paintOneWell(g, cell, pattern[static_cast<size_t>(i)],
                                               i == currentStep || held, 1.0f, playhead);
                if (i == hoverStep)
                {
                    g.setColour(juce::Colour(CustomLookAndFeel::AETHER_CYAN).withAlpha(0.9f));
                    g.drawRoundedRectangle(cell, 2.0f, 1.0f);
                }
            }
        }

        void mouseMove(const juce::MouseEvent& e) override
        {
            setHover(stepAt(e.position.x));
        }

        void mouseExit(const juce::MouseEvent&) override
        {
            painting = false;
            setHover(-1);
        }

        void mouseDown(const juce::MouseEvent& e) override
        {
            const int step = stepAt(e.position.x);
            if (step < 0 || onSetStep == nullptr)
                return;

            painting = true;
            paintValue = !pattern[static_cast<size_t>(step)];
            onSetStep(step, paintValue);
        }

        void mouseDrag(const juce::MouseEvent& e) override
        {
            if (!painting || onSetStep == nullptr)
                return;

            const int step = stepAt(e.position.x);
            if (step < 0 || pattern[static_cast<size_t>(step)] == paintValue)
                return;

            onSetStep(step, paintValue);
        }

        void mouseUp(const juce::MouseEvent&) override
        {
            painting = false;
        }

    private:
        int stepAt(float x) const
        {
            if (pattern.empty() || stepPitch <= 0.0f)
                return -1;

            const int index = static_cast<int>(std::floor(x / stepPitch));
            if (index < 0 || index >= static_cast<int>(pattern.size()))
                return -1;
            return index;
        }

        void setHover(int step)
        {
            if (hoverStep == step)
                return;
            hoverStep = step;
            repaint();
        }

        std::vector<bool> pattern;
        int currentStep = 0;
        int hoverStep = -1;
        bool monoVoice = false;
        int soundingCount = 1;
        float stepPitch = 16.0f;
        bool painting = false;
        bool paintValue = true;
    } patternDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyrhythmLayerRow)
};

class PolyrhythmLayerEditor : public juce::Component, private juce::Timer
{
public:
    PolyrhythmLayerEditor(PolyrhythmEngine& engine)
        : polyEngine(engine)
    {
        addAndMakeVisible(addLayerButton);
        addLayerButton.setButtonText("+ Add Layer");
        addLayerButton.onClick = [this]() { addLayer(); };

        addAndMakeVisible(removeLayerButton);
        removeLayerButton.setButtonText("- Remove Layer");
        removeLayerButton.onClick = [this]() { removeLayer(); };

        addAndMakeVisible(layerViewport);
        layerViewport.setViewedComponent(&layerCanvas, false);
        layerViewport.setScrollBarsShown(true, true);
        layerViewport.getVerticalScrollBar().setColour(
            juce::ScrollBar::thumbColourId, juce::Colour(CustomLookAndFeel::BRASS_AGED));
        layerViewport.getHorizontalScrollBar().setColour(
            juce::ScrollBar::thumbColourId, juce::Colour(CustomLookAndFeel::BRASS_AGED));

        setTitle("Polyrhythm Layers");
        addLayerButton.setTitle("Add Layer");
        removeLayerButton.setTitle("Remove Layer");

        addAndMakeVisible(voiceReadout);
        voiceReadout.setText("Poly", juce::dontSendNotification);
        voiceReadout.setJustificationType(juce::Justification::centredRight);
        voiceReadout.setFont(juce::Font(juce::FontOptions(11.0f)).withExtraKerningFactor(0.12f));
        voiceReadout.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::BRASS_AGED));

        rebuildLayers();
        startTimerHz(30);
    }

    void paint(juce::Graphics& g) override
    {
        if (partColumn.isEmpty())
            return;
        PatternVisualizer::paintPitchStack(
            g, partColumn.toFloat(), stackMarks, stackCount, stackLow, stackHigh, true);
    }

    void setStack(const PatternVisualizer::StackMark* marks, int count, int low, int high)
    {
        stackCount = juce::jlimit(0, 16, count);
        for (int i = 0; i < stackCount; ++i)
            stackMarks[static_cast<size_t>(i)] = marks[i];
        stackLow = low;
        stackHigh = juce::jmax(low + 1, high);
        repaint();
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(4, 2);

        auto buttonArea = area.removeFromTop(24);
        addLayerButton.setBounds(buttonArea.removeFromLeft(96).reduced(0, 1));
        buttonArea.removeFromLeft(6);
        removeLayerButton.setBounds(buttonArea.removeFromLeft(118).reduced(0, 1));
        voiceReadout.setBounds(buttonArea.removeFromRight(72));

        area.removeFromTop(4);
        partColumn = area.removeFromRight(46).reduced(0, 2);
        area.removeFromRight(6);
        layerViewport.setBounds(area);

        const int numRows = layerRows.size();
        const int longest = longestLength();
        const int viewW = juce::jmax(1, layerViewport.getMaximumVisibleWidth());
        const int viewH = juce::jmax(1, layerViewport.getMaximumVisibleHeight());
        constexpr int kBadgeW = 60;
        constexpr float kMinPitch = 12.0f;
        constexpr float kMaxPitch = 22.0f;
        const float avail = static_cast<float>(juce::jmax(kBadgeW, viewW - kBadgeW));
        const float pitch = juce::jlimit(kMinPitch, kMaxPitch, avail / static_cast<float>(longest));

        constexpr int kPreferredRow = 72;
        constexpr int kGap = 4;
        const int contentH = numRows > 0
                                 ? numRows * kPreferredRow + kGap * (numRows - 1)
                                 : viewH;
        const int stepsW = kBadgeW + juce::roundToInt(pitch * static_cast<float>(longest));
        const bool needsV = contentH > viewH;
        const int scrollW = needsV ? layerViewport.getScrollBarThickness() : 0;
        const int contentW = juce::jmax(viewW - scrollW, stepsW);

        layerCanvas.setSize(contentW, juce::jmax(contentH, viewH));

        int y = 0;
        for (int i = 0; i < numRows; ++i)
        {
            layerRows[i]->setVisible(true);
            layerRows[i]->setStepPitch(pitch);
            layerRows[i]->setBounds(0, y, contentW, kPreferredRow);
            y += kPreferredRow + kGap;
        }

        cachedLongest = longest;
    }

    int getLayerCount() const { return layerRows.size(); }
    std::function<void()> onLayersChanged;

    void setVoice(bool mono, float)
    {
        if (monoVoice == mono)
            return;
        monoVoice = mono;
        voiceReadout.setText(mono ? "Mono" : "Poly", juce::dontSendNotification);
        for (auto* row : layerRows)
            row->setVoice(mono, 1);
        repaint();
    }

    void setParts(int count, int step, int sixteenthsPerBar, float gateSecondsIn)
    {
        partCount = juce::jlimit(1, 4, count);
        barSixteenths = juce::jmax(1, sixteenthsPerBar);
        gateSeconds = juce::jlimit(0.02f, 4.0f, gateSecondsIn);
        partStep = step;
        clock.advance(partCount, step, barSixteenths);
        repaint();
    }

    void rebuildLayers()
    {
        layerRows.clear();

        const int numLayers = polyEngine.getNumLayers();
        for (int i = 0; i < numLayers; ++i)
        {
            auto* row = layerRows.add(new PolyrhythmLayerRow(i, polyEngine));
            row->onGeometryChanged = [this]() { resized(); };
            layerCanvas.addAndMakeVisible(row);
            row->setVoice(monoVoice, 1);
            row->updateFromEngine();
        }

        resized();
        repaint();
    }

private:
    int longestLength() const
    {
        int longest = 1;
        const int numLayers = polyEngine.getNumLayers();
        for (int i = 0; i < numLayers; ++i)
            if (auto* layer = polyEngine.getLayer(i))
                longest = juce::jmax(longest, layer->length);
        return longest;
    }

    void addLayer()
    {
        polyEngine.addLayer();
        rebuildLayers();
        if (onLayersChanged)
            onLayersChanged();
    }

    void removeLayer()
    {
        const int numLayers = polyEngine.getNumLayers();
        if (numLayers <= 1)
            return;

        polyEngine.removeLayer(numLayers - 1);
        rebuildLayers();
        if (onLayersChanged)
            onLayersChanged();
    }

    void timerCallback() override
    {
        for (auto* row : layerRows)
        {
            row->setVoice(monoVoice, 1);
            row->updateFromEngine();
        }

        const int longest = longestLength();
        if (longest != cachedLongest)
            resized();
    }

    PolyrhythmEngine& polyEngine;
    juce::TextButton addLayerButton;
    juce::TextButton removeLayerButton;
    juce::Label voiceReadout;
    bool monoVoice = false;
    PatternVisualizer::StackMark stackMarks[16];
    int stackCount = 0;
    int stackLow = 48;
    int stackHigh = 72;
    int partCount = 1;
    int partStep = -1;
    int barSixteenths = 16;
    float gateSeconds = 0.2f;
    PatternVisualizer::PartClock clock;
    juce::Rectangle<int> partColumn;
    juce::Component layerCanvas;
    juce::Viewport layerViewport;
    juce::OwnedArray<PolyrhythmLayerRow> layerRows;
    int cachedLongest = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyrhythmLayerEditor)
};
