/*
  ==============================================================================
    ModulationTarget.h

    Mixin component that makes any control accept drag-and-drop modulation routing
    Provides visual feedback for modulation connections

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../Modulation/ModulationMatrix.h"
#include "CustomLookAndFeel.h"

// ============================================================================
// Modulation Target Component (Mixin)
// ============================================================================
class ModulationTarget
{
public:
    ModulationTarget(const juce::String& paramID, ModulationMatrix& matrix)
        : parameterID(paramID), modMatrix(matrix)
    {
    }

    virtual ~ModulationTarget() = default;

    // Called when a modulation source is dropped on this target
    void handleModulationDrop(int sourceIndex, juce::Point<int> /*dropPosition*/)
    {
        // Check if connection already exists
        bool exists = false;
        for (const auto& conn : modMatrix.getConnections())
        {
            if (conn.sourceIndex == sourceIndex && conn.parameterID == parameterID)
            {
                exists = true;
                break;
            }
        }

        if (!exists)
        {
            // Create new connection with default depth
            modMatrix.addConnection(sourceIndex, parameterID, 0.5f);
            DBG("Created modulation connection: Source " << sourceIndex << " -> " << parameterID);
        }
        else
        {
            DBG("Modulation connection already exists");
        }
    }

    // Get modulation depth for visual display
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

    // Get current modulation value for visual display
    float getCurrentModulation() const
    {
        return modMatrix.calculateModulation(parameterID);
    }

    // Check if this target has any active modulation
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

    juce::String getParameterID() const { return parameterID; }

protected:
    juce::String parameterID;
    ModulationMatrix& modMatrix;
};

// ============================================================================
// Modulated Slider (extends JUCE Slider + ModulationTarget)
// ============================================================================
class ModulatedSlider : public juce::Slider, public ModulationTarget
{
public:
    ModulatedSlider(const juce::String& paramID, ModulationMatrix& matrix)
        : ModulationTarget(paramID, matrix)
    {
    }

    void paint(juce::Graphics& g) override
    {
        // Draw base slider
        juce::Slider::paint(g);

        // Draw modulation ring if active
        if (getSliderStyle() == juce::Slider::Rotary && hasActiveModulation())
        {
            drawModulationIndicator(g);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        // Check if this is a modulation drop from ModulationPanel
        if (e.mods.isAltDown() && e.eventComponent != this)
        {
            // This is a modulation drag-and-drop operation
            // The actual drop handling is done by the parent component
            return;
        }

        juce::Slider::mouseDown(e);
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulatedSlider)
};
