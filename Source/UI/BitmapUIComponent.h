/*
  ==============================================================================
    BitmapUIComponent.h

    Bitmap-based UI component system for Victorian steampunk aesthetic
    Uses cutout PNG/JPEG images instead of procedural drawing

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Base class for UI components built entirely from bitmap images
 * Supports layered composition, alpha blending, and proper cutouts
 */
class BitmapUIComponent : public juce::Component
{
public:
    BitmapUIComponent() = default;

    /**
     * Set the background layer image
     * This should be the base panel/background texture
     */
    void setBackgroundImage(const juce::Image& img)
    {
        backgroundImage = img;
        repaint();
    }

    /**
     * Set the foreground layer image (e.g., frame, bezel)
     * Drawn on top of background with alpha blending
     */
    void setForegroundImage(const juce::Image& img)
    {
        foregroundImage = img;
        repaint();
    }

    /**
     * Set overlay decoration (e.g., corner ornaments, labels)
     */
    void setOverlayImage(const juce::Image& img, juce::Point<int> position)
    {
        overlayImages.add({img, position});
        repaint();
    }

    /**
     * Clear all overlay images
     */
    void clearOverlays()
    {
        overlayImages.clear();
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();

        // Layer 1: Background panel texture
        if (backgroundImage.isValid())
        {
            g.setOpacity(backgroundOpacity);
            g.drawImage(backgroundImage, bounds.toFloat(),
                       juce::RectanglePlacement::fillDestination);
            g.setOpacity(1.0f);
        }

        // Layer 2: Foreground frame/bezel
        if (foregroundImage.isValid())
        {
            g.setOpacity(foregroundOpacity);
            g.drawImage(foregroundImage, bounds.toFloat(),
                       juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
            g.setOpacity(1.0f);
        }

        // Layer 3: Overlays (decorations, labels, etc.)
        for (const auto& overlay : overlayImages)
        {
            if (overlay.image.isValid())
            {
                g.setOpacity(overlayOpacity);
                g.drawImageAt(overlay.image, overlay.position.x, overlay.position.y);
                g.setOpacity(1.0f);
            }
        }
    }

    // Opacity controls for layering
    void setBackgroundOpacity(float opacity) { backgroundOpacity = opacity; repaint(); }
    void setForegroundOpacity(float opacity) { foregroundOpacity = opacity; repaint(); }
    void setOverlayOpacity(float opacity) { overlayOpacity = opacity; repaint(); }

protected:
    struct OverlayImage
    {
        juce::Image image;
        juce::Point<int> position;
    };

    juce::Image backgroundImage;
    juce::Image foregroundImage;
    juce::Array<OverlayImage> overlayImages;

    float backgroundOpacity = 1.0f;
    float foregroundOpacity = 1.0f;
    float overlayOpacity = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitmapUIComponent)
};

/**
 * Bitmap-based panel section (like GENERATOR, EXPRESSION, etc.)
 * Composed entirely of cutout PNG images
 */
class BitmapPanel : public BitmapUIComponent
{
public:
    BitmapPanel(const juce::String& title) : panelTitle(title) {}

    void setTitlePlateImage(const juce::Image& img)
    {
        titlePlateImage = img;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        // Draw base layers
        BitmapUIComponent::paint(g);

        // Draw title plate if available
        if (titlePlateImage.isValid())
        {
            auto titleBounds = getLocalBounds().removeFromTop(40).reduced(20, 5);
            g.drawImage(titlePlateImage, titleBounds.toFloat(),
                       juce::RectanglePlacement::centred);
        }

        // Draw title text (golden engraved style)
        if (panelTitle.isNotEmpty())
        {
            auto titleArea = getLocalBounds().removeFromTop(40);
            g.setColour(juce::Colour(0xffFFD700)); // Gold
            g.setFont(juce::Font(14.0f, juce::Font::bold));
            g.drawText(panelTitle, titleArea, juce::Justification::centred);
        }
    }

private:
    juce::String panelTitle;
    juce::Image titlePlateImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitmapPanel)
};

