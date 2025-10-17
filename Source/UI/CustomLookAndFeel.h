/*
  ==============================================================================
    CustomLookAndFeel.h

    Modern dark theme look and feel

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        // Modern dark color scheme
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff1a1a1a));
        setColour(juce::Slider::thumbColourId, juce::Colour(0xff00d4ff));
        setColour(juce::Slider::trackColourId, juce::Colour(0xff333333));
        setColour(juce::Slider::backgroundColourId, juce::Colour(0xff2a2a2a));
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00d4ff));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff444444));
        setColour(juce::Label::textColourId, juce::Colour(0xffcccccc));
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff333333));
        setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff00d4ff));
        setColour(juce::TextButton::textColourOffId, juce::Colour(0xffcccccc));
        setColour(juce::TextButton::textColourOnId, juce::Colour(0xff000000));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a2a2a));
        setColour(juce::ComboBox::textColourId, juce::Colour(0xffcccccc));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff444444));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                         juce::Slider& slider) override
    {
        auto radius = juce::jmin(width / 2, height / 2) - 4.0f;
        auto centreX = x + width * 0.5f;
        auto centreY = y + height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Outer circle (track)
        g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
        g.drawEllipse(rx, ry, rw, rw, 2.0f);

        // Inner fill arc
        juce::Path filledArc;
        filledArc.addPieSegment(rx, ry, rw, rw, rotaryStartAngle, angle, 0.7f);
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.fillPath(filledArc);

        // Center circle
        auto centerRadius = radius * 0.3f;
        g.setColour(juce::Colour(0xff2a2a2a));
        g.fillEllipse(centreX - centerRadius, centreY - centerRadius, centerRadius * 2.0f, centerRadius * 2.0f);

        // Pointer
        juce::Path p;
        auto pointerLength = radius * 0.7f;
        auto pointerThickness = 3.0f;
        p.addRectangle(-pointerThickness * 0.5f, -radius + 10.0f, pointerThickness, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.fillPath(p);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float maxSliderPos,
                         const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (slider.isBar())
        {
            g.setColour(slider.findColour(juce::Slider::trackColourId));
            g.fillRect(slider.isHorizontal() ? juce::Rectangle<float>(static_cast<float>(x), y + 0.5f, sliderPos - x, height - 1.0f)
                                             : juce::Rectangle<float>(x + 0.5f, sliderPos, width - 1.0f, y + (height - sliderPos)));
        }
        else
        {
            auto isTwoVal = (style == juce::Slider::SliderStyle::TwoValueVertical || style == juce::Slider::SliderStyle::TwoValueHorizontal);
            auto isThreeVal = (style == juce::Slider::SliderStyle::ThreeValueVertical || style == juce::Slider::SliderStyle::ThreeValueHorizontal);

            auto trackWidth = juce::jmin(6.0f, slider.isHorizontal() ? height * 0.25f : width * 0.25f);

            juce::Point<float> startPoint(slider.isHorizontal() ? x : x + width * 0.5f,
                                         slider.isHorizontal() ? y + height * 0.5f : height + y);

            juce::Point<float> endPoint(slider.isHorizontal() ? width + x : startPoint.x,
                                       slider.isHorizontal() ? startPoint.y : y);

            juce::Path backgroundTrack;
            backgroundTrack.startNewSubPath(startPoint);
            backgroundTrack.lineTo(endPoint);
            g.setColour(slider.findColour(juce::Slider::backgroundColourId));
            g.strokePath(backgroundTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });

            juce::Path valueTrack;
            juce::Point<float> minPoint, maxPoint, thumbPoint;

            if (isTwoVal || isThreeVal)
            {
                minPoint = { slider.isHorizontal() ? minSliderPos : width * 0.5f,
                            slider.isHorizontal() ? height * 0.5f : minSliderPos };

                if (isThreeVal)
                    thumbPoint = { slider.isHorizontal() ? sliderPos : width * 0.5f,
                                  slider.isHorizontal() ? height * 0.5f : sliderPos };

                maxPoint = { slider.isHorizontal() ? maxSliderPos : width * 0.5f,
                            slider.isHorizontal() ? height * 0.5f : maxSliderPos };
            }
            else
            {
                auto kx = slider.isHorizontal() ? sliderPos : (x + width * 0.5f);
                auto ky = slider.isHorizontal() ? (y + height * 0.5f) : sliderPos;

                minPoint = startPoint;
                maxPoint = { kx, ky };
            }

            auto thumbWidth = getSliderThumbRadius(slider);

            valueTrack.startNewSubPath(minPoint);
            valueTrack.lineTo(isThreeVal ? thumbPoint : maxPoint);
            g.setColour(slider.findColour(juce::Slider::trackColourId));
            g.strokePath(valueTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });

            if (!isTwoVal)
            {
                g.setColour(slider.findColour(juce::Slider::thumbColourId));
                g.fillEllipse(juce::Rectangle<float>(static_cast<float>(thumbWidth), static_cast<float>(thumbWidth)).withCentre(isThreeVal ? thumbPoint : maxPoint));
            }

            if (isTwoVal || isThreeVal)
            {
                auto sr = juce::jmin(trackWidth, (slider.isHorizontal() ? height : width) * 0.4f);
                auto pointerColour = slider.findColour(juce::Slider::thumbColourId);

                if (slider.isHorizontal())
                {
                    drawPointer(g, minSliderPos - sr,
                               juce::jmax(0.0f, y + height * 0.5f - trackWidth * 2.0f),
                               trackWidth * 2.0f, pointerColour, 2);

                    drawPointer(g, maxSliderPos - trackWidth,
                               juce::jmin(y + height - trackWidth * 2.0f, y + height * 0.5f),
                               trackWidth * 2.0f, pointerColour, 4);
                }
                else
                {
                    drawPointer(g, juce::jmax(0.0f, x + width * 0.5f - trackWidth * 2.0f),
                               minSliderPos - trackWidth,
                               trackWidth * 2.0f, pointerColour, 1);

                    drawPointer(g, juce::jmin(x + width - trackWidth * 2.0f, x + width * 0.5f), maxSliderPos - sr,
                               trackWidth * 2.0f, pointerColour, 3);
                }
            }
        }
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto cornerSize = 6.0f;
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

        auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
                                          .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

        if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
            baseColour = baseColour.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);

        g.setColour(baseColour);

        if (button.isConnectedOnLeft() || button.isConnectedOnRight())
        {
            juce::Path path;
            path.addRoundedRectangle(bounds.getX(), bounds.getY(),
                                    bounds.getWidth(), bounds.getHeight(),
                                    cornerSize, cornerSize,
                                    !button.isConnectedOnLeft(),
                                    !button.isConnectedOnRight(),
                                    !button.isConnectedOnLeft(),
                                    !button.isConnectedOnRight());

            g.fillPath(path);

            g.setColour(button.findColour(juce::ComboBox::outlineColourId));
            g.strokePath(path, juce::PathStrokeType(1.0f));
        }
        else
        {
            g.fillRoundedRectangle(bounds, cornerSize);

            g.setColour(button.findColour(juce::ComboBox::outlineColourId));
            g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
        }
    }

private:
    void drawPointer(juce::Graphics& g, float x, float y, float diameter, const juce::Colour& colour, int direction)
    {
        juce::Path p;
        p.startNewSubPath(x + diameter * 0.5f, y);
        p.lineTo(x + diameter, y + diameter * 0.6f);
        p.lineTo(x + diameter, y + diameter);
        p.lineTo(x, y + diameter);
        p.lineTo(x, y + diameter * 0.6f);
        p.closeSubPath();

        p.applyTransform(juce::AffineTransform::rotation(direction * (juce::MathConstants<float>::pi * 0.5f), x + diameter * 0.5f, y + diameter * 0.5f));

        g.setColour(colour);
        g.fillPath(p);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLookAndFeel)
};
