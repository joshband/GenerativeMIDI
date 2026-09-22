/*
  ==============================================================================
    PatternVisualizer.h

    Visual display of Euclidean rhythm patterns, with a live activity fallback
    for non-pattern generators. Never placeholder-only.

    Refreshed by the editor's 30 Hz timer — this component does not own a Timer.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "CustomLookAndFeel.h"
#include <array>

class PatternVisualizer : public juce::Component
{
public:
    PatternVisualizer() = default;

    void setPattern(const std::vector<bool>& newPattern)
    {
        pattern = newPattern;
        if (!pattern.empty())
            lastPattern = pattern;
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

    /** Short live status for non-pattern modes (e.g. "Markov · generating"). */
    void setStatusText(const juce::String& text)
    {
        if (statusText != text)
        {
            statusText = text;
            repaint();
        }
    }

    /** 0..1 activity pulse for fallback tick display. */
    void setActivityLevel(float level)
    {
        activityLevel = juce::jlimit(0.0f, 1.0f, level);
        repaint();
    }

    /** Push a tick into the rolling activity history (call when notes fire). */
    void pushActivityTick(bool active)
    {
        for (int i = 0; i < kActivitySlots - 1; ++i)
            activityTicks[static_cast<size_t>(i)] = activityTicks[static_cast<size_t>(i + 1)];
        activityTicks[static_cast<size_t>(kActivitySlots - 1)] = active;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        juce::ColourGradient displayGrad(
            juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN).darker(0.4f),
            bounds.getCentreX(), bounds.getY(),
            juce::Colour(CustomLookAndFeel::ABYSS_NAVY),
            bounds.getCentreX(), bounds.getBottom(),
            false);
        g.setGradientFill(displayGrad);
        g.fillRoundedRectangle(bounds, 6.0f);

        g.setColour(juce::Colour(CustomLookAndFeel::BRONZE_GOTHIC).withAlpha(0.6f));
        g.fillRoundedRectangle(bounds.reduced(2.0f), 5.0f);

        g.setColour(juce::Colour(CustomLookAndFeel::ABYSS_NAVY).darker(0.2f));
        g.fillRoundedRectangle(bounds.reduced(4.0f), 4.0f);

        g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED));
        g.drawRoundedRectangle(bounds.reduced(1.0f), 6.0f, 2.5f);

        g.setColour(juce::Colour(CustomLookAndFeel::GOLD_TEMPLE).withAlpha(0.4f));
        g.drawRoundedRectangle(bounds.reduced(2.5f), 5.5f, 1.0f);

        if (!pattern.empty())
        {
            paintPatternCells(g, bounds);
            return;
        }

        paintActivityFallback(g, bounds);
    }

