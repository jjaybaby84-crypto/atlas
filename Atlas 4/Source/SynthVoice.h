#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "SynthSound.h"

/*
    SynthVoice = one playable note. Atlas has 8 of these,
    so you can play chords (polyphony).

    renderNextBlock() runs on the audio thread and fills the
    buffer with samples: raw saw/square wave shaped by an ADSR.
*/
class SynthVoice : public juce::SynthesiserVoice
{
public:
    enum Wave { Saw = 0, Square };

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SynthSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int) override
    {
        frequency = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        level     = velocity * 0.3f;    // headroom so chords don't clip
        phase     = 0.0;

        adsr.setSampleRate (getSampleRate());
        adsr.noteOn();
    }

    void stopNote (float, bool allowTailOff) override
    {
        adsr.noteOff();
        if (! allowTailOff)
            clearCurrentNote();
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    /*  The single ENVELOPE knob (0..10) controls the whole shape:
        higher = slower attack and longer release. */
    void update (int waveChoice, float envKnob)
    {
        wave = static_cast<Wave> (waveChoice);

        const float t = envKnob / 10.0f;               // 0..1
        juce::ADSR::Parameters p;
        p.attack  = 0.002f + t * 0.5f;                 // 2 ms .. 0.5 s
        p.decay   = 0.05f  + t * 0.4f;
        p.sustain = 0.8f;
        p.release = 0.05f  + t * 1.5f;                 // 50 ms .. 1.55 s
        adsr.setParameters (p);
    }

    void renderNextBlock (juce::AudioBuffer<float>& out,
                          int startSample, int numSamples) override
    {
        if (! isVoiceActive())
            return;

        const double phaseDelta = frequency / getSampleRate();

        while (--numSamples >= 0)
        {
            // 1) raw waveform (phase runs 0 -> 1 then wraps)
            float sample = (wave == Saw) ? (float) (2.0 * phase - 1.0)
                                         : (phase < 0.5 ? 1.0f : -1.0f);

            // 2) shape with envelope + velocity
            sample *= adsr.getNextSample() * level;

            // 3) same signal to all channels (mono voice)
            for (int ch = 0; ch < out.getNumChannels(); ++ch)
                out.addSample (ch, startSample, sample);

            ++startSample;
            phase += phaseDelta;
            if (phase >= 1.0) phase -= 1.0;

            if (! adsr.isActive())      // release finished -> free the voice
            {
                clearCurrentNote();
                break;
            }
        }
    }

private:
    double frequency = 440.0;
    double phase     = 0.0;
    float  level     = 0.0f;
    Wave   wave      = Saw;
    juce::ADSR adsr;
};
