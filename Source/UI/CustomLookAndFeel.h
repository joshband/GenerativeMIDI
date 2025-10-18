/*
  ==============================================================================
    CustomLookAndFeel.h

    SYNAPTIK - Gilded Steampunk Aesthetic
    Where Victorian brass meets neural fire and sacred geometry breathes steam

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // SYNAPTIK Gilded Steampunk Palette

    // FOUNDATION METALS - Structural colors
    static constexpr uint32_t ABYSS_NAVY = 0xff0A1628;       // Background - deep void
    static constexpr uint32_t BRASS_AGED = 0xffB8860B;       // Primary metal - aged brass
    static constexpr uint32_t GOLD_TEMPLE = 0xffFFD700;      // Accents - polished gold
    static constexpr uint32_t COPPER_STEAM = 0xffB87333;     // Warm highlights - copper
    static constexpr uint32_t BRONZE_GOTHIC = 0xff665D1E;    // Shadows - dark bronze
    static constexpr uint32_t STEEL_OBSIDIAN = 0xff1A2F3A;   // Dark panels

    // ENERGY COLORS - Glowing accents
    static constexpr uint32_t AETHER_CYAN = 0xff00CED1;      // Primary glow - cyan
    static constexpr uint32_t AMBER_TESLA = 0xffFFBF00;      // Warm energy - amber
    static constexpr uint32_t VIOLET_ALCHEMY = 0xff9370DB;   // Mystical - violet
    static constexpr uint32_t ROSE_STEAM = 0xffFF69B4;       // Heated metal - rose
    static constexpr uint32_t GREEN_VERDIGRIS = 0xff50C878;  // Patina - green

    CustomLookAndFeel()
    {
        // SYNAPTIK Gilded Steampunk Theme
        // Background: Abyss Navy - deep Victorian void
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(ABYSS_NAVY));

        // Brass Knob colors - ornate Victorian instrument aesthetic
        setColour(juce::Slider::thumbColourId, juce::Colour(AETHER_CYAN)); // Glowing cyan pointer
        setColour(juce::Slider::trackColourId, juce::Colour(BRONZE_GOTHIC)); // Dark brass shadows
        setColour(juce::Slider::backgroundColourId, juce::Colour(BRASS_AGED)); // Aged brass body
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(AMBER_TESLA)); // Golden energy
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(GOLD_TEMPLE)); // Polished rim

        // Text - golden brass labels
        setColour(juce::Label::textColourId, juce::Colour(GOLD_TEMPLE));

        // Buttons - mechanical switches
        setColour(juce::TextButton::buttonColourId, juce::Colour(STEEL_OBSIDIAN));
        setColour(juce::TextButton::buttonOnColourId, juce::Colour(BRASS_AGED));
        setColour(juce::TextButton::textColourOffId, juce::Colour(COPPER_STEAM));
        setColour(juce::TextButton::textColourOnId, juce::Colour(ABYSS_NAVY));

        // Combo boxes - brass plates
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(STEEL_OBSIDIAN));
        setColour(juce::ComboBox::textColourId, juce::Colour(COPPER_STEAM));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(BRASS_AGED));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                         juce::Slider& slider) override
    {
        auto radius = juce::jmin(width / 2, height / 2) - 8.0f;
        auto centreX = x + width * 0.5f;
        auto centreY = y + height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Aether glow (outer radiance)
        auto glowRadius = radius + 12.0f;
        juce::ColourGradient aetherGlow(
            juce::Colour(AETHER_CYAN).withAlpha(0.3f),
            centreX, centreY,
            juce::Colour(AMBER_TESLA).withAlpha(0.0f),
            centreX, centreY - glowRadius,
            true);
        g.setGradientFill(aetherGlow);
        g.fillEllipse(centreX - glowRadius, centreY - glowRadius, glowRadius * 2.0f, glowRadius * 2.0f);

        // Polished gold rim (ornate bezel)
        g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
        g.drawEllipse(rx - 3, ry - 3, rw + 6, rw + 6, 3.0f);

        // Inner shadow ring (depth)
        g.setColour(juce::Colour(BRONZE_GOTHIC).withAlpha(0.6f));
        g.drawEllipse(rx - 1, ry - 1, rw + 2, rw + 2, 2.0f);

        // Aged brass body with metallic gradient
        juce::ColourGradient brassGrad(
            juce::Colour(BRASS_AGED).brighter(0.2f), centreX - radius * 0.5f, ry,
            juce::Colour(BRASS_AGED).darker(0.3f), centreX + radius * 0.5f, ry + rw,
            false);
        g.setGradientFill(brassGrad);
        g.fillEllipse(rx, ry, rw, rw);

        // Verdigris patina accent (aged copper detail)
        g.setColour(juce::Colour(GREEN_VERDIGRIS).withAlpha(0.15f));
        g.fillEllipse(rx + rw * 0.6f, ry + rw * 0.1f, rw * 0.3f, rw * 0.3f);

        // Engraved tick marks (12 positions - Victorian clock aesthetic)
        g.setColour(juce::Colour(BRONZE_GOTHIC));
        for (int i = 0; i < 12; ++i)
        {
            float tickAngle = i * (juce::MathConstants<float>::twoPi / 12.0f) - juce::MathConstants<float>::halfPi;
            float x1 = centreX + radius * 0.75f * std::cos(tickAngle);
            float y1 = centreY + radius * 0.75f * std::sin(tickAngle);
            float x2 = centreX + radius * 0.88f * std::cos(tickAngle);
            float y2 = centreY + radius * 0.88f * std::sin(tickAngle);
            g.drawLine(x1, y1, x2, y2, 2.0f);
        }

        // Golden energy arc (value indicator with glow)
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, radius * 0.82f, radius * 0.82f,
                              0.0f, rotaryStartAngle, angle, true);

        // Arc glow
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId).withAlpha(0.4f));
        g.strokePath(valueArc, juce::PathStrokeType(5.0f));

        // Arc core
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(valueArc, juce::PathStrokeType(2.5f));

        // Aether crystal center hub
        auto hubRadius = radius * 0.22f;

        // Hub glow
        juce::ColourGradient hubGlow(
            juce::Colour(AETHER_CYAN).withAlpha(0.6f), centreX, centreY,
            juce::Colour(AETHER_CYAN).withAlpha(0.0f), centreX, centreY - hubRadius * 2.5f,
            true);
        g.setGradientFill(hubGlow);
        g.fillEllipse(centreX - hubRadius * 1.8f, centreY - hubRadius * 1.8f,
                     hubRadius * 3.6f, hubRadius * 3.6f);

        // Hub brass ring
        g.setColour(juce::Colour(GOLD_TEMPLE));
        g.drawEllipse(centreX - hubRadius, centreY - hubRadius,
                     hubRadius * 2.0f, hubRadius * 2.0f, 2.0f);

        // Hub copper fill
        juce::ColourGradient hubGrad(
            juce::Colour(COPPER_STEAM).brighter(0.1f), centreX, centreY - hubRadius,
            juce::Colour(COPPER_STEAM).darker(0.2f), centreX, centreY + hubRadius,
            false);
        g.setGradientFill(hubGrad);
        g.fillEllipse(centreX - hubRadius * 0.9f, centreY - hubRadius * 0.9f,
                     hubRadius * 1.8f, hubRadius * 1.8f);

        // Crystal highlight
        g.setColour(juce::Colour(AETHER_CYAN).withAlpha(0.7f));
        g.fillEllipse(centreX - hubRadius * 0.4f, centreY - hubRadius * 0.5f,
                     hubRadius * 0.8f, hubRadius * 0.8f);

        // Glowing aether pointer (energy beam)
        juce::Path pointer;
        auto pointerLength = radius * 0.68f;
        auto pointerThickness = 4.0f;

        // Pointer glow trail (wider)
        pointer.addRoundedRectangle(-pointerThickness * 0.8f, -radius + hubRadius,
                                   pointerThickness * 1.6f, pointerLength,
                                   pointerThickness * 0.4f);
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(slider.findColour(juce::Slider::thumbColourId).withAlpha(0.5f));
        g.fillPath(pointer);

        // Pointer core beam
        juce::Path pointerCore;
        pointerCore.addRoundedRectangle(-pointerThickness * 0.35f, -radius + hubRadius,
                                       pointerThickness * 0.7f, pointerLength * 0.95f,
                                       pointerThickness * 0.35f);
        pointerCore.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.fillPath(pointerCore);

        // Pointer tip highlight (bright aether spark)
        auto tipX = centreX + std::sin(angle) * (radius - hubRadius * 1.2f);
        auto tipY = centreY - std::cos(angle) * (radius - hubRadius * 1.2f);
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.fillEllipse(tipX - pointerThickness * 0.5f, tipY - pointerThickness * 0.5f,
                     pointerThickness, pointerThickness);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float maxSliderPos,
                         const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto trackWidth = juce::jmin(10.0f, slider.isHorizontal() ? height * 0.3f : width * 0.3f);

        juce::Rectangle<float> track;
        if (slider.isHorizontal())
            track = { static_cast<float>(x), y + (height - trackWidth) * 0.5f,
                     static_cast<float>(width), trackWidth };
        else
            track = { x + (width - trackWidth) * 0.5f, static_cast<float>(y),
                     trackWidth, static_cast<float>(height) };

        // Brass channel background
        juce::ColourGradient channelGrad(
            juce::Colour(BRONZE_GOTHIC), track.getCentreX() - trackWidth * 0.5f, track.getCentreY(),
            juce::Colour(STEEL_OBSIDIAN), track.getCentreX() + trackWidth * 0.5f, track.getCentreY(),
            false);
        g.setGradientFill(channelGrad);
        g.fillRoundedRectangle(track, trackWidth * 0.5f);

        // Brass outline
        g.setColour(juce::Colour(BRASS_AGED).withAlpha(0.8f));
        g.drawRoundedRectangle(track, trackWidth * 0.5f, 1.5f);

        // Amber energy fill (from min to current value)
        juce::Rectangle<float> valueFill;
        if (slider.isHorizontal())
            valueFill = { track.getX(), track.getY(), sliderPos - x, track.getHeight() };
        else
            valueFill = { track.getX(), sliderPos, track.getWidth(), track.getBottom() - sliderPos };

        // Gradient amber/cyan energy flow
        juce::ColourGradient energyGrad(
            juce::Colour(AMBER_TESLA).withAlpha(0.4f),
            slider.isHorizontal() ? valueFill.getX() : valueFill.getCentreX(),
            slider.isHorizontal() ? valueFill.getCentreY() : valueFill.getBottom(),
            juce::Colour(AETHER_CYAN).withAlpha(0.8f),
            slider.isHorizontal() ? valueFill.getRight() : valueFill.getCentreX(),
            slider.isHorizontal() ? valueFill.getCentreY() : valueFill.getY(),
            false);
        g.setGradientFill(energyGrad);
        g.fillRoundedRectangle(valueFill, trackWidth * 0.5f);

        // Copper thumb (diamond/gem shape)
        float thumbSize = 16.0f;
        juce::Point<float> thumbPos;
        if (slider.isHorizontal())
            thumbPos = { sliderPos, y + height * 0.5f };
        else
            thumbPos = { x + width * 0.5f, sliderPos };

        // Thumb aether glow
        juce::ColourGradient thumbGlow(
            slider.findColour(juce::Slider::thumbColourId).withAlpha(0.6f),
            thumbPos.x, thumbPos.y,
            slider.findColour(juce::Slider::thumbColourId).withAlpha(0.0f),
            thumbPos.x, thumbPos.y - thumbSize * 1.5f,
            true);
        g.setGradientFill(thumbGlow);
        g.fillEllipse(thumbPos.x - thumbSize * 1.3f, thumbPos.y - thumbSize * 1.3f,
                     thumbSize * 2.6f, thumbSize * 2.6f);

        // Brass diamond bezel
        juce::Path diamond;
        diamond.addTriangle(thumbPos.x, thumbPos.y - thumbSize * 0.75f,  // top
                           thumbPos.x - thumbSize * 0.75f, thumbPos.y,    // left
                           thumbPos.x, thumbPos.y + thumbSize * 0.75f);   // bottom
        diamond.addTriangle(thumbPos.x, thumbPos.y - thumbSize * 0.75f,  // top
                           thumbPos.x + thumbSize * 0.75f, thumbPos.y,    // right
                           thumbPos.x, thumbPos.y + thumbSize * 0.75f);   // bottom

        g.setColour(juce::Colour(GOLD_TEMPLE));
        g.fillPath(diamond);

        // Copper inner gem
        juce::Path innerGem;
        innerGem.addTriangle(thumbPos.x, thumbPos.y - thumbSize * 0.55f,  // top
                            thumbPos.x - thumbSize * 0.55f, thumbPos.y,    // left
                            thumbPos.x, thumbPos.y + thumbSize * 0.55f);   // bottom
        innerGem.addTriangle(thumbPos.x, thumbPos.y - thumbSize * 0.55f,  // top
                            thumbPos.x + thumbSize * 0.55f, thumbPos.y,    // right
                            thumbPos.x, thumbPos.y + thumbSize * 0.55f);   // bottom

        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.fillPath(innerGem);

        // Aether crystal highlight
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.fillEllipse(thumbPos.x - thumbSize * 0.25f, thumbPos.y - thumbSize * 0.35f,
                     thumbSize * 0.5f, thumbSize * 0.5f);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);

        auto baseColour = backgroundColour;
        if (shouldDrawButtonAsDown)
            baseColour = baseColour.brighter(0.4f);
        else if (shouldDrawButtonAsHighlighted)
            baseColour = baseColour.brighter(0.15f);

        // Aether glow when active
        if (button.getToggleState() || shouldDrawButtonAsHighlighted)
        {
            juce::ColourGradient glowGrad(
                juce::Colour(AETHER_CYAN).withAlpha(0.4f),
                bounds.getCentreX(), bounds.getCentreY(),
                juce::Colour(AMBER_TESLA).withAlpha(0.0f),
                bounds.getCentreX(), bounds.getCentreY() - bounds.getHeight(),
                true);
            g.setGradientFill(glowGrad);
            g.fillRoundedRectangle(bounds.expanded(5.0f), 8.0f);
        }

        // Brass body gradient
        juce::ColourGradient buttonGrad(baseColour.brighter(0.15f),
                                        bounds.getX(), bounds.getCentreY(),
                                        baseColour.darker(0.25f),
                                        bounds.getRight(), bounds.getCentreY(), false);
        g.setGradientFill(buttonGrad);
        g.fillRoundedRectangle(bounds, 6.0f);

        // Brass rim
        g.setColour(juce::Colour(BRASS_AGED).withAlpha(button.getToggleState() ? 0.9f : 0.5f));
        g.drawRoundedRectangle(bounds, 6.0f, 2.0f);

        // Inner shadow (mechanical depth)
        g.setColour(juce::Colour(BRONZE_GOTHIC).withAlpha(0.4f));
        g.drawRoundedRectangle(bounds.reduced(2.0f), 4.0f, 1.0f);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool,
                     int, int, int, int, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();

        // Brass plate background gradient
        juce::ColourGradient plateGrad(
            box.findColour(juce::ComboBox::backgroundColourId).brighter(0.1f),
            bounds.getCentreX(), bounds.getY(),
            box.findColour(juce::ComboBox::backgroundColourId).darker(0.2f),
            bounds.getCentreX(), bounds.getBottom(), false);
        g.setGradientFill(plateGrad);
        g.fillRoundedRectangle(bounds.reduced(1.0f), 4.0f);

        // Brass bezel
        g.setColour(box.findColour(juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle(bounds, 4.0f, 2.0f);

        // Inner shadow
        g.setColour(juce::Colour(BRONZE_GOTHIC).withAlpha(0.4f));
        g.drawRoundedRectangle(bounds.reduced(2.5f), 3.0f, 1.0f);

        // Brass chevron
        auto arrowZone = bounds.removeFromRight(24.0f).reduced(6.0f);
        juce::Path chevron;
        chevron.startNewSubPath(arrowZone.getX(), arrowZone.getY() + 3);
        chevron.lineTo(arrowZone.getCentreX(), arrowZone.getBottom() - 3);
        chevron.lineTo(arrowZone.getRight(), arrowZone.getY() + 3);

        g.setColour(box.findColour(juce::ComboBox::textColourId));
        g.strokePath(chevron, juce::PathStrokeType(2.0f));
    }

    juce::Font getLabelFont(juce::Label& label) override
    {
        // Stencil-style bold font for vintage pedal look
        return juce::Font(label.getHeight() * 0.7f, juce::Font::bold);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLookAndFeel)
};
