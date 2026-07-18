#include "PluginEditor.h"

using namespace AtlasColours;

AtlasAudioProcessorEditor::AtlasAudioProcessorEditor (AtlasAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lookAndFeel);

    // ---------- top bar: preset browser ----------
    addAndMakeVisible (prevPreset);
    addAndMakeVisible (nextPreset);
    addAndMakeVisible (saveButton);
    addAndMakeVisible (presetName);
    presetName.setJustification (juce::Justification::centred);
    presetName.setFont (juce::Font (juce::FontOptions (16.0f)));
    presetName.setText ("808-Style Bass");

    saveButton.onClick = [this] { presets.save (presetName.getText()); };
    prevPreset.onClick = [this] { presets.loadPrevious(); presetName.setText (presets.getCurrentName()); };
    nextPreset.onClick = [this] { presets.loadNext();     presetName.setText (presets.getCurrentName()); };

    // ---------- oscillator switch ----------
    for (auto* b : { &sawButton, &squareButton })
    {
        b->setClickingTogglesState (false);
        addAndMakeVisible (*b);
    }
    sawButton.onClick    = [this] { oscAttachment->setValueAsCompleteGesture (0.0f); };
    squareButton.onClick = [this] { oscAttachment->setValueAsCompleteGesture (1.0f); };

    // keeps the buttons in sync with the parameter (also when the DAW automates it)
    oscAttachment = std::make_unique<juce::ParameterAttachment> (
        *processor.apvts.getParameter ("osc"),
        [this] (float v) { updateOscButtons ((int) v); });
    oscAttachment->sendInitialUpdate();

    // ---------- knobs ----------
    for (auto* s : { &envelopeKnob, &cutoffKnob, &resoKnob, &delayKnob, &outputKnob })
        setupKnob (*s);

    envelopeAtt = std::make_unique<SliderAttachment> (processor.apvts, "envelope", envelopeKnob);
    cutoffAtt   = std::make_unique<SliderAttachment> (processor.apvts, "cutoff",   cutoffKnob);
    resoAtt     = std::make_unique<SliderAttachment> (processor.apvts, "reso",     resoKnob);
    outputAtt   = std::make_unique<SliderAttachment> (processor.apvts, "output",   outputKnob);

    // delay knob edits time / feedback / mix depending on the selected button
    for (auto* b : { &delayTimeBtn, &delayFbBtn, &delayMixBtn })
    {
        b->setClickingTogglesState (false);
        addAndMakeVisible (*b);
    }
    delayTimeBtn.onClick = [this] { selectDelayParam ("delaytime", delayTimeBtn); };
    delayFbBtn.onClick   = [this] { selectDelayParam ("delayfb",   delayFbBtn);   };
    delayMixBtn.onClick  = [this] { selectDelayParam ("delaymix",  delayMixBtn);  };
    selectDelayParam ("delaytime", delayTimeBtn);   // TIME selected by default

    // ---------- bypass ----------
    bypassButton.setClickingTogglesState (true);
    addAndMakeVisible (bypassButton);
    bypassAtt = std::make_unique<ButtonAttachment> (processor.apvts, "bypass", bypassButton);

    // ---------- monitor ----------
    processor.visualiser.setColours (display, cyan);
    addAndMakeVisible (processor.visualiser);

    // ---------- keyboard (C2..C6 like the mockup) ----------
    keyboard.setAvailableRange (36, 96);
    keyboard.setKeyWidth (18.0f);
    addAndMakeVisible (keyboard);

    setSize (920, 720);
}

AtlasAudioProcessorEditor::~AtlasAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void AtlasAudioProcessorEditor::setupKnob (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 20);
    s.setNumDecimalPlacesToDisplay (1);
    addAndMakeVisible (s);
}

void AtlasAudioProcessorEditor::selectDelayParam (const juce::String& paramID,
                                                  juce::TextButton& button)
{
    delayAtt.reset();   // detach the old parameter first
    delayAtt = std::make_unique<SliderAttachment> (processor.apvts, paramID, delayKnob);

    for (auto* b : { &delayTimeBtn, &delayFbBtn, &delayMixBtn })
        b->setToggleState (b == &button, juce::dontSendNotification);
}

void AtlasAudioProcessorEditor::updateOscButtons (int choice)
{
    sawButton.setToggleState    (choice == 0, juce::dontSendNotification);
    squareButton.setToggleState (choice == 1, juce::dontSendNotification);
}

