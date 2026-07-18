#include "PluginProcessor.h"
#include "PluginEditor.h"

AtlasAudioProcessor::AtlasAudioProcessor()
    : AudioProcessor (BusesProperties()
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameters())
{
    for (int i = 0; i < numVoices; ++i)
        synth.addVoice (new SynthVoice());

    synth.addSound (new SynthSound());

    visualiser.setBufferSize (512);
    visualiser.setSamplesPerBlock (64);
}

/*
    All knobs use a hardware-style 0..10 range (like the mockup's
    3.1 / 3.8 / 2.9 readouts). Mapping to real units happens later.
*/
juce::AudioProcessorValueTreeState::ParameterLayout
AtlasAudioProcessor::createParameters()
{
    using P = juce::AudioParameterFloat;
    auto knob = [] (const char* id, const char* name, float def)
    {
        return std::make_unique<P> (id, name,
                                    juce::NormalisableRange<float> (0.0f, 10.0f, 0.1f), def);
    };

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        "osc", "Oscillator", juce::StringArray { "Saw", "Square" }, 0));

    layout.add (knob ("envelope", "Envelope",   3.1f));
    layout.add (knob ("cutoff",   "Cutoff",     3.8f));
    layout.add (knob ("reso",     "Resonance",  2.9f));
    layout.add (knob ("delaytime","Delay Time", 5.0f));
    layout.add (knob ("delayfb",  "Delay FB",   3.0f));
    layout.add (knob ("delaymix", "Delay Mix",  2.5f));
    layout.add (knob ("output",   "Output",     8.6f));

    layout.add (std::make_unique<juce::AudioParameterBool> ("bypass", "Bypass", false));

    return layout;
}

void AtlasAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    synth.setCurrentPlaybackSampleRate (sampleRate);

    juce::dsp::ProcessSpec spec { sampleRate,
                                  (juce::uint32) samplesPerBlock,
                                  (juce::uint32) getTotalNumOutputChannels() };
    filter.prepare (spec);
    filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

    delayLine.prepare (spec);
    delayLine.reset();

    visualiser.clear();
}

bool AtlasAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono()
        || out == juce::AudioChannelSet::stereo();
}

void AtlasAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // Merge notes played on the on-screen keyboard into the MIDI stream
    keyboardState.processNextMidiBuffer (midi, 0, buffer.getNumSamples(), true);

    // ---- read knobs (0..10) and map to real units ----
    auto get = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };

    const int   osc      = (int) get ("osc");
    const float envelope = get ("envelope");

    // cutoff: 0..10 -> 60 Hz .. 18 kHz (exponential feels natural)
    const float cutoffHz = 60.0f * std::pow (300.0f, get ("cutoff") / 10.0f);
    // resonance: 0..10 -> Q of 0.7 .. 6
    const float resoQ    = 0.7f + (get ("reso") / 10.0f) * 5.3f;
    // delay: 0..10 -> up to 1 second, feedback capped below self-oscillation
    const float delaySec = 0.02f + (get ("delaytime") / 10.0f) * 0.98f;
    const float delayFb  = (get ("delayfb") / 10.0f) * 0.85f;
    const float delayMix = get ("delaymix") / 10.0f;
    const float outGain  = std::pow (get ("output") / 10.0f, 1.5f);

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
            voice->update (osc, envelope);

    // ---- 1) voices render into the buffer ----
    synth.renderNextBlock (buffer, midi, 0, buffer.getNumSamples());

    // ---- 2) low-pass filter ----
    filter.setCutoffFrequency (cutoffHz);
    filter.setResonance (resoQ);
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    filter.process (ctx);

    // ---- 3) delay (per-sample so feedback works) ----
    delayLine.setDelay ((float) (delaySec * currentSampleRate));

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        for (int n = 0; n < buffer.getNumSamples(); ++n)
        {
            const float dry     = data[n];
            const float delayed = delayLine.popSample (ch);
            delayLine.pushSample (ch, dry + delayed * delayFb);
            data[n] = dry + delayed * delayMix;
        }
    }

    // ---- 4) output gain / bypass ----
    buffer.applyGain (get ("bypass") > 0.5f ? 0.0f : outGain);

    // feed the MONITOR display
    visualiser.pushBuffer (buffer);
}

// ---- save / restore knob positions with the DAW project ----
void AtlasAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void AtlasAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* AtlasAudioProcessor::createEditor()
{
    return new AtlasAudioProcessorEditor (*this);
}

// The factory function every JUCE plugin needs
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AtlasAudioProcessor();
}
