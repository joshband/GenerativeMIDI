/*
  ==============================================================================
    PatternVisualizer.h

    Visual display of rhythm patterns

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class PatternVisualizer : public juce::Component, private juce::Timer
{
public:
    PatternVisualizer()
    {
        startTimerHz(30); // 30 FPS refresh
    }

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

        // Background
        g.setColour(juce::Colour(0xff0a0a0a));
        g.fillRoundedRectangle(bounds, 8.0f);

        // Border
        g.setColour(juce::Colour(0xff333333));
        g.drawRoundedRectangle(bounds.reduced(1.0f), 8.0f, 2.0f);

        if (pattern.empty())
            return;

        // Draw pattern
        float stepWidth = bounds.getWidth() / pattern.size();
        float padding = 4.0f;

        for (size_t i = 0; i < pattern.size(); ++i)
        {
            float x = bounds.getX() + i * stepWidth + padding;
            float y = bounds.getY() + padding;
            float w = stepWidth - padding * 2.0f;
            float h = bounds.getHeight() - padding * 2.0f;

            bool isCurrent = (static_cast<int>(i) == currentStep);
            bool isActive = pattern[i];

            if (isActive)
            {
                // Active step
                if (isCurrent)
                {
                    // Current active step - bright cyan
                    g.setColour(juce::Colour(0xff00ffff));
                    g.fillRoundedRectangle(x, y, w, h, 3.0f);
                }
                else
                {
                    // Non-current active step - cyan
                    g.setColour(juce::Colour(0xff00d4ff));
                    g.fillRoundedRectangle(x, y, w, h, 3.0f);
                }
            }
            else
            {
                // Inactive step
                if (isCurrent)
                {
                    // Current inactive - dim highlight
                    g.setColour(juce::Colour(0xff444444));
                    g.fillRoundedRectangle(x, y, w, h, 3.0f);
                }
                else
                {
                    // Non-current inactive - very dim
                    g.setColour(juce::Colour(0xff222222));
                    g.drawRoundedRectangle(x, y, w, h, 3.0f, 1.0f);
                }
            }
        }

        // Draw step numbers on 4s
        g.setColour(juce::Colour(0xff666666));
        g.setFont(10.0f);
        for (size_t i = 0; i < pattern.size(); ++i)
        {
            if (i % 4 == 0)
            {
                float x = bounds.getX() + i * stepWidth;
                juce::String label = juce::String(static_cast<int>(i));
                g.drawText(label, x, bounds.getBottom() - 15.0f, stepWidth, 12.0f,
                          juce::Justification::centred);
            }
        }
    }

private:
    void timerCallback() override
    {
        repaint();
    }

    std::vector<bool> pattern;
    int currentStep = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternVisualizer)
};
