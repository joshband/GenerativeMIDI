/*
  ==============================================================================
    PatternAreaTabs.h

    Tabbed interface for pattern display area
    Switches between Pattern Visualizer and Polyrhythm Layer Editor

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "CustomLookAndFeel.h"
#include "PatternVisualizer.h"
#include "PolyrhythmLayerEditor.h"

class PatternAreaTabs : public juce::Component
{
public:
    PatternAreaTabs(PatternVisualizer& patternViz, PolyrhythmLayerEditor& layerEditor)
        : patternVisualizer(patternViz), polyLayerEditor(layerEditor)
    {
        // Tab buttons
        addAndMakeVisible(patternTabButton);
        patternTabButton.setButtonText("Pattern");
        patternTabButton.setRadioGroupId(1);
        patternTabButton.setClickingTogglesState(true);
        patternTabButton.setToggleState(true, juce::dontSendNotification);
        patternTabButton.onClick = [this]() { showPatternView(); };

        addAndMakeVisible(layersTabButton);
        layersTabButton.setButtonText("Layers");
        layersTabButton.setRadioGroupId(1);
        layersTabButton.setClickingTogglesState(true);
        layersTabButton.onClick = [this]() { showLayersView(); };

        // Add the content components
        addAndMakeVisible(patternVisualizer);
        addAndMakeVisible(polyLayerEditor);

        // Show pattern view by default
        showPatternView();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Dark background
        g.setColour(juce::Colour(CustomLookAndFeel::ABYSS_NAVY).darker(0.2f));
        g.fillRoundedRectangle(bounds, 4.0f);
    }

    void resized() override
    {
        auto area = getLocalBounds();

        // Tab buttons at top
        auto tabArea = area.removeFromTop(25);
        int tabWidth = 80;
        patternTabButton.setBounds(tabArea.removeFromLeft(tabWidth).reduced(2));
        layersTabButton.setBounds(tabArea.removeFromLeft(tabWidth).reduced(2));

        area.removeFromTop(2); // Small gap

        // Content area
        patternVisualizer.setBounds(area);
        polyLayerEditor.setBounds(area);
    }

    void showPatternView()
    {
        patternVisualizer.setVisible(true);
        polyLayerEditor.setVisible(false);
        patternTabButton.setToggleState(true, juce::dontSendNotification);
        layersTabButton.setToggleState(false, juce::dontSendNotification);
    }

    void showLayersView()
    {
        patternVisualizer.setVisible(false);
        polyLayerEditor.setVisible(true);
        patternTabButton.setToggleState(false, juce::dontSendNotification);
        layersTabButton.setToggleState(true, juce::dontSendNotification);
        polyLayerEditor.rebuildLayers(); // Refresh when shown
    }

    void setPolyrhythmMode(bool isPolyrhythm)
    {
        // Show/hide layers tab based on mode
        layersTabButton.setVisible(isPolyrhythm);

        // If not in polyrhythm mode, force pattern view
        if (!isPolyrhythm)
        {
            showPatternView();
        }
    }

private:
    PatternVisualizer& patternVisualizer;
    PolyrhythmLayerEditor& polyLayerEditor;

    juce::TextButton patternTabButton;
    juce::TextButton layersTabButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternAreaTabs)
};
