/*
  ==============================================================================
    PolyrhythmLayerEditor.h

    Visual editor for polyrhythm layers with per-layer controls
    Victorian steampunk multi-layer display with brass controls

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "CustomLookAndFeel.h"
#include "../Core/PolyrhythmEngine.h"

class PolyrhythmLayerRow : public juce::Component
{
public:
    PolyrhythmLayerRow(int layerIdx, PolyrhythmEngine& engine)
        : layerIndex(layerIdx), polyEngine(engine)
    {
        // Enable/disable toggle
        addAndMakeVisible(enableButton);
        enableButton.setButtonText("");
        enableButton.setClickingTogglesState(true);
        enableButton.setToggleState(true, juce::dontSendNotification);
        enableButton.onClick = [this]() { onEnableChanged(); };

        // Layer number label
        addAndMakeVisible(layerLabel);
        layerLabel.setText(juce::String(layerIndex + 1), juce::dontSendNotification);
        layerLabel.setFont(juce::Font(14.0f, juce::Font::bold));
        layerLabel.setJustificationType(juce::Justification::centred);
        layerLabel.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::GOLD_TEMPLE));

        // Division slider (time division)
        addAndMakeVisible(divisionSlider);
        divisionSlider.setRange(1, 32, 1);
        divisionSlider.setValue(4, juce::dontSendNotification);
        divisionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        divisionSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        divisionSlider.onValueChange = [this]() { onDivisionChanged(); };

        // Length slider (pattern length)
        addAndMakeVisible(lengthSlider);
        lengthSlider.setRange(1, 32, 1);
        lengthSlider.setValue(16, juce::dontSendNotification);
        lengthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        lengthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        lengthSlider.onValueChange = [this]() { onLengthChanged(); };

        // Pitch offset slider
        addAndMakeVisible(pitchSlider);
        pitchSlider.setRange(-24, 24, 1);
        pitchSlider.setValue(0, juce::dontSendNotification);
        pitchSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        pitchSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        pitchSlider.onValueChange = [this]() { onPitchChanged(); };

        // Velocity multiplier slider
        addAndMakeVisible(velocitySlider);
        velocitySlider.setRange(0.0, 2.0, 0.01);
        velocitySlider.setValue(1.0, juce::dontSendNotification);
        velocitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
        velocitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        velocitySlider.onValueChange = [this]() { onVelocityChanged(); };

        // Pattern display (mini visualizer)
        addAndMakeVisible(patternDisplay);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Brass panel background
        juce::ColourGradient panelGrad(
            juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN).brighter(0.05f),
            bounds.getCentreX(), bounds.getY(),
            juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN).darker(0.1f),
            bounds.getCentreX(), bounds.getBottom(),
            false);
        g.setGradientFill(panelGrad);
        g.fillRoundedRectangle(bounds, 4.0f);

        // Brass rim
        g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.5f));
        g.drawRoundedRectangle(bounds, 4.0f, 1.5f);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(3);

        // Left section: Enable toggle + layer number
        auto leftSection = area.removeFromLeft(50);
        enableButton.setBounds(leftSection.removeFromLeft(25).reduced(2));
        layerLabel.setBounds(leftSection);

        area.removeFromLeft(5); // Spacing

        // Pattern display (compact mini visualizer)
        patternDisplay.setBounds(area.removeFromLeft(150).reduced(1));

        area.removeFromLeft(5); // Spacing

        // Right section: Sliders (compact)
        int sliderWidth = 100;
        divisionSlider.setBounds(area.removeFromLeft(sliderWidth).reduced(1));
        area.removeFromLeft(3);
        lengthSlider.setBounds(area.removeFromLeft(sliderWidth).reduced(1));
        area.removeFromLeft(3);
        pitchSlider.setBounds(area.removeFromLeft(sliderWidth).reduced(1));
        area.removeFromLeft(3);
        velocitySlider.setBounds(area.removeFromLeft(sliderWidth).reduced(1));
    }

    void updateFromEngine()
    {
        auto* layer = polyEngine.getLayer(layerIndex);
        if (layer)
        {
            enableButton.setToggleState(layer->enabled, juce::dontSendNotification);
            divisionSlider.setValue(layer->division, juce::dontSendNotification);
            lengthSlider.setValue(layer->length, juce::dontSendNotification);

            // Update pattern display
            patternDisplay.setPattern(layer->pattern);
            patternDisplay.setCurrentStep(layer->currentStep);
        }
    }

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
        updateFromEngine(); // Refresh pattern display
    }

    void onPitchChanged()
    {
        // For now, just log the change - actual pitch will be set when pattern is created
        DBG("Pitch offset changed to: " << pitchSlider.getValue());
    }

    void onVelocityChanged()
    {
        // For now, just log the change - actual velocity will be set when pattern is created
        DBG("Velocity multiplier changed to: " << velocitySlider.getValue());
    }

    int layerIndex;
    PolyrhythmEngine& polyEngine;

    juce::ToggleButton enableButton;
    juce::Label layerLabel;
    juce::Slider divisionSlider;
    juce::Slider lengthSlider;
    juce::Slider pitchSlider;
    juce::Slider velocitySlider;

    // Mini pattern visualizer
    class MiniPatternDisplay : public juce::Component
    {
    public:
        void setPattern(const std::vector<bool>& newPattern)
        {
            pattern = newPattern;
            repaint();
        }

        void setCurrentStep(int step)
        {
            currentStep = step;
            repaint();
        }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();

            // Dark background
            g.setColour(juce::Colour(CustomLookAndFeel::ABYSS_NAVY).darker(0.2f));
            g.fillRoundedRectangle(bounds, 2.0f);

            if (pattern.empty())
                return;

            // Draw mini step indicators
            float stepWidth = bounds.getWidth() / pattern.size();
            float stepHeight = bounds.getHeight() - 4.0f;

            for (size_t i = 0; i < pattern.size(); ++i)
            {
                float x = bounds.getX() + i * stepWidth + 1.0f;
                float y = bounds.getY() + 2.0f;
                float w = stepWidth - 2.0f;

                bool isCurrent = (static_cast<int>(i) == currentStep);
                bool isActive = pattern[i];

                if (isActive)
                {
                    if (isCurrent)
                    {
                        // Current active - amber
                        g.setColour(juce::Colour(CustomLookAndFeel::AMBER_TESLA));
                        g.fillRoundedRectangle(x, y, w, stepHeight, 1.0f);
                    }
                    else
                    {
                        // Active - cyan
                        g.setColour(juce::Colour(CustomLookAndFeel::AETHER_CYAN));
                        g.fillRoundedRectangle(x, y, w, stepHeight * 0.7f, 1.0f);
                    }
                }
                else if (isCurrent)
                {
                    // Current position marker
                    g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.4f));
                    g.fillRoundedRectangle(x, y, w, stepHeight * 0.3f, 1.0f);
                }
            }
        }

    private:
        std::vector<bool> pattern;
        int currentStep = 0;
    } patternDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyrhythmLayerRow)
};

class PolyrhythmLayerEditor : public juce::Component, private juce::Timer
{
public:
    PolyrhythmLayerEditor(PolyrhythmEngine& engine)
        : polyEngine(engine)
    {
        // Add layer button
        addAndMakeVisible(addLayerButton);
        addLayerButton.setButtonText("+ Add Layer");
        addLayerButton.onClick = [this]() { addLayer(); };

        // Remove layer button
        addAndMakeVisible(removeLayerButton);
        removeLayerButton.setButtonText("- Remove Layer");
        removeLayerButton.onClick = [this]() { removeLayer(); };

        // Column headers
        addAndMakeVisible(headersLabel);
        headersLabel.setFont(juce::Font(10.0f, juce::Font::plain));
        headersLabel.setJustificationType(juce::Justification::left);
        headersLabel.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::COPPER_STEAM));

        // Create initial layer rows
        rebuildLayers();

        // Start timer for updating displays
        startTimerHz(30);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Background panel
        g.setColour(juce::Colour(CustomLookAndFeel::ABYSS_NAVY).darker(0.3f));
        g.fillRoundedRectangle(bounds, 6.0f);

        // Brass border
        g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.6f));
        g.drawRoundedRectangle(bounds, 6.0f, 2.0f);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(5);

        // Top: Buttons
        auto buttonArea = area.removeFromTop(25);
        addLayerButton.setBounds(buttonArea.removeFromLeft(90).reduced(2));
        buttonArea.removeFromLeft(5);
        removeLayerButton.setBounds(buttonArea.removeFromLeft(110).reduced(2));

        area.removeFromTop(3);

        // Headers
        auto headerArea = area.removeFromTop(15);
        headersLabel.setBounds(headerArea);

        area.removeFromTop(3);

        // Layer rows - calculate available space
        int rowHeight = 40;
        int numRows = layerRows.size();

        // Limit the number of visible rows if needed
        int maxVisibleRows = (area.getHeight() - 5) / (rowHeight + 3);
        int visibleRows = juce::jmin(numRows, maxVisibleRows);

        for (int i = 0; i < visibleRows && i < layerRows.size(); ++i)
        {
            layerRows[i]->setBounds(area.removeFromTop(rowHeight));
            area.removeFromTop(3);
        }
    }

    void rebuildLayers()
    {
        DBG("Rebuilding layers...");

        // Clear existing rows
        layerRows.clear();

        // Create a row for each layer
        int numLayers = polyEngine.getNumLayers();
        DBG("Creating " << numLayers << " layer rows");

        for (int i = 0; i < numLayers; ++i)
        {
            auto* row = layerRows.add(new PolyrhythmLayerRow(i, polyEngine));
            addAndMakeVisible(row);
            row->updateFromEngine();
            DBG("Created layer row " << i);
        }

        // Update header text (compact)
        headersLabel.setText("  #  Pattern           Div   Len   Pitch  Vel",
                            juce::dontSendNotification);

        resized();
        repaint();
        DBG("Rebuild complete, " << layerRows.size() << " rows visible");
    }

private:
    void addLayer()
    {
        int newLayerIndex = polyEngine.addLayer();
        DBG("Added layer, total layers: " << polyEngine.getNumLayers());
        rebuildLayers();
        resized();
        repaint();
    }

    void removeLayer()
    {
        int numLayers = polyEngine.getNumLayers();
        if (numLayers > 1) // Keep at least one layer
        {
            polyEngine.removeLayer(numLayers - 1);
            DBG("Removed layer, total layers: " << polyEngine.getNumLayers());
            rebuildLayers();
            resized();
            repaint();
        }
        else
        {
            DBG("Cannot remove last layer");
        }
    }

    void timerCallback() override
    {
        // Update all layer displays
        for (auto* row : layerRows)
        {
            row->updateFromEngine();
        }
    }

    PolyrhythmEngine& polyEngine;
    juce::TextButton addLayerButton;
    juce::TextButton removeLayerButton;
    juce::Label headersLabel;

    juce::OwnedArray<PolyrhythmLayerRow> layerRows;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyrhythmLayerEditor)
};
