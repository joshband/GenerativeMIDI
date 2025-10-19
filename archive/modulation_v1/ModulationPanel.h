/*
  ==============================================================================
    ModulationPanel.h

    Visual modulation source panel with JUCE drag-and-drop routing
    Shows LFOs, Envelopes, Random sources
    Uses Victorian steampunk UI assets from SynaptikUIToolkit

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "CustomLookAndFeel.h"
#include "../Modulation/ModulationMatrix.h"

// Helper function to load UI assets from SynaptikUIToolkit submodule
inline juce::Image loadUIAsset(const juce::String& relativePath)
{
    // Path to SynaptikUIToolkit submodule
    juce::File projectRoot = juce::File::getCurrentWorkingDirectory();

    // Try common project layouts
    std::vector<juce::String> possiblePaths = {
        "../SynaptikUIToolkit/themes/victorian-steampunk/" + relativePath,
        "../../SynaptikUIToolkit/themes/victorian-steampunk/" + relativePath,
        "../../../SynaptikUIToolkit/themes/victorian-steampunk/" + relativePath
    };

    for (const auto& path : possiblePaths)
    {
        juce::File assetFile = projectRoot.getChildFile(path);
        if (assetFile.existsAsFile())
        {
            juce::Image img = juce::ImageCache::getFromFile(assetFile);
            if (img.isValid())
                return img;
        }
    }

    // Return empty image if not found
    return juce::Image();
}

// ============================================================================
// Modulation Source Button (draggable using JUCE DragAndDropContainer)
// ============================================================================
class ModulationSourceButton : public juce::Component
{
public:
    ModulationSourceButton(int sourceIdx, ModulationSource* src)
        : sourceIndex(sourceIdx), source(src)
    {
        setSize(80, 80);

        // Load button background image
        buttonImage = loadUIAsset("ui-elements/buttons/img_8168_128.png");
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Draw button image as background
        if (buttonImage.isValid())
        {
            // Calculate aspect-correct bounds for the image
            float imgAspect = static_cast<float>(buttonImage.getWidth()) / buttonImage.getHeight();
            float boundsAspect = bounds.getWidth() / bounds.getHeight();

            juce::Rectangle<float> imageBounds = bounds;

            if (imgAspect > boundsAspect)
                imageBounds = imageBounds.withSizeKeepingCentre(bounds.getWidth(), bounds.getWidth() / imgAspect);
            else
                imageBounds = imageBounds.withSizeKeepingCentre(bounds.getHeight() * imgAspect, bounds.getHeight());

            // Brighten when dragging
            float alpha = isDragging ? 1.0f : 0.85f;
            g.setOpacity(alpha);
            g.drawImage(buttonImage, imageBounds, juce::RectanglePlacement::centred);
            g.setOpacity(1.0f);
        }
        else
        {
            // Fallback to procedural brass button if image not loaded
            juce::ColourGradient panelGrad(
                juce::Colour(CustomLookAndFeel::BRASS_AGED).brighter(isDragging ? 0.3f : 0.1f),
                bounds.getCentreX(), bounds.getY(),
                juce::Colour(CustomLookAndFeel::BRASS_AGED).darker(0.2f),
                bounds.getCentreX(), bounds.getBottom(),
                false);
            g.setGradientFill(panelGrad);
            g.fillRoundedRectangle(bounds, 4.0f);

            g.setColour(juce::Colour(CustomLookAndFeel::GOLD_TEMPLE).withAlpha(0.6f));
            g.drawRoundedRectangle(bounds, 4.0f, 1.5f);
        }

        // Source active glow (drawn over the image)
        if (source && source->getEnabled())
        {
            float glowValue = std::abs(source->getNormalizedValue());
            if (glowValue > 0.1f)
            {
                juce::ColourGradient glow(
                    juce::Colour(CustomLookAndFeel::AETHER_CYAN).withAlpha(glowValue * 0.4f),
                    bounds.getCentreX(), bounds.getCentreY(),
                    juce::Colour(CustomLookAndFeel::AETHER_CYAN).withAlpha(0.0f),
                    bounds.getRight(), bounds.getY(),
                    false);
                g.setGradientFill(glow);
                g.fillEllipse(bounds.reduced(10.0f));
            }
        }

        // Source name (in small text at bottom)
        g.setColour(juce::Colour(CustomLookAndFeel::GOLD_TEMPLE));
        g.setFont(juce::Font(9.0f, juce::Font::bold));
        g.drawText(source ? source->getName() : "Empty",
                   bounds.removeFromBottom(16).reduced(2.0f),
                   juce::Justification::centred);

        // Enabled/disabled overlay
        if (source && !source->getEnabled())
        {
            g.setColour(juce::Colours::black.withAlpha(0.6f));
            g.fillRoundedRectangle(bounds, 4.0f);

            g.setColour(juce::Colour(CustomLookAndFeel::BRONZE_GOTHIC));
            g.setFont(juce::Font(11.0f, juce::Font::bold));
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
    juce::Image buttonImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationSourceButton)
};

// ============================================================================
// Modulation Panel
// ============================================================================
class ModulationPanel : public juce::Component,
                        private juce::Timer
{
public:
    ModulationPanel(ModulationMatrix& matrix)
        : modMatrix(matrix)
    {
        // Load panel background image
        panelBackground = loadUIAsset("ui-elements/panels/img_8119_512.png");

        // Create source buttons
        rebuildSourceButtons();

        // Start timer for visual updates
        startTimerHz(30);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Draw panel background image if loaded
        if (panelBackground.isValid())
        {
            // Draw tiled or stretched background
            g.setOpacity(0.7f);
            g.drawImage(panelBackground, bounds, juce::RectanglePlacement::fillDestination);
            g.setOpacity(1.0f);

            // Dark overlay for legibility
            g.setColour(juce::Colour(CustomLookAndFeel::ABYSS_NAVY).withAlpha(0.3f));
            g.fillRoundedRectangle(bounds, 6.0f);
        }
        else
        {
            // Fallback to procedural background
            g.setColour(juce::Colour(CustomLookAndFeel::ABYSS_NAVY).darker(0.3f));
            g.fillRoundedRectangle(bounds, 6.0f);
        }

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

        // Layout source buttons horizontally with improved spacing
        int buttonSpacing = 8;
        int buttonSize = 80;

        // Center the buttons horizontally
        int totalWidth = (buttonSize * sourceButtons.size()) + (buttonSpacing * (sourceButtons.size() - 1));
        int startX = (area.getWidth() - totalWidth) / 2;

        auto buttonArea = area.withX(area.getX() + startX).withWidth(totalWidth);

        for (auto* button : sourceButtons)
        {
            button->setBounds(buttonArea.removeFromLeft(buttonSize).removeFromTop(buttonSize));
            buttonArea.removeFromLeft(buttonSpacing);
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
    juce::Image panelBackground;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationPanel)
};
