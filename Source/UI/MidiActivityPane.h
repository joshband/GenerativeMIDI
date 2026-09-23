/*
  ==============================================================================
    MidiActivityPane.h

    Collapsible MIDI activity / last-events strip (brass + cyan-on-navy).

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../DSP/MidiActivityLog.h"
#include "CustomLookAndFeel.h"
#include <deque>

class MidiActivityPane : public juce::Component
{
public:
    static constexpr int kCollapsedHeight = 28;
    static constexpr int kExpandedExtra = 100;
    static constexpr int kMaxVisible = 24;

    MidiActivityPane()
    {
        setTitle ("MIDI Activity Log");

        addAndMakeVisible (toggleButton);
        toggleButton.setButtonText ("MIDI Log v");
        toggleButton.setName ("MIDI Log");
        toggleButton.setClickingTogglesState (true);
        toggleButton.setToggleState (true, juce::dontSendNotification);
        toggleButton.setTitle ("Toggle MIDI Activity Log");
        toggleButton.setAccessible (true);
        toggleButton.onClick = [this]
        {
            expanded = toggleButton.getToggleState();
            toggleButton.setButtonText (expanded ? "MIDI Log v" : "MIDI Log >");
            if (onExpandedChanged)
                onExpandedChanged();
            resized();
            repaint();
        };

        addAndMakeVisible (clearButton);
        clearButton.setButtonText ("Clear");
        clearButton.setName ("Clear MIDI Log");
        clearButton.setTitle ("Clear MIDI Activity Log");
        clearButton.setAccessible (true);
        clearButton.onClick = [this]
        {
            lines.clear();
            repaint();
        };

        addAndMakeVisible (hintLabel);
        hintLabel.setText ("Recent note-ons / offs", juce::dontSendNotification);
        hintLabel.setFont (juce::FontOptions (11.0f));
        hintLabel.setColour (juce::Label::textColourId,
                             juce::Colour (CustomLookAndFeel::COPPER_STEAM));
        hintLabel.setJustificationType (juce::Justification::centredLeft);
        hintLabel.setTitle ("MIDI Activity Hint");
    }

    void setExpanded (bool shouldExpand)
    {
        if (expanded == shouldExpand)
            return;
        expanded = shouldExpand;
        toggleButton.setToggleState (expanded, juce::dontSendNotification);
        toggleButton.setButtonText (expanded ? "MIDI Log v" : "MIDI Log >");
        if (onExpandedChanged)
            onExpandedChanged();
        resized();
        repaint();
    }

    bool isExpanded() const noexcept { return expanded; }

    int getPreferredHeight() const noexcept
    {
        return expanded ? (kCollapsedHeight + kExpandedExtra) : kCollapsedHeight;
    }

    std::function<void()> onExpandedChanged;

    /** Append drained FIFO events (newest last in fifo → show newest at top). */
    void ingest (const MidiActivityEvent* events, int count)
    {
        if (events == nullptr || count <= 0)
            return;

        for (int i = 0; i < count; ++i)
            lines.push_front (MidiActivityLog::formatEvent (events[i]));

        while (static_cast<int> (lines.size()) > kMaxVisible)
            lines.pop_back();

        repaint();
    }

    void clearLines()
    {
        lines.clear();
        repaint();
    }

    int getLineCount() const noexcept { return static_cast<int> (lines.size()); }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        g.setColour (juce::Colour (CustomLookAndFeel::STEEL_OBSIDIAN));
        g.fillRoundedRectangle (bounds, 6.0f);
        g.setColour (juce::Colour (CustomLookAndFeel::BRASS_AGED).withAlpha (0.85f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), 6.0f, 1.25f);

        if (! expanded)
            return;

        auto logArea = getLocalBounds()
                           .withTrimmedTop (kCollapsedHeight)
                           .reduced (10, 4)
                           .toFloat();

        g.setColour (juce::Colour (CustomLookAndFeel::ABYSS_NAVY).withAlpha (0.65f));
        g.fillRoundedRectangle (logArea, 4.0f);

        g.setFont (juce::FontOptions (11.0f).withStyle ("Regular"));
        g.setColour (juce::Colour (CustomLookAndFeel::AETHER_CYAN));

        float y = logArea.getY() + 2.0f;
        const float lineH = 14.0f;
        int drawn = 0;
        for (const auto& line : lines)
        {
            if (y + lineH > logArea.getBottom())
                break;
            g.drawText (line,
                        juce::Rectangle<float> (logArea.getX() + 6.0f, y,
                                               logArea.getWidth() - 12.0f, lineH),
                        juce::Justification::centredLeft, false);
            y += lineH;
            ++drawn;
        }

        if (drawn == 0)
        {
            g.setColour (juce::Colour (CustomLookAndFeel::COPPER_STEAM).withAlpha (0.7f));
            g.drawText ("Waiting for note activity…",
                        logArea.reduced (8.0f),
                        juce::Justification::centredLeft, false);
        }
    }

    void resized() override
    {
        auto header = getLocalBounds().removeFromTop (kCollapsedHeight).reduced (6, 3);
        toggleButton.setBounds (header.removeFromLeft (100));
        header.removeFromLeft (8);
        clearButton.setBounds (header.removeFromLeft (64));
        header.removeFromLeft (10);
        hintLabel.setBounds (header);

        clearButton.setVisible (expanded);
        hintLabel.setVisible (expanded);
    }

private:
    juce::TextButton toggleButton;
    juce::TextButton clearButton;
    juce::Label hintLabel;
    std::deque<juce::String> lines;
    bool expanded = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiActivityPane)
};