// ------------------------------------------------------------------
void AtlasAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (chassis);

    // ---- branding ----
    g.setColour (text);
    g.setFont (juce::Font (juce::FontOptions (34.0f, juce::Font::bold)));
    g.drawText ("ATLAS", 24, 14, 220, 34, juce::Justification::left);
    g.setFont (juce::Font (juce::FontOptions (15.0f)));
    g.setColour (dimText);
    g.drawText ("BOOGIE  LABS", 26, 46, 220, 16, juce::Justification::left);

    g.setFont (juce::Font (juce::FontOptions (11.0f)));
    g.drawText ("PRESET", getWidth() - 470, 10, 60, 14, juce::Justification::left);

    // BTL power light
    auto led = juce::Rectangle<float> ((float) getWidth() - 48.0f, 22.0f, 22.0f, 22.0f);
    g.setColour (orange);
    g.drawEllipse (led, 2.0f);
    g.fillEllipse (led.reduced (6.0f));
    g.drawText ("BTL", (int) led.getX() - 4, (int) led.getBottom() + 2, 30, 12,
                juce::Justification::centred);

    // ---- section panels ----
    auto drawPanel = [&g] (juce::Rectangle<int> r, const juce::String& title,
                           juce::Colour ledColour)
    {
        g.setColour (panel);
        g.fillRoundedRectangle (r.toFloat(), 8.0f);
        g.setColour (panelEdge);
        g.drawRoundedRectangle (r.toFloat(), 8.0f, 1.0f);

        g.setColour (ledColour);
        g.fillEllipse ((float) r.getX() + 14.0f, (float) r.getY() + 14.0f, 8.0f, 8.0f);

        g.setColour (text);
        g.setFont (juce::Font (juce::FontOptions (13.0f, juce::Font::bold)));
        g.drawText (title, r.getX() + 30, r.getY() + 8, r.getWidth() - 40, 18,
                    juce::Justification::left);
    };

    drawPanel (oscPanel,     "OSCILLATOR", orange);
    drawPanel (envPanel,     "ENVELOPE",   orange);
    drawPanel (cutPanel,     "CUTOFF",     orange);
    drawPanel (resoPanel,    "RESO",       orange);
    drawPanel (delayPanel,   "DELAY",      cyan);
    drawPanel (monitorPanel, "MONITOR",    cyan);

    // OUTPUT label above the output knob
    g.setColour (text);
    g.setFont (juce::Font (juce::FontOptions (12.0f, juce::Font::bold)));
    g.drawText ("OUTPUT", outputKnob.getX(), outputKnob.getY() - 16,
                outputKnob.getWidth(), 14, juce::Justification::centred);
}

void AtlasAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20);

    // ---- top bar ----
    auto top = area.removeFromTop (46);
    {
        auto right = top.removeFromRight (460);
        right.removeFromRight (60);                       // room for the BTL light
        prevPreset.setBounds (right.removeFromLeft (36).reduced (0, 6));
        right.removeFromLeft (6);
        presetName.setBounds (right.removeFromLeft (230).reduced (0, 6));
        right.removeFromLeft (6);
        nextPreset.setBounds (right.removeFromLeft (36).reduced (0, 6));
        right.removeFromLeft (8);
        saveButton.setBounds (right.removeFromLeft (64).reduced (0, 6));
    }
    area.removeFromTop (14);

    // ---- oscillator row ----
    oscPanel = area.removeFromTop (56);
    {
        auto row = oscPanel.withTrimmedLeft (140).withTrimmedRight (40).reduced (0, 13);
        auto centre = row.withSizeKeepingCentre (280, row.getHeight());
        sawButton.setBounds (centre.removeFromLeft (130));
        centre.removeFromLeft (20);
        squareButton.setBounds (centre);
    }
    area.removeFromTop (12);

    // ---- knob row ----
    auto knobRow = area.removeFromTop (200);
    const int gap = 12;
    const int panelW = (knobRow.getWidth() - 3 * gap) / 4;

    auto placeKnob = [] (juce::Slider& s, juce::Rectangle<int> r, int bottomSpace)
    {
        s.setBounds (r.withTrimmedTop (28).withTrimmedBottom (bottomSpace).reduced (10, 0));
    };

    envPanel = knobRow.removeFromLeft (panelW);  knobRow.removeFromLeft (gap);
    cutPanel = knobRow.removeFromLeft (panelW);  knobRow.removeFromLeft (gap);
    resoPanel = knobRow.removeFromLeft (panelW); knobRow.removeFromLeft (gap);
    delayPanel = knobRow;

    placeKnob (envelopeKnob, envPanel,  10);
    placeKnob (cutoffKnob,   cutPanel,  10);
    placeKnob (resoKnob,     resoPanel, 10);
    placeKnob (delayKnob,    delayPanel, 38);    // leave room for TIME/FB/MIX

    {
        auto btnRow = delayPanel.withTop (delayPanel.getBottom() - 32)
                                .reduced (12, 6);
        const int bw = (btnRow.getWidth() - 12) / 3;
        delayTimeBtn.setBounds (btnRow.removeFromLeft (bw));
        btnRow.removeFromLeft (6);
        delayFbBtn.setBounds (btnRow.removeFromLeft (bw));
        btnRow.removeFromLeft (6);
        delayMixBtn.setBounds (btnRow);
    }
    area.removeFromTop (12);

    // ---- monitor ----
    monitorPanel = area.removeFromTop (110);
    processor.visualiser.setBounds (monitorPanel.reduced (14).withTrimmedTop (18));
    area.removeFromTop (12);

    // ---- bypass + output ----
    auto util = area.removeFromTop (96);
    bypassButton.setBounds (util.getX(), util.getCentreY() - 15, 100, 30);
    outputKnob.setBounds (util.getRight() - 110, util.getY() + 14, 100, 82);
    area.removeFromTop (8);

    // ---- keyboard ----
    keyboard.setBounds (area);
}
