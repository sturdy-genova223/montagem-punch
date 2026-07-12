#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Same Montagem brand palette as the Finisher/Widener plugins: dark
// background, red = "soft", green = "punchy". The knob arc interpolates
// between them as the single "amount" parameter increases, so the color
// itself communicates the effect strength instead of a plain gray dial.
class PunchLookAndFeel : public juce::LookAndFeel_V4
{
public:
    inline static const juce::Colour bg        { 0xff0d0d12 };
    inline static const juce::Colour bgLighter  { 0xff1c1c24 };
    inline static const juce::Colour red        { 0xffff3b30 };
    inline static const juce::Colour green      { 0xff30d158 };
    inline static const juce::Colour textDim    { 0xffa0a0aa };

    PunchLookAndFeel()
    {
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override
    {
        auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height).reduced(10.0f);
        const auto centre = bounds.getCentre();
        const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        const auto knobColour = red.interpolatedWith(green, sliderPosProportional);

        // outer track
        {
            juce::Path track;
            track.addCentredArc(centre.x, centre.y, radius - 4.0f, radius - 4.0f,
                                 0.0f, rotaryStartAngle, rotaryEndAngle, true);
            g.setColour(bgLighter);
            g.strokePath(track, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // filled value arc
        {
            juce::Path valueArc;
            valueArc.addCentredArc(centre.x, centre.y, radius - 4.0f, radius - 4.0f,
                                    0.0f, rotaryStartAngle, angle, true);
            g.setColour(knobColour);
            g.strokePath(valueArc, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // subtle glow behind the dial face, tinted by the current colour
        g.setColour(knobColour.withAlpha(0.10f));
        g.fillEllipse(bounds.reduced(radius * 0.18f));

        // dial face
        auto face = bounds.reduced(radius * 0.34f);
        g.setColour(bgLighter);
        g.fillEllipse(face);
        g.setColour(knobColour.withAlpha(0.6f));
        g.drawEllipse(face, 1.5f);

        // pointer
        {
            juce::Path pointer;
            const float pointerLength = face.getWidth() * 0.38f;
            const float pointerThickness = 3.5f;
            pointer.addRoundedRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength, pointerThickness * 0.5f);
            pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centre));
            g.setColour(juce::Colours::white);
            g.fillPath(pointer);
        }
    }

    juce::Font getLabelFont(juce::Label&) override
    {
        return juce::Font(juce::FontOptions(14.0f, juce::Font::bold));
    }
};