private:
    static constexpr int kActivitySlots = 24;

    void paintPatternCells(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
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
                    juce::ColourGradient crystalGrad(
                        accentColor.brighter(0.2f),
                        x + w * 0.5f, y,
                        accentColor.darker(0.1f),
                        x + w * 0.5f, y + ledHeight,
                        false);
                    g.setGradientFill(crystalGrad);
                    g.fillRoundedRectangle(x, y, w, ledHeight, 2.0f);

                    juce::ColourGradient aetherGlow(
                        accentColor.withAlpha(0.5f),
                        x + w * 0.5f, y + ledHeight * 0.5f,
                        accentColor.withAlpha(0.0f),
                        x + w * 0.5f, y,
                        true);
                    g.setGradientFill(aetherGlow);
                    g.fillEllipse(x - 2, y + ledHeight * 0.3f, w + 4, ledHeight * 0.4f);

                    g.setColour(juce::Colours::white.withAlpha(0.8f));
                    g.fillRoundedRectangle(x + w * 0.2f, y + ledHeight * 0.1f,
                                          w * 0.6f, ledHeight * 0.3f, 1.0f);
                }
                else
                {
                    juce::ColourGradient crystalGrad(
                        accentColor.withAlpha(0.8f).brighter(0.1f),
                        x + w * 0.5f, y,
                        accentColor.withAlpha(0.8f).darker(0.2f),
                        x + w * 0.5f, y + ledHeight * 0.7f,
                        false);
                    g.setGradientFill(crystalGrad);
                    g.fillRoundedRectangle(x, y, w, ledHeight * 0.7f, 2.0f);

                    g.setColour(juce::Colours::white.withAlpha(0.4f));
                    g.fillRoundedRectangle(x + w * 0.3f, y + ledHeight * 0.1f,
                                          w * 0.4f, ledHeight * 0.2f, 1.0f);
                }
            }
            else
            {
                if (isCurrent)
                {
                    g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.5f));
                    g.fillRoundedRectangle(x, y, w, ledHeight * 0.3f, 2.0f);
                }
                else
                {
                    g.setColour(juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN).withAlpha(0.4f));
                    g.fillRoundedRectangle(x, y, w, ledHeight * 0.2f, 2.0f);
                }
            }

            g.setColour(juce::Colour(CustomLookAndFeel::BRONZE_GOTHIC).withAlpha(0.4f));
            g.drawRoundedRectangle(x, y, w, ledHeight, 2.0f, 0.8f);
        }

        g.setColour(juce::Colour(CustomLookAndFeel::COPPER_STEAM));
        g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
        for (size_t i = 0; i < pattern.size(); ++i)
        {
            if (i % 4 == 0)
            {
                float x = bounds.getX() + 10.0f + static_cast<float>(i) * stepWidth;
                juce::String label = juce::String(static_cast<int>(i + 1));
                g.drawText(label,
                           juce::Rectangle<float>(x, bounds.getBottom() - 20.0f, stepWidth, 15.0f),
                           juce::Justification::centred);
            }
        }
    }

    void paintActivityFallback(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        auto tickArea = bounds.reduced(14.0f, 18.0f);
        tickArea.removeFromBottom(22.0f);

        const float slotW = tickArea.getWidth() / static_cast<float>(kActivitySlots);
        const float baseH = tickArea.getHeight();

        for (int i = 0; i < kActivitySlots; ++i)
        {
            const bool lit = activityTicks[static_cast<size_t>(i)];
            const float age = static_cast<float>(i) / static_cast<float>(kActivitySlots - 1);
            const float h = lit ? (0.35f + 0.55f * age + 0.1f * activityLevel) * baseH
                                : 0.12f * baseH;
            const float x = tickArea.getX() + static_cast<float>(i) * slotW + 1.0f;
            const float y = tickArea.getBottom() - h;
            const float w = juce::jmax(2.0f, slotW - 2.0f);

            if (lit)
            {
                g.setColour(accentColor.withAlpha(0.35f + 0.55f * age));
                g.fillRoundedRectangle(x, y, w, h, 1.5f);
            }
            else
            {
                g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.28f));
                g.fillRoundedRectangle(x, y, w, juce::jmax(h, 0.22f * baseH), 1.5f);
            }
        }

        // Dim ghost of last Euclidean pattern as secondary context when available
        if (!lastPattern.empty() && lastPattern.size() <= 32)
        {
            const float ghostY = tickArea.getY() + 2.0f;
            const float ghostH = 6.0f;
            const float gw = (tickArea.getWidth() - 4.0f) / static_cast<float>(lastPattern.size());
            for (size_t i = 0; i < lastPattern.size(); ++i)
            {
                if (!lastPattern[i])
                    continue;
                g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.22f));
                g.fillRoundedRectangle(tickArea.getX() + 2.0f + static_cast<float>(i) * gw,
                                       ghostY, juce::jmax(1.5f, gw - 1.0f), ghostH, 1.0f);
            }
        }

        const juce::String line = statusText.isNotEmpty() ? statusText : "Live activity";
        g.setColour(juce::Colour(CustomLookAndFeel::AETHER_CYAN).withAlpha(0.85f));
        g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
        g.drawText(line,
                   bounds.reduced(10.0f).removeFromBottom(20.0f),
                   juce::Justification::centred);
    }

    std::vector<bool> pattern;
    std::vector<bool> lastPattern;
    std::array<bool, kActivitySlots> activityTicks {};
    int currentStep = 0;
    float activityLevel = 0.0f;
    juce::String statusText;
    juce::Colour accentColor = juce::Colour(CustomLookAndFeel::AMBER_TESLA);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternVisualizer)
};
