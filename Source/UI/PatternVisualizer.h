/*
  ==============================================================================
    PatternVisualizer.h

    Visual display of Euclidean rhythm patterns
    Gilded steampunk brass VU meter with aether crystal indicators

    Refreshed by the editor's 30 Hz timer — this component does not own a Timer.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "CustomLookAndFeel.h"

class PatternVisualizer : public juce::Component
{
public:
    PatternVisualizer() = default;

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

    void setAccentColor(juce::Colour color)
    {
        accentColor = color;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Obsidian glass display background (Victorian instrument face)
        juce::ColourGradient displayGrad(
            juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN).darker(0.4f),
            bounds.getCentreX(), bounds.getY(),
            juce::Colour(CustomLookAndFeel::ABYSS_NAVY),
            bounds.getCentreX(), bounds.getBottom(),
            false);
        g.setGradientFill(displayGrad);
        g.fillRoundedRectangle(bounds, 6.0f);

        // Brass bezel shadow (depth)
        g.setColour(juce::Colour(CustomLookAndFeel::BRONZE_GOTHIC).withAlpha(0.6f));
        g.fillRoundedRectangle(bounds.reduced(2.0f), 5.0f);

        // Dark glass surface
        g.setColour(juce::Colour(CustomLookAndFeel::ABYSS_NAVY).darker(0.2f));
        g.fillRoundedRectangle(bounds.reduced(4.0f), 4.0f);

        // Polished brass border
        g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED));
        g.drawRoundedRectangle(bounds.reduced(1.0f), 6.0f, 2.5f);

        // Golden highlight rim
        g.setColour(juce::Colour(CustomLookAndFeel::GOLD_TEMPLE).withAlpha(0.4f));
        g.drawRoundedRectangle(bounds.reduced(2.5f), 5.5f, 1.0f);

        if (pattern.empty())
        {
            // Honest empty state for non-Euclidean generators (no fake pattern)
            g.setColour(juce::Colour(CustomLookAndFeel::COPPER_STEAM).withAlpha(0.7f));
            g.setFont(juce::Font(12.0f));
            g.drawText("Euclidean pattern view",
                       bounds.reduced(8.0f),
                       juce::Justification::centred);
            return;
        }

        // Draw LED-style pattern indicators
        float stepWidth = (bounds.getWidth() - 20.0f) / static_cast<float>(pattern.size());
        float ledHeight = bounds.getHeight() - 35.0f;
        float padding = 2.0f;

        for (size_t i = 0; i < pattern.size(); ++i)
        {
            float x = bounds.getX() + 10.0f + static_cast<float>(i) * stepWidth + padding;
            float y = bounds.getY() + 10.0f;
            float w = stepWidth - padding * 2.0f;

            bool isCurrent = (static_cast<int>(i) == currentStep);
            bool isActive = pattern[i];

            if (isActive)
            {
                if (isCurrent)
                {
                    // Current active step - brilliant colored crystal (playing now)
                    juce::ColourGradient crystalGrad(
                        accentColor.brighter(0.2f),
                        x + w * 0.5f, y,
                        accentColor.darker(0.1f),
                        x + w * 0.5f, y + ledHeight,
                        false);
                    g.setGradientFill(crystalGrad);
                    g.fillRoundedRectangle(x, y, w, ledHeight, 2.0f);

                    // Aether glow (energy radiance)
                    juce::ColourGradient aetherGlow(
                        accentColor.withAlpha(0.5f),
                        x + w * 0.5f, y + ledHeight * 0.5f,
                        accentColor.withAlpha(0.0f),
                        x + w * 0.5f, y,
                        true);
                    g.setGradientFill(aetherGlow);
                    g.fillEllipse(x - 2, y + ledHeight * 0.3f, w + 4, ledHeight * 0.4f);

                    // Crystal facet highlight
                    g.setColour(juce::Colours::white.withAlpha(0.8f));
                    g.fillRoundedRectangle(x + w * 0.2f, y + ledHeight * 0.1f,
                                          w * 0.6f, ledHeight * 0.3f, 1.0f);
                }
                else
                {
                    // Non-current active step - glowing crystal (will play)
                    juce::ColourGradient crystalGrad(
                        accentColor.withAlpha(0.8f).brighter(0.1f),
                        x + w * 0.5f, y,
                        accentColor.withAlpha(0.8f).darker(0.2f),
                        x + w * 0.5f, y + ledHeight * 0.7f,
                        false);
                    g.setGradientFill(crystalGrad);
                    g.fillRoundedRectangle(x, y, w, ledHeight * 0.7f, 2.0f);

                    // Crystal highlight
                    g.setColour(juce::Colours::white.withAlpha(0.4f));
                    g.fillRoundedRectangle(x + w * 0.3f, y + ledHeight * 0.1f,
                                          w * 0.4f, ledHeight * 0.2f, 1.0f);
                }
            }
            else
            {
                // Inactive step - dormant crystal
                if (isCurrent)
                {
                    // Current position marker (brass playhead on empty step)
                    g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.5f));
                    g.fillRoundedRectangle(x, y, w, ledHeight * 0.3f, 2.0f);
                }
                else
                {
                    // Dormant crystal chamber
                    g.setColour(juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN).withAlpha(0.4f));
                    g.fillRoundedRectangle(x, y, w, ledHeight * 0.2f, 2.0f);
                }
            }

            // Brass chamber outline (Victorian crystal holder)
            g.setColour(juce::Colour(CustomLookAndFeel::BRONZE_GOTHIC).withAlpha(0.4f));
            g.drawRoundedRectangle(x, y, w, ledHeight, 2.0f, 0.8f);
        }

        // Step markers every 4 beats (engraved brass numerals)
        g.setColour(juce::Colour(CustomLookAndFeel::COPPER_STEAM));
        g.setFont(juce::Font(9.0f, juce::Font::bold));
        for (size_t i = 0; i < pattern.size(); ++i)
        {
            if (i % 4 == 0)
            {
                float x = bounds.getX() + 10.0f + static_cast<float>(i) * stepWidth;
                juce::String label = juce::String(static_cast<int>(i + 1));
                g.drawText(label, x, bounds.getBottom() - 20.0f, stepWidth, 15.0f,
                          juce::Justification::centred);
            }
        }
    }

private:
    std::vector<bool> pattern;
    int currentStep = 0;
    juce::Colour accentColor = juce::Colour(CustomLookAndFeel::AMBER_TESLA);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternVisualizer)
};
