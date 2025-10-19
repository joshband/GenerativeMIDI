/*
  ==============================================================================
    ModulationTarget.h

    Components that accept drag-and-drop modulation routing
    Provides visual feedback for modulation connections

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../Modulation/ModulationMatrix.h"
#include "CustomLookAndFeel.h"

// ============================================================================
// Modulated Slider (extends JUCE Slider + DragAndDropTarget)
// ============================================================================
class ModulatedSlider : public juce::Slider,
                        public juce::DragAndDropTarget
{
public:
    ModulatedSlider(const juce::String& paramID, ModulationMatrix& matrix)
        : parameterID(paramID), modMatrix(matrix)
    {
    }

    // DragAndDropTarget interface
    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override
    {
        // Accept drags that contain an integer (modulation source index)
        return dragSourceDetails.description.isInt();
    }

    void itemDragEnter(const SourceDetails& /*dragSourceDetails*/) override
    {
        isDragOver = true;
        repaint();
    }

    void itemDragExit(const SourceDetails& /*dragSourceDetails*/) override
    {
        isDragOver = false;
        repaint();
    }

    void itemDropped(const SourceDetails& dragSourceDetails) override
    {
        isDragOver = false;

        // Get modulation source index from drag description
        int sourceIndex = dragSourceDetails.description;

        // Check if connection already exists
        bool exists = false;
        for (const auto& conn : modMatrix.getConnections())
        {
            if (conn.sourceIndex == sourceIndex && conn.parameterID == parameterID)
            {
                exists = true;
                DBG("Connection already exists: Source " << sourceIndex << " -> " << parameterID);
                break;
            }
        }

        if (!exists)
        {
            // Create new connection with default depth
            modMatrix.addConnection(sourceIndex, parameterID, 0.5f);
            DBG("Created modulation connection: Source " << sourceIndex << " -> " << parameterID);
        }

        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        // Draw base slider
        juce::Slider::paint(g);

        // Draw drag-over indicator
        if (isDragOver)
        {
            auto bounds = getLocalBounds().toFloat();
            g.setColour(juce::Colour(CustomLookAndFeel::AETHER_CYAN).withAlpha(0.3f));
            g.fillRoundedRectangle(bounds, 4.0f);

            g.setColour(juce::Colour(CustomLookAndFeel::AETHER_CYAN));
            g.drawRoundedRectangle(bounds, 4.0f, 2.0f);
        }

        // Draw modulation indicator if active
        if (getSliderStyle() == juce::Slider::Rotary && hasActiveModulation())
        {
            drawModulationIndicator(g);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        // Right-click to show modulation connections menu
        if (e.mods.isPopupMenu())
        {
            showModulationMenu();
        }
        else
        {
            juce::Slider::mouseDown(e);
        }
    }

private:
    void drawModulationIndicator(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f - 4.0f;

        // Get modulation value (-1.0 to +1.0 range)
        float modValue = getCurrentModulation();
        float modDepth = getModulationDepth();

        // Draw modulation ring (cyan for positive, amber for negative)
        if (std::abs(modValue) > 0.01f)
        {
            float ringThickness = 3.0f;
            juce::Colour modColour = modValue > 0.0f
                ? juce::Colour(CustomLookAndFeel::AETHER_CYAN)
                : juce::Colour(CustomLookAndFeel::AMBER_TESLA);

            // Modulation arc (shows current modulation amount)
            float startAngle = juce::MathConstants<float>::pi * 1.25f; // Bottom left
            float modAngle = modValue * juce::MathConstants<float>::pi * 1.5f; // Max ±270 degrees

            juce::Path modArc;
            modArc.addCentredArc(centerX, centerY, radius, radius,
                               0.0f, startAngle, startAngle + modAngle, true);

            g.setColour(modColour.withAlpha(0.6f));
            g.strokePath(modArc, juce::PathStrokeType(ringThickness));

            // Glow effect
            g.setColour(modColour.withAlpha(0.2f));
            g.strokePath(modArc, juce::PathStrokeType(ringThickness + 4.0f));
        }

        // Draw depth indicator (subtle ring showing max modulation depth)
        if (modDepth > 0.01f)
        {
            g.setColour(juce::Colour(CustomLookAndFeel::GOLD_TEMPLE).withAlpha(0.3f * modDepth));
            g.drawEllipse(centerX - radius, centerY - radius,
                         radius * 2.0f, radius * 2.0f, 1.0f);
        }
    }

    void showModulationMenu()
    {
        juce::PopupMenu menu;

        // Show current connections
        bool hasConnections = false;
        int menuItemID = 1;

        for (int i = 0; i < modMatrix.getNumConnections(); ++i)
        {
            auto* conn = modMatrix.getConnection(i);
            if (conn && conn->parameterID == parameterID)
            {
                auto* source = modMatrix.getSource(conn->sourceIndex);
                if (source)
                {
                    hasConnections = true;

                    juce::PopupMenu connMenu;
                    connMenu.addItem(menuItemID + 1, "Depth: " + juce::String(conn->depth, 2), false, false);
                    connMenu.addItem(menuItemID + 2, conn->enabled ? "Disable" : "Enable", true, conn->enabled);
                    connMenu.addItem(menuItemID + 3, "Remove Connection", true, false);

                    menu.addSubMenu(source->getName(), connMenu);
                    menuItemID += 10;
                }
            }
        }

        if (!hasConnections)
        {
            menu.addItem(1, "No modulation connections", false);
        }

        menu.addSeparator();
        menu.addItem(1000, "Drag modulation sources here...", false);

        menu.showMenuAsync(juce::PopupMenu::Options());
    }

    float getModulationDepth() const
    {
        float totalDepth = 0.0f;
        for (const auto& conn : modMatrix.getConnections())
        {
            if (conn.parameterID == parameterID && conn.enabled)
            {
                auto* source = modMatrix.getSource(conn.sourceIndex);
                if (source && source->getEnabled())
                {
                    totalDepth += std::abs(conn.depth);
                }
            }
        }
        return juce::jlimit(0.0f, 1.0f, totalDepth);
    }

    float getCurrentModulation() const
    {
        return modMatrix.calculateModulation(parameterID);
    }

    bool hasActiveModulation() const
    {
        for (const auto& conn : modMatrix.getConnections())
        {
            if (conn.parameterID == parameterID && conn.enabled)
            {
                auto* source = modMatrix.getSource(conn.sourceIndex);
                if (source && source->getEnabled())
                    return true;
            }
        }
        return false;
    }

    juce::String parameterID;
    ModulationMatrix& modMatrix;
    bool isDragOver = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulatedSlider)
};
