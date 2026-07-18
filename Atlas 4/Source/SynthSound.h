#pragma once
#include <juce_audio_utils/juce_audio_utils.h>

/*
    A "sound" in JUCE's synth framework describes WHICH notes/channels
    a voice may play. Ours says: play everything.
    (The audio itself is generated in SynthVoice.h)
*/
class SynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote    (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};
