#pragma once
#include "PluginProcessor.h"
#include "AtlasLookAndFeel.h"
#include "PresetManager.h"

/*
    The Atlas window, laid out like the mockup:

      ATLAS / BOOGIE LABS          [<] [preset name] [>] [Save]  (BTL)
      OSCILLATOR      SAW  <switch>  SQUARE
      [ENVELOPE] [CUTOFF] [RESO] [DELAY + time/fb/mix buttons]
      MONITOR  ~~~~~waveform~~~~~
      [Bypass]                                   OUTPUT knob
      |||| virtual keyboard ||||
*/
class AtlasAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit AtlasAudioProcessorEditor (AtlasAudioProcessor&);
    ~AtlasAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AtlasAudioProcessor& processor;
    AtlasLookAndFeel lookAndFeel;
    PresetManager presets { processor.apvts };

    // --- top bar ---
    juce::TextButton prevPreset { "<" }, nextPreset { ">" }, saveButton { "Save" };
    juce::TextEditor presetName;

    // --- oscillator switch ---
    juce::TextButton sawButton { "SAW" }, squareButton { "SQUARE" };
    std::unique_ptr<juce::ParameterAttachment> oscAttachment;

    // --- knobs ---
    juce::Slider envelopeKnob, cutoffKnob, resoKnob, delayKnob, outputKnob;

    // delay mode buttons: the DELAY knob edits whichever is selected
    juce::TextButton delayTimeBtn { "TIME" }, delayFbBtn { "FB" }, delayMixBtn { "MIX" };

    juce::TextButton bypassButton { "Bypass" };

    // --- keyboard ---
    juce::MidiKeyboardComponent keyboard { processor.keyboardState,
                                           juce::MidiKeyboardComponent::horizontalKeyboard };

    // attachments keep widgets and parameters in sync
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> envelopeAtt, cutoffAtt, resoAtt, delayAtt, outputAtt;
    std::unique_ptr<ButtonAttachment> bypassAtt;

    // panel rectangles, computed in resized(), drawn in paint()
    juce::Rectangle<int> oscPanel, envPanel, cutPanel, resoPanel, delayPanel, monitorPanel;

    void setupKnob (juce::Slider&);
    void selectDelayParam (const juce::String& paramID, juce::TextButton& button);
    void updateOscButtons (int choice);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AtlasAudioProcessorEditor)
};
