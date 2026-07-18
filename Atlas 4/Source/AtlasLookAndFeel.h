#pragma once
#include <juce_audio_utils/juce_audio_utils.h>

/*
    The LookAndFeel controls how JUCE draws widgets.
    This one recreates the mockup style: matte black chassis,
    machined-aluminium knobs with tick marks, orange accents.
*/
namespace AtlasColours
{
    const juce::Colour chassis   { 0xff141414 };   // outer background
    const juce::Colour panel     { 0xff1d1d1f };   // section panels
    const juce::Colour panelEdge { 0xff2c2c2e };
    const juce::Colour display   { 0xff0c0c0e };   // value readouts / monitor
    const juce::Colour orange    { 0xffe8862a };
    const juce::Colour cyan      { 0xff53d8e8 };
    const juce::Colour text      { 0xffe8e8ea };
    const juce::Colour dimText   { 0xff9a9aa0 };
}

class AtlasLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AtlasLookAndFeel()
    {
        using namespace AtlasColours;
        setColour (juce::Slider::textBoxTextColourId,       text);
        setColour (juce::Slider::textBoxBackgroundColourId, display);
        setColour (juce::Slider::textBoxOutlineColourId,    panelEdge);
        setColour (juce::Label::textColourId,               dimText);
        setColour (juce::TextButton::buttonColourId,        panel);
        setColour (juce::TextButton::buttonOnColourId,      orange);
        setColour (juce::TextButton::textColourOffId,       text);
        setColour (juce::TextButton::textColourOnId,        juce::Colours::black);
        setColour (juce::TextEditor::backgroundColourId,    display);
        setColour (juce::TextEditor::textColourId,          orange);
        setColour (juce::TextEditor::outlineColourId,       panelEdge);
        setColour (juce::ComboBox::backgroundColourId,      display);
    }

    // Machined-aluminium style rotary knob with tick ring + orange pointer
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override
    {
        auto bounds = juce::Rectangle<float> ((float) x, (float) y,
                                              (float) width, (float) height).reduced (6.0f);
        const float size   = juce::jmin (bounds.getWidth(), bounds.getHeight());
        const auto  centre = bounds.getCentre();
        const float radius = size * 0.5f;
        const float angle  = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // tick marks around the knob
        g.setColour (AtlasColours::dimText.withAlpha (0.5f));
        for (int i = 0; i <= 20; ++i)
        {
            const float a = rotaryStartAngle + (i / 20.0f) * (rotaryEndAngle - rotaryStartAngle);
            const auto p1 = centre.getPointOnCircumference (radius * 0.92f, a);
            const auto p2 = centre.getPointOnCircumference (radius,         a);
            g.drawLine ({ p1, p2 }, i % 5 == 0 ? 1.6f : 0.8f);
        }

        // knob body: subtle radial "metal" gradient + dark rim
        const float knobR = radius * 0.78f;
        juce::ColourGradient metal (juce::Colour (0xff6a6a6e), centre.x - knobR * 0.4f, centre.y - knobR * 0.5f,
                                    juce::Colour (0xff2a2a2c), centre.x + knobR * 0.6f, centre.y + knobR * 0.8f, true);
        g.setGradientFill (metal);
        g.fillEllipse (centre.x - knobR, centre.y - knobR, knobR * 2, knobR * 2);
        g.setColour (juce::Colour (0xff0a0a0a));
        g.drawEllipse (centre.x - knobR, centre.y - knobR, knobR * 2, knobR * 2, 2.0f);

        // pointer
        const auto tip  = centre.getPointOnCircumference (knobR * 0.9f, angle);
        const auto root = centre.getPointOnCircumference (knobR * 0.35f, angle);
        g.setColour (AtlasColours::orange);
        g.drawLine ({ root, tip }, 3.0f);
    }
};
