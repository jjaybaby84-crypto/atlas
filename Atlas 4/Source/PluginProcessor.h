#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "SynthVoice.h"
#include "SynthSound.h"

/*
    The AudioProcessor is the engine of the plugin. The DAW sends it
    MIDI and asks processBlock() to produce audio.

    Signal chain:  8 synth voices -> low-pass filter -> delay -> output gain

    All knobs live in the AudioProcessorValueTreeState ("apvts") so the
    DAW can automate them and they're saved with the project.
    Knob ranges are 0..10 like on hardware; we map them to real units
    (Hz, seconds...) inside processBlock.
*/
class AtlasAudioProcessor : public juce::AudioProcessor
{
public:
    AtlasAudioProcessor();
    ~AtlasAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                  { return true; }

    const juce::String getName() const override      { return "Atlas"; }
    bool acceptsMidi() const override                { return true; }
    bool producesMidi() const override               { return false; }
    bool isMidiEffect() const override               { return false; }
    double getTailLengthSeconds() const override     { return 2.0; }   // delay tail

    int getNumPrograms() override                    { return 1; }
    int getCurrentProgram() override                 { return 0; }
    void setCurrentProgram (int) override            {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ---- shared with the editor ----
    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;        // the on-screen keyboard feeds this
    juce::AudioVisualiserComponent visualiser { 1 };   // the MONITOR display

private:
    juce::Synthesiser synth;
    static constexpr int numVoices = 8;

    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine { 96000 * 2 };
    double currentSampleRate = 44100.0;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AtlasAudioProcessor)
};
