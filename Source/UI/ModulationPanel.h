/*
  ==============================================================================
    ModulationPanel.h

    Visual modulation source panel with JUCE drag-and-drop routing
    Shows LFOs, Envelopes, Random sources

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "CustomLookAndFeel.h"
#include "../Modulation/ModulationMatrix.h"

// ============================================================================
// Modulation Source Button (draggable using JUCE DragAndDropContainer)
// ============================================================================
class ModulationSourceButton : public juce::Component
{
public:
    ModulationSourceButton(int sourceIdx, ModulationSource* src)
        : sourceIndex(sourceIdx), source(src)
    {
        setSize(80, 60);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Source active glow
        if (source && source->getEnabled())
        {
            float glowValue = std::abs(source->getNormalizedValue());
            juce::ColourGradient glow(
                juce::Colour(CustomLookAndFeel::AETHER_CYAN).withAlpha(glowValue * 0.3f),
                bounds.getCentreX(), bounds.getCentreY(),
                juce::Colour(CustomLookAndFeel::AMBER_TESLA).withAlpha(0.0f),
                bounds.getCentreX(), bounds.getY(),
                true);
            g.setGradientFill(glow);
            g.fillRoundedRectangle(bounds.expanded(4.0f), 6.0f);
        }

        // Brass panel background
        juce::ColourGradient panelGrad(
            juce::Colour(CustomLookAndFeel::BRASS_AGED).brighter(isDragging ? 0.3f : 0.1f),
            bounds.getCentreX(), bounds.getY(),
            juce::Colour(CustomLookAndFeel::BRASS_AGED).darker(0.2f),
            bounds.getCentreX(), bounds.getBottom(),
            false);
        g.setGradientFill(panelGrad);
        g.fillRoundedRectangle(bounds, 4.0f);

        // Brass border
        g.setColour(juce::Colour(CustomLookAndFeel::GOLD_TEMPLE).withAlpha(isDragging ? 1.0f : 0.6f));
        g.drawRoundedRectangle(bounds, 4.0f, isDragging ? 2.5f : 1.5f);

        // Source name
        g.setColour(juce::Colour(CustomLookAndFeel::GOLD_TEMPLE));
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText(source ? source->getName() : "Empty",
                   bounds.reduced(4.0f),
                   juce::Justification::centredTop);

        // Value indicator
        if (source)
        {
            float value = source->getNormalizedValue();
            juce::String valueText;

            if (source->getBipolar())
                valueText = juce::String(value, 2);
            else
                valueText = juce::String(value * 100.0f, 0) + "%";

            g.setFont(juce::Font(10.0f));
            g.setColour(juce::Colour(CustomLookAndFeel::AETHER_CYAN));
            g.drawText(valueText,
                       bounds.reduced(4.0f).withTrimmedTop(20),
                       juce::Justification::centred);
        }

        // Enabled/disabled indicator
        if (source && !source->getEnabled())
        {
            g.setColour(juce::Colours::black.withAlpha(0.5f));
            g.fillRoundedRectangle(bounds, 4.0f);

            g.setColour(juce::Colour(CustomLookAndFeel::BRONZE_GOTHIC));
            g.setFont(juce::Font(10.0f));
            g.drawText("OFF", bounds, juce::Justification::centred);
        }
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (!isDragging && e.getDistanceFromDragStart() > 5)
        {
            isDragging = true;

            // Start drag using JUCE's drag and drop system
            auto* container = juce::DragAndDropContainer::findParentDragContainerFor(this);
            if (container)
            {
                // Create drag description with source index
                juce::var dragDescription;
                dragDescription = sourceIndex;

                container->startDragging(dragDescription, this);
            }

            repaint();
        }
    }

    void mouseUp(const juce::MouseEvent& /*e*/) override
    {
        isDragging = false;
        repaint();
    }

    int getSourceIndex() const { return sourceIndex; }
    ModulationSource* getSource() const { return source; }

private:
    int sourceIndex;
    ModulationSource* source;
    bool isDragging = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationSourceButton)
};

// ============================================================================
// Modulation Panel (DragAndDropContainer)
// ============================================================================
class ModulationPanel : public juce::Component,
                        public juce::DragAndDropContainer,
                        private juce::Timer
{
public:
    ModulationPanel(ModulationMatrix& matrix)
        : modMatrix(matrix)
    {
        // Create source buttons
        rebuildSourceButtons();

        // Start timer for visual updates
        startTimerHz(30);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Dark background
        g.setColour(juce::Colour(CustomLookAndFeel::ABYSS_NAVY).darker(0.3f));
        g.fillRoundedRectangle(bounds, 6.0f);

        // Brass border
        g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.6f));
        g.drawRoundedRectangle(bounds, 6.0f, 2.0f);

        // Title
        g.setColour(juce::Colour(CustomLookAndFeel::GOLD_TEMPLE));
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText("MODULATION SOURCES - Drag to Knobs",
                   bounds.reduced(10, 5).removeFromTop(20),
                   juce::Justification::centredLeft);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);
        area.removeFromTop(25); // Title space

        // Layout source buttons horizontally
        int buttonSpacing = 5;
        int buttonWidth = 80;

        for (auto* button : sourceButtons)
        {
            button->setBounds(area.removeFromLeft(buttonWidth).removeFromTop(60));
            area.removeFromLeft(buttonSpacing);
        }
    }

    void rebuildSourceButtons()
    {
        sourceButtons.clear();

        int numSources = modMatrix.getNumSources();
        for (int i = 0; i < numSources; ++i)
        {
            auto* source = modMatrix.getSource(i);
            if (source)
            {
                auto* button = sourceButtons.add(new ModulationSourceButton(i, source));
                addAndMakeVisible(button);
            }
        }

        resized();
    }

private:
    void timerCallback() override
    {
        // Update visual display
        for (auto* button : sourceButtons)
        {
            button->repaint();
        }
    }

    ModulationMatrix& modMatrix;
    juce::OwnedArray<ModulationSourceButton> sourceButtons;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationPanel)
};
